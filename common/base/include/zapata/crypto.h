#pragma once

#include <cstring>
#include <iostream>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string>
#include <vector>

namespace zpt {
class aes_crypto {
  public:
    aes_crypto();
    aes_crypto(std::string const& _key_str, std::string const& _iv_str);

    auto encrypt(std::string const& _plaintext) -> std::vector<unsigned char>;
    auto decrypt(std::vector<unsigned char> const& _ciphertext) -> std::string;
    auto print_hex(std::ostream& out, std::vector<unsigned char> const& _data) -> void;

  private:
    unsigned char __key[32];
    unsigned char __iv[16];
};
} // namespace zpt
