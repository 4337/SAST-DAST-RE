#include <Windows.h>
#include <string>

#include "Utils.h"
#include "Crypto.h"

int Crypto::generate_key_pair() { 

    if (sodium_init() < 0) {                                                               
        return -1;
    }

    public_key.resize(crypto_box_PUBLICKEYBYTES);
    private_key.resize(crypto_box_SECRETKEYBYTES);

    int result = crypto_box_keypair(&public_key[0], &private_key[0]);

    if (result != 0) {
        public_key.clear();
        private_key.clear();
    }

    return result;
}

binary_string Crypto::get_shared_key(const binary_string& foreign_public_key) const noexcept(false) {

    if (foreign_public_key.size() != crypto_box_PUBLICKEYBYTES) {
        throw std::invalid_argument("Peer public key size is invalid.");
    }

    binary_string scalar_mult_result;
    scalar_mult_result.resize(crypto_scalarmult_BYTES);

    if (crypto_scalarmult(&scalar_mult_result[0], this->private_key.data(), foreign_public_key.data()) != 0) {
        sodium_memzero(&scalar_mult_result[0], scalar_mult_result.size());
        throw std::runtime_error("Error during scalar multiplication calculation.");
    }

    binary_string shared_key;
    shared_key.resize(crypto_hash_sha256_BYTES);
    crypto_hash_sha256(&shared_key[0], scalar_mult_result.data(), scalar_mult_result.size());

    sodium_memzero(&scalar_mult_result[0], scalar_mult_result.size());

    return shared_key;

}

binary_string Crypto::encrypt(const std::string& plaintext, const binary_string& shared_key) const noexcept(false) {

    if (shared_key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
        throw std::invalid_argument("Invalid key length. 32 bytes required.");
    }

    const size_t version_len = sizeof(VERSION_BYTES);
    const size_t nonce_len = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; 
    const size_t mac_len = crypto_aead_xchacha20poly1305_ietf_ABYTES;  

    size_t ciphertext_max_len = plaintext.size() + mac_len;
    size_t total_len = version_len + nonce_len + ciphertext_max_len;

    binary_string result(total_len, '\0');

    std::memcpy(result.data(), VERSION_BYTES, version_len);

    uint8_t* nonce_ptr = result.data() + version_len;
    randombytes_buf(nonce_ptr, nonce_len);

    uint8_t* ciphertext_ptr = nonce_ptr + nonce_len;
    unsigned long long ciphertext_real_len = 0;

    int status = crypto_aead_xchacha20poly1305_ietf_encrypt(
        ciphertext_ptr, &ciphertext_real_len,
        reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size(),
        nullptr, 0,            
        nullptr,               
        nonce_ptr,             
        shared_key.data()      
    );

    if (status != 0) {
        throw std::runtime_error("Critical error during data encryption operation.");
    }

    result.resize(version_len + nonce_len + ciphertext_real_len);

    return result;
}

std::string Crypto::decrypt(const binary_string& cipher, const binary_string& shared_key) const {
    
    size_t VERSION_SIZE = 1;
    static constexpr size_t NONCE_SIZE = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; 
    static constexpr size_t TAG_SIZE = crypto_aead_xchacha20poly1305_ietf_ABYTES;    

    if (shared_key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
        return "";
    }

    size_t minLength = VERSION_SIZE + NONCE_SIZE + TAG_SIZE;
    if (cipher.size() < minLength) {
        return "";
    }

    const unsigned char* noncePtr = cipher.data() + VERSION_SIZE;
    const unsigned char* ciphertextPtr = cipher.data() + VERSION_SIZE + NONCE_SIZE;

    size_t ciphertextLength = cipher.size() - VERSION_SIZE - NONCE_SIZE;

    size_t decryptedLength = ciphertextLength - TAG_SIZE;
    binary_string decryptedMessage(decryptedLength, 0x00);
    unsigned long long actualDecryptedLength = 0;

    int result = crypto_aead_xchacha20poly1305_ietf_decrypt(
        decryptedMessage.data(), &actualDecryptedLength,
        nullptr, 
        ciphertextPtr, ciphertextLength,
        nullptr, 0, 
        noncePtr,
        shared_key.data()
    );

    if (result != 0) {
        return "";
    }

    decryptedMessage.resize(actualDecryptedLength);
    return std::string(decryptedMessage.begin(), decryptedMessage.end());

}
