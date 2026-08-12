#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>

using binary_string = std::basic_string<unsigned char>;

binary_string decode_base64(const std::string&);
std::string encode_base64(const binary_string&);

const int BUFF_SIZE = 4096;

std::string read_file(const std::string&);

bool is_valid_file_path(const std::string& path_str);

class Console {
    HANDLE out_handler;
    HANDLE in_handler;

    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

public:
    Console() : out_handler(INVALID_HANDLE_VALUE), in_handler(INVALID_HANDLE_VALUE) {
        if (AllocConsole()) {
            out_handler = CreateFileA(
                "CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
            );
            in_handler = CreateFile(
                "CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
            );
        }
    }

    void Close() {
        if (out_handler != INVALID_HANDLE_VALUE) {
            CloseHandle(out_handler);
            out_handler = INVALID_HANDLE_VALUE;
        }
        if (in_handler != INVALID_HANDLE_VALUE) {
            CloseHandle(in_handler);
            in_handler = INVALID_HANDLE_VALUE;
        }
        FreeConsole();
    }

    ~Console() {
        Close();
    }

    DWORD write(const char* format, ...) {

        if (out_handler == INVALID_HANDLE_VALUE || !format) {
            return 0;
        }

        va_list args;

        va_start(args, format);
        int potrzebny_rozmiar = vsnprintf(nullptr, 0, format, args);
        va_end(args);

        if (potrzebny_rozmiar <= 0) {
            return 0;
        }

        std::vector<char> bufor(potrzebny_rozmiar + 1);

        va_start(args, format);
        vsnprintf(bufor.data(), bufor.size(), format, args);
        va_end(args);

        DWORD written_bytes = 0;
        WriteFile(out_handler, bufor.data(), static_cast<DWORD>(potrzebny_rozmiar), &written_bytes, NULL);
        return written_bytes;

    }

    int read_int_range(int min_val, int max_val) {
        if (in_handler == INVALID_HANDLE_VALUE) {
            return 0; 
        }

        int value = 0;
        char buffer[BUFF_SIZE];
        RtlZeroMemory(buffer, sizeof(buffer));

        DWORD bytes_read = 0;
        if (ReadFile(in_handler, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {

            char end_char = '\0';
            int parsed_count = sscanf_s(buffer, "%d%c", &value, &end_char, (unsigned int)sizeof(end_char));

            if (parsed_count >= 1) {
                if (parsed_count == 2 && end_char != '\n' && end_char != '\r' && end_char != ' ') {
                    return 0;
                }

                if (value >= min_val && value <= max_val) {
                    return value; 
                }
            }
        }

        return 0;
    }

    std::string read_string() {
        if (in_handler == INVALID_HANDLE_VALUE) {
            return "";
        }

        char buffer[BUFF_SIZE];
        RtlZeroMemory(buffer, sizeof(buffer));

        DWORD bytes_read = 0;
        if (ReadFile(in_handler, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {

            std::string result(buffer, bytes_read);

            while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
                result.pop_back();
            }

            return result;
        }

        return "";
    }

};
