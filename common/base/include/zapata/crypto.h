#pragma once

#include <cstring>
#include <iostream>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string>
#include <vector>

namespace zpt {

/**
 * @brief AES encryption and decryption utility.
 *
 * Provides AES encryption using EVP (high-level OpenSSL API) with
 * AES-256-CBC cipher. Automatically pads plaintext and generates
 * random IV when constructed without arguments.
 */
class aes_crypto {
  public:
    /** @brief Default constructor with random key and IV. */
    aes_crypto();
    /**
     * @brief Constructs with user-specified key and IV.
     * @param _key_str Key string (must be 32 bytes or fewer).
     * @param _iv_str IV string (must be 16 bytes).
     */
    aes_crypto(std::string const& _key_str, std::string const& _iv_str);

    /**
     * @brief Encrypts plaintext using AES-256-CBC.
     * @param _plaintext The plaintext string to encrypt.
     * @return Encrypted bytes as a vector.
     */
    auto encrypt(std::string const& _plaintext) -> std::vector<unsigned char>;
    /**
     * @brief Decrypts ciphertext using AES-256-CBC.
     * @param _ciphertext The encrypted bytes to decrypt.
     * @return Decrypted plaintext string.
     */
    auto decrypt(std::vector<unsigned char> const& _ciphertext) -> std::string;
    /**
     * @brief Writes hex representation of data to a stream.
     * @param out Output stream to write to.
     * @param _data Data bytes to convert to hex.
     */
    auto print_hex(std::ostream& out, std::vector<unsigned char> const& _data) -> void;

  private:
    unsigned char __key[32]; ///< Encryption key.
    unsigned char __iv[16];  ///< Initialization vector.
};
} // namespace zpt
