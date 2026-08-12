#include <string>
#include <regex>
#include <algorithm>
#include <string_view>

#include "Ipc.h"
#include "Utils.h"

DWORD Ipc::impersonation() {

    return ERROR_SUCCESS;

}

DWORD Ipc::client(const std::string& pipe_name, DWORD access_mode, DWORD timeout) {
    
    if (pipe_name.empty()) {
        return 0x43374337;
    }

    if (handler != INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    while (true) {

        handler = CreateFile(pipe_name.c_str(),
            access_mode,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (handler != INVALID_HANDLE_VALUE) {
            break;
        }

        if (GetLastError() != ERROR_PIPE_BUSY) {
            return GetLastError();
        }

        if (!WaitNamedPipe(pipe_name.c_str(), timeout)) {
            return GetLastError();
        }
    }

    return ERROR_SUCCESS;

}

DWORD Ipc::server(const std::string& pipe_name, DWORD open_mode, DWORD pipe_mode, DWORD max_instances) {

    if (handler != INVALID_HANDLE_VALUE) {
        return ERROR_INVALID_HANDLE;
    }

    if (pipe_name.empty()) {
        return 0x43374337;
    }

    SECURITY_DESCRIPTOR sd;
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        return GetLastError();
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, nullptr, FALSE)) {
        return GetLastError();
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    handler = CreateNamedPipe(
        pipe_name.c_str(),
        open_mode,
        pipe_mode,
        max_instances,
        1024 << 2,
        1024 << 2,
        0,
        NULL//&sa
    );

    if (handler == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    BOOL connected = ConnectNamedPipe(handler, nullptr);

    if (!connected) {
        DWORD last_error = GetLastError();
        if (last_error == ERROR_PIPE_CONNECTED) {
            return NO_ERROR;
        }

        CloseHandle(handler);
        handler = INVALID_HANDLE_VALUE;
        return last_error;
    }

    return NO_ERROR;
}

template <typename T>
T Ipc::read(size_t read_size) {

    if (read_size == 0) {
        return T();
    }

    T buffer;
    buffer.resize(read_size);

    DWORD readed_bytes = 0;

    if (!ReadFile(handler, buffer.data(),
        static_cast<DWORD>(read_size),
        &readed_bytes, nullptr)) {
        return T();
    }

    buffer.resize(static_cast<size_t>(readed_bytes));

    return buffer;

}

binary_string Ipc::read_crypto_stream(size_t bytes_to_read) {

    binary_string result;
    if (bytes_to_read == 0) {
        return result;
    }

    result.resize(bytes_to_read);

    size_t all_read_bytes = 0;
    DWORD chunk_to_read = 0;
    DWORD bytes_read = 0;

    while (all_read_bytes < bytes_to_read) {
        size_t remaining = bytes_to_read - all_read_bytes;
        chunk_to_read = static_cast<DWORD>((std::min)(remaining, static_cast<size_t>(MAXDWORD)));
        unsigned char* buffer_ptr = result.data() + all_read_bytes;

        if (!ReadFile(handler, buffer_ptr, 
                      chunk_to_read, 
                      &bytes_read, 
                      nullptr)) {
            break;
        }

        if (bytes_read == 0) {
            break; 
        }

        all_read_bytes += bytes_read;
    }

    if (all_read_bytes < bytes_to_read) {
        result.resize(all_read_bytes);
    }

    return result;
}


size_t Ipc::write_crypto_stream(const binary_string& data) {

    size_t total_length = data.length();
    size_t all_written_bytes = 0;
    DWORD bytes_to_write = 0;
    DWORD bytes_written = 0;

    while (all_written_bytes < total_length) {

        size_t remaining = total_length - all_written_bytes;
        bytes_to_write = static_cast<DWORD>((std::min)(remaining, static_cast<size_t>(MAXDWORD)));
        const unsigned char* buffer_ptr = data.data() + all_written_bytes;

        if (!WriteFile(handler, buffer_ptr, 
                       bytes_to_write, 
                       &bytes_written, nullptr)) {
            return all_written_bytes; 
        }
        if (bytes_written == 0) {
            break;
        }
        all_written_bytes += bytes_written;

    }

    return all_written_bytes;
}


template <typename T>
size_t Ipc::write(const T& data) {

    DWORD writed_bytes = 0;
    WriteFile(handler, data.data(), static_cast<DWORD>(data.size()), &writed_bytes, nullptr);
    return static_cast<size_t>(writed_bytes);

}

binary_string Ipc::crypto_stream_header(const binary_string& cip) {

    size_t len = cip.size();

    binary_string msg(len + 4,0);
    
    msg[0] = static_cast<unsigned char>((len >> 24) & 0xFF);
    msg[1] = static_cast<unsigned char>((len >> 16) & 0xFF);
    msg[2] = static_cast<unsigned char>((len >> 8) & 0xFF);
    msg[3] = static_cast<unsigned char>(len & 0xFF);

    //msg[4] = 0x00;
    //msg[5] = 0x00;
    //msg[6] = 0x00;
    //msg[7] = 0x01;

    std::memcpy(msg.data() + 4, cip.data(), len);

    return msg;
}

template<typename T>
binary_string Ipc::create_public_msg(const T& msg) {

    size_t msg_len = msg.size();

    if (msg_len <= HEADER_SIZE) {
        return binary_string({ 0 });
    }

    binary_string message(msg_len + HEADER_SIZE, 0);

    message[0] = static_cast<unsigned char>((msg_len >> 24) & 0xFF);
    message[1] = static_cast<unsigned char>((msg_len >> 16) & 0xFF);
    message[2] = static_cast<unsigned char>((msg_len >> 8) & 0xFF);
    message[3] = static_cast<unsigned char>(msg_len & 0xFF);

    std::memcpy(message.data() + HEADER_SIZE, msg.data(), msg_len);

    return message;
}

template <typename T>
std::string Ipc::create_pipename(const T& msg) {
    if (msg.size() <= HEADER_SIZE) {
        return "";
    }

    std::string msg_str(reinterpret_cast<const char*>(msg.data()) + HEADER_SIZE, msg.size() - HEADER_SIZE);

    if (msg_str.empty()) {
        return "";
    }

    std::regex pattern(R"(\"(?:Name|PrivatePipeNames)\"\s*:\s*\[?\s*\"([^\"]+)\")");
    std::smatch matches; // Używamy smatch dla std::string

    std::string pipe_name;

    if (std::regex_search(msg_str, matches, pattern)) {
        std::string result = matches.str(1);

        result = std::regex_replace(result, std::regex(R"(\\\\)"), "\\");

        pipe_name = R"(\\.\pipe\)";
        pipe_name += result;
    }

    return pipe_name;

}



template<typename T>
std::string Ipc::get_public_key(const T& json) {

    if (!json.empty()) {

        std::string_view json_view(reinterpret_cast<const char*>(json.data()), json.size());

        std::regex pattern("\\\"PublicKey\\\"\\s*:\\s*\\\"([A-Za-z0-9+/=]+)\\\"");
        std::match_results<std::string_view::const_iterator> matches;

        if (std::regex_search(json_view.begin(), json_view.end(), matches, pattern)) {
            return matches.str(1);
        }

    }

    return "";

}

template binary_string Ipc::create_private_msg(const std::string&);
template binary_string Ipc::create_private_msg(const binary_string&);
template binary_string Ipc::create_private_msg(const std::u8string&);

template std::string Ipc::get_public_key(const binary_string&);
template std::string Ipc::get_public_key(const std::string&);
template std::string Ipc::get_public_key(const std::u8string&);


template binary_string Ipc::create_public_msg(const binary_string&);
template binary_string Ipc::create_public_msg(const std::string&);
template binary_string Ipc::create_public_msg(const std::u8string&);

template size_t Ipc::write<binary_string>(const binary_string&);
template size_t Ipc::write<std::string>(const std::string&);


template size_t Ipc::write<std::u8string>(const std::u8string&);

template std::string Ipc::create_pipename<std::string>(const std::string&);
template std::string Ipc::create_pipename<binary_string>(const binary_string&);

template std::string Ipc::read(size_t);
template binary_string Ipc::read(size_t);

