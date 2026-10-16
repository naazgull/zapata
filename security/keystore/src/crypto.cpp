/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <zapata/keystore/crypto.h>

namespace zpt {
namespace keystore {
namespace crypto {

auto random_bytes(std::vector<unsigned char>& _out, std::size_t _size) -> void {
    _out.resize(_size);
    if (_size > 0 && RAND_bytes(_out.data(), static_cast<int>(_size)) != 1) {
        throw std::runtime_error("keystore: failed to generate random bytes");
    }
}

auto seal(std::vector<unsigned char> const& _key,
          std::vector<unsigned char> const& _plaintext,
          std::vector<unsigned char>& _ciphertext,
          std::vector<unsigned char>& _nonce,
          std::vector<unsigned char>& _tag) -> void {
    random_bytes(_nonce, nonce_size);
    _tag.assign(tag_size, 0);
    // GCM is a stream mode: the ciphertext is exactly as long as the plaintext.
    _ciphertext.assign(_plaintext.size(), 0);

    EVP_CIPHER_CTX* _ctx = EVP_CIPHER_CTX_new();
    if (_ctx == nullptr) { throw std::runtime_error("keystore: failed to create cipher context"); }

    bool _ok = EVP_EncryptInit_ex(_ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
               EVP_CIPHER_CTX_ctrl(
                 _ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce_size), nullptr) == 1 &&
               EVP_EncryptInit_ex(_ctx, nullptr, nullptr, _key.data(), _nonce.data()) == 1;
    int _len = 0;
    if (_ok && EVP_EncryptUpdate(_ctx,
                                 _ciphertext.data(),
                                 &_len,
                                 _plaintext.data(),
                                 static_cast<int>(_plaintext.size())) == 1) {
        int _final_len = 0;
        _ok = EVP_EncryptFinal_ex(_ctx, _ciphertext.data() + _len, &_final_len) == 1;
        if (_ok) { _ciphertext.resize(_len + _final_len); }
    }
    else { _ok = false; }
    if (_ok) {
        _ok = EVP_CIPHER_CTX_ctrl(
                _ctx, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag_size), _tag.data()) == 1;
    }

    EVP_CIPHER_CTX_free(_ctx);
    if (!_ok) { throw std::runtime_error("keystore: AES-256-GCM encryption failed"); }
}

auto open(std::vector<unsigned char> const& _key,
          std::vector<unsigned char> const& _ciphertext,
          std::vector<unsigned char> const& _nonce,
          std::vector<unsigned char> const& _tag,
          std::vector<unsigned char>& _plaintext) -> bool {
    _plaintext.assign(_ciphertext.size(), 0);

    EVP_CIPHER_CTX* _ctx = EVP_CIPHER_CTX_new();
    if (_ctx == nullptr) { return false; }

    bool _ok = EVP_DecryptInit_ex(_ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1 &&
               EVP_CIPHER_CTX_ctrl(
                 _ctx, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(nonce_size), nullptr) == 1 &&
               EVP_DecryptInit_ex(_ctx, nullptr, nullptr, _key.data(), _nonce.data()) == 1;
    int _len = 0;
    if (_ok && EVP_DecryptUpdate(_ctx,
                                 _plaintext.data(),
                                 &_len,
                                 _ciphertext.data(),
                                 static_cast<int>(_ciphertext.size())) == 1) {
        // Supply the expected tag; DecryptFinal verifies it (returns -1 on mismatch).
        _ok = EVP_CIPHER_CTX_ctrl(_ctx,
                                  EVP_CTRL_GCM_SET_TAG,
                                  static_cast<int>(tag_size),
                                  const_cast<unsigned char*>(_tag.data())) == 1;
        if (_ok) {
            int _final_len = 0;
            _ok = EVP_DecryptFinal_ex(_ctx, _plaintext.data() + _len, &_final_len) == 1;
            if (_ok) { _plaintext.resize(_len + _final_len); }
        }
    }
    else { _ok = false; }

    EVP_CIPHER_CTX_free(_ctx);
    return _ok;
}

auto derive_key(std::string const& _passphrase,
                std::vector<unsigned char> const& _salt,
                long long _iterations,
                std::vector<unsigned char>& _key) -> void {
    _key.resize(key_size);
    if (PKCS5_PBKDF2_HMAC(_passphrase.data(),
                          static_cast<int>(_passphrase.size()),
                          _salt.data(),
                          static_cast<int>(_salt.size()),
                          static_cast<int>(_iterations),
                          EVP_sha256(),
                          static_cast<int>(key_size),
                          _key.data()) != 1) {
        throw std::runtime_error("keystore: PBKDF2-HMAC-SHA256 key derivation failed");
    }
}

} // namespace crypto
} // namespace keystore
} // namespace zpt
