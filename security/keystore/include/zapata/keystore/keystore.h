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
 * @file keystore/keystore.h
 * @brief Abstract secure key store and its global accessor.
 *
 * A key store holds symmetric secrets (API keys, tokens, connection passwords,
 * keying material, ...). Values are never persisted in plaintext: each secret is
 * encrypted under its own data-encryption key (DEK), which is in turn wrapped
 * under a master key derived from a passphrase (envelope encryption).
 */

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <zapata/connector/connector.h>
#include <zapata/json.h>

namespace zpt {
namespace keystore {

/**
 * @brief Abstract interface for a secure key store.
 */
class key_store {
  public:
    using ptr = std::shared_ptr<key_store>;
    key_store() = default;
    virtual ~key_store() = default;

    /**
     * @brief Stores (or overwrites) a secret.
     * @param _id Identifier for the key.
     * @param _value The secret value.
     * @param _meta Optional metadata, persisted alongside the (encrypted) value.
     */
    virtual auto create(std::string const& _id,
                        std::string const& _value,
                        zpt::json _meta = json_null) -> void = 0;

    /**
     * @brief Retrieves a secret.
     * @param _id Identifier for the key.
     * @return The secret value.
     * @throws zpt::ExpectationException If the key is missing, or the integrity
     *         check fails (wrong passphrase or tampering).
     */
    virtual auto get(std::string const& _id) -> std::string = 0;

    /**
     * @brief Replaces the value of an existing secret (rotation).
     * @param _id Identifier for the key.
     * @param _value The new secret value.
     * @throws zpt::ExpectationException If the key is missing.
     */
    virtual auto rotate(std::string const& _id, std::string const& _value) -> void = 0;

    /**
     * @brief Removes a secret.
     * @param _id Identifier for the key.
     * @return Number of rows removed (0 if the key was not present).
     */
    virtual auto remove(std::string const& _id) -> std::size_t = 0;

    /**
     * @brief Checks for the presence of a key.
     * @param _id Identifier for the key.
     * @return true if the key is present, false otherwise.
     */
    virtual auto has(std::string const& _id) -> bool = 0;

    /**
     * @brief Lists all keys.
     * @return A JSON array of {_id, meta, version, created} objects. Secret
     *         values and encryption material are never included.
     */
    virtual auto list() -> zpt::json = 0;

    /** @brief Removes all secrets. */
    virtual auto clear() -> void = 0;
};

/**
 * @brief Storage-backed, envelope-encrypted implementation of @ref key_store.
 *
 * Secrets are persisted in a SQLite database and are never stored in plaintext:
 * each value is encrypted under its own data-encryption key (DEK) using
 * AES-256-GCM, and the DEK is wrapped under a master key derived from the
 * passphrase (PBKDF2-HMAC-SHA256). Only encrypted material and non-secret
 * metadata are ever written to disk.
 *
 * SQLite layout:
 *   store(_id TEXT PRIMARY KEY, salt TEXT, kdf TEXT, iterations INTEGER)
 *   keys(_id TEXT PRIMARY KEY, meta TEXT, dek_wrapped TEXT, dek_nonce TEXT,
 *        dek_tag TEXT, ct TEXT, ct_nonce TEXT, ct_tag TEXT, version INTEGER,
 *        created INTEGER)
 */
class storage_key_store : public key_store {
  public:
    /**
     * @brief Opens (or creates) the store and derives the master key.
     * @param _passphrase The root-of-trust passphrase. When empty, it is read
     *        from the ZAPATA_KEYSTORE_PASSPHRASE environment variable, then
     *        from the "passphrase" entry of the keystore configuration.
     */
    explicit storage_key_store(std::string const& _passphrase);
    ~storage_key_store() override = default;

    auto create(std::string const& _id, std::string const& _value, zpt::json _meta = json_null)
      -> void override;
    auto get(std::string const& _id) -> std::string override;
    auto rotate(std::string const& _id, std::string const& _value) -> void override;
    auto remove(std::string const& _id) -> std::size_t override;
    auto has(std::string const& _id) -> bool override;
    auto list() -> zpt::json override;
    auto clear() -> void override;

  private:
    /** @brief Loads the row for _id. @throws If the key is not present. */
    auto load_row(std::string const& _id) -> zpt::json;
    /** @brief Parses the (stored) metadata of a row, if any. */
    auto parse_meta(zpt::json const& _row) -> zpt::json;
    /** @brief Encrypts a value under a fresh DEK wrapped by the master key. */
    auto encrypt_row(std::string const& _id,
                     std::string const& _value,
                     zpt::json _meta,
                     long long _version,
                     long long _created) -> zpt::json;
    /** @brief Unwraps the DEK and decrypts a row. @throws If integrity fails. */
    auto decrypt_row(zpt::json const& _row) -> std::string;
    /** @brief Replaces the row for its _id (remove + add). */
    auto upsert(zpt::json const& _row) -> void;

    zpt::storage::connection __connection;
    zpt::storage::collection __store;
    zpt::storage::collection __keys;
    std::vector<unsigned char> __master_key;
};

} // namespace keystore

/**
 * @brief Global accessor for the (single) key store instance.
 *
 * On first invocation the store is constructed: the persistent backend is
 * opened and the master key is derived from the passphrase. Subsequent calls
 * return the same instance (mirrors the zpt::OAUTH2_SERVER accessor).
 * @param _passphrase The root-of-trust passphrase. When empty, it is read from
 *        the ZAPATA_KEYSTORE_PASSPHRASE environment variable, then from the
 *        "passphrase" entry of the keystore configuration.
 * @return Reference to the key store.
 */
auto KEY_STORE(std::string const& _passphrase = "") -> keystore::key_store&;

} // namespace zpt
