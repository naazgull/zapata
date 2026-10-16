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

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <zapata/base.h>
#include <zapata/config/config.h>
#include <zapata/keystore/crypto.h>
#include <zapata/keystore/keystore.h>
#include <zapata/sqlite.h>

namespace {

/** @brief Base64-encodes a byte buffer into a string. */
auto b64_encode(std::vector<unsigned char> const& _in) -> std::string {
    std::string _out;
    zpt::base64::encode(_in, _out);
    return _out;
}

/** @brief Base64-decodes a string into a byte buffer. */
auto b64_decode(std::string const& _in) -> std::vector<unsigned char> {
    std::vector<unsigned char> _out;
    zpt::base64::decode(_in, _out);
    return _out;
}

/** @brief Current time, in milliseconds since the epoch. */
auto now_millis() -> long long {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

} // namespace

zpt::keystore::storage_key_store::storage_key_store(std::string const& _passphrase) {
    std::string _pp = _passphrase;
    if (_pp.empty()) {
        if (char const* _env = std::getenv("ZAPATA_KEYSTORE_PASSPHRASE")) { _pp = _env; }
    }
    if (_pp.empty()) {
        auto _cfg_passphrase = zpt::GLOBAL_CONFIG()("keystore")("passphrase");
        if (_cfg_passphrase->ok()) { _pp = _cfg_passphrase->string(); }
    }
    expect(!_pp.empty(),
           "No passphrase available, set ZAPATA_KEYSTORE_PASSPHRASE, or set `keystore.passphrase`");

    auto _cfg = zpt::GLOBAL_CONFIG()("keystore");
    expect(_cfg("path")->is_string(),
           "No configuration provided for the key-store location, set `keystore.path`");
    auto _path = _cfg("path")->string();
    auto _database_name =
      _cfg("database")->ok() ? _cfg("database")->string() : std::string("keystore");
    auto _iterations = _cfg("pbkdf2_iterations")->ok() ? _cfg("pbkdf2_iterations")->integer()
                                                       : zpt::keystore::crypto::default_iterations;

    // The sqlite connector treats the configured "path" as a directory and opens
    // "<path>/<database>" inside it; make sure the directory exists.
    std::error_code _ec;
    std::filesystem::create_directories(_path, _ec);

    auto _storage = zpt::json{ "storage", { "sqlite", { "path", _path } } };
    this->__connection = zpt::make_connection<zpt::storage::sqlite::connection>(_storage);
    auto _session = this->__connection->session();
    auto _database = _session->database(_database_name);

    _database->sql("CREATE TABLE IF NOT EXISTS store (_id TEXT PRIMARY KEY, salt TEXT, "
                   "kdf TEXT, iterations INTEGER)");
    _database->sql("CREATE TABLE IF NOT EXISTS keys (_id TEXT PRIMARY KEY, meta TEXT, "
                   "dek_wrapped TEXT, dek_nonce TEXT, dek_tag TEXT, ct TEXT, ct_nonce TEXT, "
                   "ct_tag TEXT, version INTEGER, created INTEGER)");

    this->__store = _database->collection("store");
    this->__keys = _database->collection("keys");

    // Load (or create) the master-key derivation salt, then derive the master key.
    std::vector<unsigned char> _salt;
    auto _master_rows = this->__store->find({ "_id", "master" })->execute()->fetch();
    if (_master_rows.size() == 0) {
        zpt::keystore::crypto::random_bytes(_salt, zpt::keystore::crypto::salt_size);
        this->__store
          ->add({ "_id",
                  "master",
                  "salt",
                  b64_encode(_salt),
                  "kdf",
                  "pbkdf2-sha256",
                  "iterations",
                  _iterations })
          ->execute();
    }
    else {
        _salt = b64_decode(_master_rows(0)("salt")->string());
        if (_master_rows(0)("iterations")->ok()) {
            _iterations = _master_rows(0)("iterations")->integer();
        }
    }
    zpt::keystore::crypto::derive_key(_pp, _salt, _iterations, this->__master_key);
}

auto zpt::keystore::storage_key_store::load_row(std::string const& _id) -> zpt::json {
    auto _rows = this->__keys->find({ "_id", _id })->execute()->fetch();
    expect(_rows.size() > 0, "key_store: key not found: " << _id);
    return _rows(0);
}

auto zpt::keystore::storage_key_store::parse_meta(zpt::json const& _row) -> zpt::json {
    if (_row("meta")->ok() && !_row("meta")->string().empty()) {
        return zpt::json::parse_json_str(_row("meta")->string());
    }
    return json_null;
}

auto zpt::keystore::storage_key_store::encrypt_row(std::string const& _id,
                                                   std::string const& _value,
                                                   zpt::json _meta,
                                                   long long _version,
                                                   long long _created) -> zpt::json {
    // Fresh data-encryption key for this secret.
    std::vector<unsigned char> _dek;
    zpt::keystore::crypto::random_bytes(_dek, zpt::keystore::crypto::key_size);

    // Seal the secret under its DEK.
    std::vector<unsigned char> _plaintext(_value.begin(), _value.end());
    std::vector<unsigned char> _ct;
    std::vector<unsigned char> _ct_nonce;
    std::vector<unsigned char> _ct_tag;
    zpt::keystore::crypto::seal(_dek, _plaintext, _ct, _ct_nonce, _ct_tag);

    // Wrap the DEK under the master key.
    std::vector<unsigned char> _dek_wrapped;
    std::vector<unsigned char> _dek_nonce;
    std::vector<unsigned char> _dek_tag;
    zpt::keystore::crypto::seal(this->__master_key, _dek, _dek_wrapped, _dek_nonce, _dek_tag);

    std::string _meta_str;
    if (_meta->ok()) { _meta_str = _meta.stringify(); }

    return zpt::json{ "_id",         _id,
                      "meta",        _meta_str,
                      "dek_wrapped", b64_encode(_dek_wrapped),
                      "dek_nonce",   b64_encode(_dek_nonce),
                      "dek_tag",     b64_encode(_dek_tag),
                      "ct",          b64_encode(_ct),
                      "ct_nonce",    b64_encode(_ct_nonce),
                      "ct_tag",      b64_encode(_ct_tag),
                      "version",     _version,
                      "created",     _created };
}

auto zpt::keystore::storage_key_store::decrypt_row(zpt::json const& _row) -> std::string {
    // Unwrap the DEK with the master key.
    std::vector<unsigned char> _dek_wrapped = b64_decode(_row("dek_wrapped")->string());
    std::vector<unsigned char> _dek_nonce = b64_decode(_row("dek_nonce")->string());
    std::vector<unsigned char> _dek_tag = b64_decode(_row("dek_tag")->string());

    std::vector<unsigned char> _dek;
    expect(
      zpt::keystore::crypto::open(this->__master_key, _dek_wrapped, _dek_nonce, _dek_tag, _dek),
      "key_store: failed to unwrap data-encryption key (wrong passphrase or tampered "
      "record)");

    // Decrypt the value with the DEK.
    std::vector<unsigned char> _ct = b64_decode(_row("ct")->string());
    std::vector<unsigned char> _ct_nonce = b64_decode(_row("ct_nonce")->string());
    std::vector<unsigned char> _ct_tag = b64_decode(_row("ct_tag")->string());

    std::vector<unsigned char> _plaintext;
    expect(zpt::keystore::crypto::open(_dek, _ct, _ct_nonce, _ct_tag, _plaintext),
           "key_store: failed to decrypt value (wrong passphrase or tampered record)");

    return std::string(_plaintext.begin(), _plaintext.end());
}

auto zpt::keystore::storage_key_store::upsert(zpt::json const& _row) -> void {
    this->__keys->remove({ "_id", _row("_id")->string() })->execute();
    this->__keys->add(_row)->execute();
}

auto zpt::keystore::storage_key_store::create(std::string const& _id,
                                              std::string const& _value,
                                              zpt::json _meta) -> void {
    expect(!_id.empty(), "key_store: key id must not be empty");
    this->upsert(this->encrypt_row(_id, _value, _meta, 1, now_millis()));
}

auto zpt::keystore::storage_key_store::get(std::string const& _id) -> std::string {
    return this->decrypt_row(this->load_row(_id));
}

auto zpt::keystore::storage_key_store::rotate(std::string const& _id, std::string const& _value)
  -> void {
    expect(!_id.empty(), "key_store: key id must not be empty");
    auto _row = this->load_row(_id);
    auto _meta = this->parse_meta(_row);
    long long _version = _row("version")->ok() ? _row("version")->integer() + 1 : 1;
    long long _created = _row("created")->ok() ? _row("created")->integer() : now_millis();
    this->upsert(this->encrypt_row(_id, _value, _meta, _version, _created));
}

auto zpt::keystore::storage_key_store::remove(std::string const& _id) -> std::size_t {
    return this->__keys->remove({ "_id", _id })->execute()->count();
}

auto zpt::keystore::storage_key_store::has(std::string const& _id) -> bool {
    auto _rows = this->__keys->find({ "_id", _id })->execute()->fetch();
    return _rows.size() > 0;
}

auto zpt::keystore::storage_key_store::list() -> zpt::json {
    auto _rows = this->__keys->find("1 = 1")->execute()->fetch();
    auto _out = zpt::json::array();
    for (std::size_t _i = 0; _i < _rows.size(); ++_i) {
        auto _row = _rows(_i);
        _out << zpt::json{ "_id",     _row("_id")->string(),
                           "meta",    this->parse_meta(_row),
                           "version", _row("version")->ok() ? _row("version")->integer() : 0,
                           "created", _row("created")->ok() ? _row("created")->integer() : 0 };
    }
    return _out;
}

auto zpt::keystore::storage_key_store::clear() -> void { this->__keys->remove({})->execute(); }

auto zpt::KEY_STORE(std::string const& _passphrase) -> zpt::keystore::key_store& {
    static auto _global = std::make_shared<zpt::keystore::storage_key_store>(_passphrase);
    return *_global;
}
