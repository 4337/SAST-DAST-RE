#include "Utils.h"

bool is_valid_file_path(const std::string& path_str) {
    if (path_str.empty()) {
        return false;
    }
    std::filesystem::path p(path_str);
    return std::filesystem::exists(p) && std::filesystem::is_regular_file(p);
}

std::string read_file(const std::string& path) {

    HANDLE h_file = CreateFileA(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (h_file == INVALID_HANDLE_VALUE) {
        return "";
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(h_file, &file_size) || file_size.QuadPart <= 0) {
        CloseHandle(h_file);
        return "";
    }

    std::string content;
    content.resize(static_cast<size_t>(file_size.QuadPart));

    DWORD bytes_to_read = static_cast<DWORD>(file_size.QuadPart);
    DWORD bytes_read = 0;

    BOOL read_success = ReadFile(
        h_file,
        &content[0],
        bytes_to_read,
        &bytes_read,
        nullptr
    );

    CloseHandle(h_file);

    if (!read_success || bytes_read != bytes_to_read) {
        return "";
    }

    return content;
}

std::string encode_base64(const binary_string& input) {
    std::string output;

    static const char lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t length = input.length();
    if (length == 0) {
        return output;
    }
    output.reserve(((length + 2) / 3) * 4);

    size_t i = 0;
    while (i < length) {
      
        uint32_t b0 = input[i++];
        uint32_t b1 = (i < length) ? input[i++] : 0;
        uint32_t b2 = (i < length) ? input[i++] : 0;

        uint32_t triple = (b0 << 16) | (b1 << 8) | b2;

        output.push_back(lookup[(triple >> 18) & 0x3F]);
        output.push_back(lookup[(triple >> 12) & 0x3F]);
        output.push_back(lookup[(triple >> 6) & 0x3F]);
        output.push_back(lookup[triple & 0x3F]);
    }

    size_t remainder = length % 3;
    if (remainder == 1) {
        output[output.length() - 1] = '=';
        output[output.length() - 2] = '=';
    }
    else if (remainder == 2) {
        output[output.length() - 1] = '=';
    }

    return output;
}

binary_string decode_base64(const std::string& b64) {
    binary_string bin;

    static const unsigned char lookup[256] = {
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
        52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
        64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
        64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
    };

    if (b64.empty()) return bin;

    bin.reserve(((b64.length() + 3) / 4) * 3);

    int val = 0;
    int valb = -8;

    for (unsigned char c : b64) {
        if (c == '=') break;
        if (lookup[c] == 64) continue;

        val = (val << 6) + lookup[c];
        valb += 6;

        if (valb >= 0) {
            bin.push_back(static_cast<unsigned char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return bin;
}
