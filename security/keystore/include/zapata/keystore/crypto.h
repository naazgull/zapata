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

/**
 * @file keystore/crypto.h
 * @brief Self-contained AEAD and key-derivation primitives for the key store.
 *
 * These helpers wrap the OpenSSL EVP interface with the exact algorithms the
 * key store relies on: AES-256-GCM for authenticated encryption and
 * PBKDF2-HMAC-SHA256 for passphrase key derivation.
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace zpt {
namespace keystore {
namespace crypto {

/** @brief AES-256 key length, in bytes. */
inline constexpr std::size_t key_size = 32;
/** @brief GCM nonce length, in bytes (96 bits, the recommended value). */
inline constexpr std::size_t nonce_size = 12;
/** @brief GCM authentication tag length, in bytes. */
inline constexpr std::size_t tag_size = 16;
/** @brief Recommended random salt length, in bytes. */
inline constexpr std::size_t salt_size = 16;
/** @brief Recommended PBKDF2 iteration count. */
inline constexpr long long default_iterations = 600000LL;

/**
 * @brief Generates cryptographically secure random bytes.
 * @param _out Output buffer (resized to @p _size).
 * @param _size Number of bytes to generate.
 * @throws std::runtime_error If the CSPRNG fails.
 */
auto random_bytes(std::vector<unsigned char>& _out, std::size_t _size) -> void;

/**
 * @brief Authenticates and encrypts a value using AES-256-GCM (seal).
 *
 * A random nonce is generated and an authentication tag is produced. The
 * ciphertext, nonce and tag are returned in separate buffers so that each can
 * be persisted independently.
 * @param _key The 32-byte key.
 * @param _plaintext The value to protect.
 * @param _ciphertext Output ciphertext (does not include the tag).
 * @param _nonce Output generated nonce.
 * @param _tag Output authentication tag.
 * @throws std::runtime_error If the cipher cannot be initialised or the
 *         encryption fails.
 */
auto seal(std::vector<unsigned char> const& _key,
          std::vector<unsigned char> const& _plaintext,
          std::vector<unsigned char>& _ciphertext,
          std::vector<unsigned char>& _nonce,
          std::vector<unsigned char>& _tag) -> void;

/**
 * @brief Authenticates and decrypts a value sealed with AES-256-GCM (open).
 *
 * The authentication tag is verified before the plaintext is released, which
 * detects both tampering and use of the wrong key.
 * @param _key The 32-byte key.
 * @param _ciphertext The ciphertext (does not include the tag).
 * @param _nonce The nonce used at seal time.
 * @param _tag The authentication tag from seal time.
 * @param _plaintext Output decrypted value.
 * @return true if the tag verifies and the plaintext is intact, false otherwise.
 */
auto open(std::vector<unsigned char> const& _key,
          std::vector<unsigned char> const& _ciphertext,
          std::vector<unsigned char> const& _nonce,
          std::vector<unsigned char> const& _tag,
          std::vector<unsigned char>& _plaintext) -> bool;

/**
 * @brief Derives a 32-byte key from a passphrase using PBKDF2-HMAC-SHA256.
 * @param _passphrase The passphrase (root of trust).
 * @param _salt The (non-secret) random salt.
 * @param _iterations The PBKDF2 iteration count.
 * @param _key Output 32-byte derived key.
 * @throws std::runtime_error If the derivation fails.
 */
auto derive_key(std::string const& _passphrase,
                std::vector<unsigned char> const& _salt,
                long long _iterations,
                std::vector<unsigned char>& _key) -> void;

} // namespace crypto
} // namespace keystore
} // namespace zpt
