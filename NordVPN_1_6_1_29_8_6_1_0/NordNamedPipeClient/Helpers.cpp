#include <string>
#include <stdexcept>
#include "Helpers.h"

Options parse(int argc, char** argv) noexcept(false) {
   
    if (argc < 3) {
        throw std::runtime_error("Missing command-line arguments -dll");
    }

    Options ret{}; 

    for (int i = 1; i < argc; i++) {

        if (strcmp("-dll", argv[i]) == 0) {

            if (i + 1 >= argc) {
                throw std::runtime_error("Missing value for -dll argument");
            }
            ret.dll = argv[i + 1];
            i++; 

        }
    }

    if (std::string(ret.dll).empty()) {
        throw std::runtime_error("Invalid command-line arguments (-dll [path]])");
    }

    return ret; 

}