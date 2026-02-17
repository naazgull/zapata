#include <zapata/crypto.h>

zpt::aes_crypto::aes_crypto() {
    RAND_bytes(this->__key, sizeof(this->__key));
    RAND_bytes(this->__iv, sizeof(this->__iv));
}

zpt::aes_crypto::aes_crypto(std::string const& _key_str, std::string const& _iv_str) {
    std::memset(this->__key, 0, sizeof(this->__key));
    std::memcpy(this->__key, _key_str.c_str(), std::min(_key_str.length(), sizeof(this->__key)));

    std::memset(this->__iv, 0, sizeof(this->__iv));
    std::memcpy(this->__iv, _iv_str.c_str(), std::min(_iv_str.length(), sizeof(this->__iv)));
}

auto zpt::aes_crypto::encrypt(std::string const& _plaintext) -> std::vector<unsigned char> {
    EVP_CIPHER_CTX* _ctx = EVP_CIPHER_CTX_new();
    if (!_ctx) { throw std::runtime_error("Failed to create cipher context"); }

    if (EVP_EncryptInit_ex(_ctx, EVP_aes_256_cbc(), nullptr, this->__key, this->__iv) != 1) {
        EVP_CIPHER_CTX_free(_ctx);
        throw std::runtime_error("Failed to initialize encryption");
    }

    std::vector<unsigned char> _ciphertext(_plaintext.length() + AES_BLOCK_SIZE);
    int _len = 0;
    int _ciphertext_len = 0;

    if (EVP_EncryptUpdate(_ctx,
                          _ciphertext.data(),
                          &_len,
                          reinterpret_cast<const unsigned char*>(_plaintext.c_str()),
                          _plaintext.length()) != 1) {
        EVP_CIPHER_CTX_free(_ctx);
        throw std::runtime_error("Encryption failed");
    }
    _ciphertext_len = _len;

    if (EVP_EncryptFinal_ex(_ctx, _ciphertext.data() + _len, &_len) != 1) {
        EVP_CIPHER_CTX_free(_ctx);
        throw std::runtime_error("Final encryption failed");
    }
    _ciphertext_len += _len;

    EVP_CIPHER_CTX_free(_ctx);

    _ciphertext.resize(_ciphertext_len);
    return _ciphertext;
}

auto zpt::aes_crypto::decrypt(std::vector<unsigned char> const& _ciphertext) -> std::string {
    EVP_CIPHER_CTX* _ctx = EVP_CIPHER_CTX_new();
    if (!_ctx) { throw std::runtime_error("Failed to create cipher context"); }

    if (EVP_DecryptInit_ex(_ctx, EVP_aes_256_cbc(), nullptr, this->__key, this->__iv) != 1) {
        EVP_CIPHER_CTX_free(_ctx);
        throw std::runtime_error("Failed to initialize decryption");
    }

    std::vector<unsigned char> _plaintext(_ciphertext.size() + AES_BLOCK_SIZE);
    int _len = 0;
    int _plaintext_len = 0;

    if (EVP_DecryptUpdate(_ctx, _plaintext.data(), &_len, _ciphertext.data(), _ciphertext.size()) !=
        1) {
        EVP_CIPHER_CTX_free(_ctx);
        throw std::runtime_error("Decryption failed");
    }
    _plaintext_len = _len;

    if (EVP_DecryptFinal_ex(_ctx, _plaintext.data() + _len, &_len) != 1) {
        EVP_CIPHER_CTX_free(_ctx);
        throw std::runtime_error("Final decryption failed");
    }
    _plaintext_len += _len;

    EVP_CIPHER_CTX_free(_ctx);

    return std::string(reinterpret_cast<char*>(_plaintext.data()), _plaintext_len);
}

auto zpt::aes_crypto::print_hex(std::ostream& out, std::vector<unsigned char> const& _data)
  -> void {
    for (unsigned char _byte : _data) { out << std::hex << _byte; }
}
