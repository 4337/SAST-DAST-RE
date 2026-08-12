#pragma once

#include <string>
#include <stdexcept>

#define SODIUM_STATIC

#include "Utils.h"
/* static libsodium*/
#include "sodium.h"


class Crypto {

    binary_string private_key;
    binary_string public_key;

    static constexpr uint8_t VERSION_BYTES[4] = { 0, 0, 0, 1 };

    int generate_key_pair();

public:

    Crypto() noexcept(false) {
        if (generate_key_pair() != 0) {
            throw std::runtime_error("An error occurred while generating the Crypto key pair.");
        }
    }

    binary_string pub_key() const noexcept {
        return public_key;
    }

    binary_string get_shared_key(const binary_string& foreign_public_key) const noexcept(false);

    binary_string encrypt(const std::string& plaintext, const binary_string& shared_key) const noexcept(false);

    std::string decrypt(const binary_string& cipher, const binary_string& shared_key) const;

};
