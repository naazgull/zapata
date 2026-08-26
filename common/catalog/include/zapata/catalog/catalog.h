/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright inteautomaton in the software to the public
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
 * @file catalog.h
 * @brief SQLite-backed service catalog for handler registration and lookup.
 *
 * Provides a catalog that stores service/handler registrations in an in-memory
 * SQLite database, supporting pattern-based lookup (e.g., URI routing).
 * Used internally by the REST engine resolver for service discovery.
 *
 * @see zpt::events::resolver_t
 */

#pragma once

#include <deque>
#include <string>
#include <zapata/json.h>
#include <zapata/sqlite.h>

namespace zpt {
/**
 * @brief SQLite-backed service catalog with pattern matching.
 *
 * Stores key-value handler registrations in an in-memory SQLite database.
 * Keys support hierarchical pattern matching (e.g., "/api/users/{id}")
 * for URI-based routing.
 *
 * @tparam K Key type (typically std::string for URI paths).
 * @tparam M Metadata type (stored as serialized text).
 *
 * @par Example Usage
 * @code
 * zpt::catalog<std::string, zpt::json> cat("services", "node-1");
 * cat.add("/api/users", hash_code, zpt::json{ "handler", "users" });
 * auto matches = cat.resolve("/api/users");
 * @endcode
 */
template<typename K, typename M>
class catalog {
  public:
    using ptr = std::shared_ptr<catalog>;

    /**
     * @brief Constructs a catalog backed by an in-memory SQLite database.
     * @param _catalog_name Name for the SQLite database.
     * @param _self_id Provider ID for this node.
     */
    catalog(std::string const& _catalog_name, std::string const& _self_id);
    virtual ~catalog() = default;

    /**
     * @brief Removes all entries and providers.
     * @return Reference to this catalog.
     */
    auto clear() -> catalog&;
    /**
     * @brief Adds an entry using self as provider.
     * @param _key Entry key.
     * @param hash Entry hash code.
     * @param _metadata Entry metadata.
     * @return Reference to this catalog.
     */
    auto add(K _key, std::uint64_t hash, M _metadata) -> catalog&;
    /**
     * @brief Adds an entry with an explicit provider ID.
     * @param _key Entry key.
     * @param _provider_id Provider identifier.
     * @param _hash Entry hash code.
     * @param _metadata Entry metadata.
     * @return Reference to this catalog.
     */
    auto add(K _key, std::string const& _provider_id, std::uint64_t _hash, M _metadata) -> catalog&;
    /**
     * @brief Removes an entry by key.
     * @param _key Entry key to remove.
     * @param _hash Entry hash code.
     * @return Number of removed entries.
     */
    auto remove(K _key, std::uint64_t _hash) -> size_t;
    /**
     * @brief Resolves a pattern to matching entries (self provider only).
     * @param _pattern Pattern to match (supports {} placeholders).
     * @return JSON array of matching entries.
     */
    auto resolve(K const& _pattern) const -> zpt::json const;
    /**
     * @brief Searches for entries matching a pattern, optionally filtering by provider.
     * @param _pattern Pattern to match against entry keys.
     * @param _provider Optional provider ID to filter results.
     * @return JSON array of matching entries.
     */
    auto search(K const& _pattern, std::string const& _provider = "") const -> zpt::json const;
    /**
     * @brief Lists all entries for a provider (default: self).
     * @param _provider_id Provider ID to list entries for.
     * @return JSON array of all entries for the provider.
     */
    auto list(std::string const& _provider_id = "") const -> zpt::json const;

    /**
     * @brief Registers a service provider.
     * @param _id Provider identifier.
     * @param _info Provider information metadata.
     * @return Reference to this catalog.
     */
    auto add_provider(std::string const& _id, zpt::json const& _info) -> catalog&;
    /**
     * @brief Unregisters a service provider and its entries.
     * @param _id Provider ID to remove.
     * @return Reference to this catalog.
     */
    auto remove_provider(std::string const& _id) -> catalog&;
    /**
     * @brief Retrieves provider information by ID.
     * @param _id Provider ID to look up.
     * @return JSON object with provider details, or null if not found.
     */
    auto get_provider(std::string const& _id) const -> zpt::json;

  private:
    /** @brief This catalog's provider ID used for self-registered entries. */
    std::string __self_id;
    /** @brief SQLite database connection handle. */
    mutable zpt::storage::connection __connection;
    /** @brief Collection backing the catalog (key-value handler registrations). */
    mutable zpt::storage::collection __catalog;
    /** @brief Collection backing the provider registry. */
    mutable zpt::storage::collection __provider;

    /**
     * @brief Executes a raw query against the catalog table.
     * @param _query SQL-like query string.
     * @return JSON array of matching entries.
     */
    auto query(std::string const& _query) const -> zpt::json const;
};

namespace catalog_id {
template<typename K>
auto separator() -> std::string;
auto split(std::string const& _pattern) -> zpt::json;
} // namespace catalog_id
} // namespace zpt

template<typename K, typename M>
zpt::catalog<K, M>::catalog(std::string const& _catalog_name, std::string const& _self_id)
  : __self_id{ _self_id } {
    this->__connection = zpt::make_connection<zpt::storage::sqlite::connection>(zpt::undefined);
    auto _session = this->__connection->session();
    auto _database = _session->database(_catalog_name);

    sqlite3_exec(static_cast<zpt::storage::sqlite::database*>(&(*_database))->connection().get(), //
                 "CREATE TABLE IF NOT EXISTS catalog ("
                 "    _id TEXT,"
                 "    provider_id TEXT NOT NULL,"
                 "    hash INTEGER NOT NULL,"
                 "    pattern TEXT NOT NULL,"
                 "    metadata TEXT,"
                 "    PRIMARY KEY(_id, provider_id, hash),"
                 "    FOREIGN KEY(provider_id) REFERENCES provider(_id)"
                 ")",
                 nullptr,
                 nullptr,
                 nullptr);
    sqlite3_exec(static_cast<zpt::storage::sqlite::database*>(&(*_database))->connection().get(), //
                 "CREATE INDEX pattern_idx ON catalog (pattern)",
                 nullptr,
                 nullptr,
                 nullptr);
    sqlite3_exec(static_cast<zpt::storage::sqlite::database*>(&(*_database))->connection().get(), //
                 "CREATE TABLE IF NOT EXISTS provider ("
                 "    _id TEXT PRIMARY KEY,"
                 "    name TEXT NOT NULL,"
                 "    protocols TEXT NOT NULL"
                 ")",
                 nullptr,
                 nullptr,
                 nullptr);

    this->__catalog = _database->collection("catalog");
    this->__provider = _database->collection("provider");
}

template<typename K, typename M>
auto zpt::catalog<K, M>::clear() -> catalog& {
    this
      ->__catalog //
      ->remove({})
      ->execute();
    this
      ->__provider //
      ->remove({})
      ->execute();
    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::add(K _key, std::uint64_t _hash, M _metadata) -> catalog& {
    return this->add(_key, this->__self_id, _hash, _metadata);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::add(K _key,
                             std::string const& _provider_id,
                             std::uint64_t _hash,
                             M _metadata) -> catalog& {
    std::ostringstream _oss;
    _oss << _key << std::flush;
    std::string _t_key{ _oss.str() };
    _oss.str("");
    _oss << _metadata << std::flush;
    zpt::json _body{ "_id",         _t_key,
                     "provider_id", _provider_id,
                     "hash",        static_cast<long long int>(_hash),
                     "pattern",     zpt::r_replace(_t_key, "{}", "%"),
                     "metadata",    _oss.str() };

    if (static_cast<long long int>(_hash) != 0) { zlog("Registered " << _t_key, zpt::trace); }
    this
      ->__catalog //
      ->add(_body)
      ->execute();

    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::remove(K _key, std::uint64_t _hash) -> size_t {
    std::ostringstream _oss;
    _oss << _key << std::flush;
    std::string _t_key{ _oss.str() };

    if (static_cast<long long int>(_hash) != 0) { zlog("Unregistered " << _t_key, zpt::trace); }
    auto _removed = this
                      ->__catalog //
                      ->remove({ "_id", _t_key, "hash", _hash })
                      ->execute()
                      ->count();

    return _removed;
}

template<typename K, typename M>
auto zpt::catalog<K, M>::resolve(K const& _pattern) const -> zpt::json const {
    std::string _t_pattern;
    if constexpr (std::is_convertible<K, std::string>::value) { _t_pattern.assign(_pattern); }
    else {
        std::ostringstream _oss;
        _oss << _pattern << std::flush;
        _t_pattern.assign(_oss.str());
    }
    auto _parts = std::count(_t_pattern.begin(), _t_pattern.end(), '/');

    auto _query = std::format("('{}' like pattern) and "
                              "((length(pattern) - length(replace(pattern, '/', ''))) = {}) and "
                              "(hash <> 0)",
                              _t_pattern,
                              _parts);
    auto _result = this
                     ->__catalog //
                     ->find(_query)
                     ->fields({ zpt::array, "hash" })
                     ->execute()
                     ->fetch();

    return _result;
}

template<typename K, typename M>
auto zpt::catalog<K, M>::search(K const& _pattern, std::string const& _provider) const
  -> zpt::json const {
    std::string _t_pattern;
    if constexpr (std::is_convertible<K, std::string>::value) { _t_pattern.assign(_pattern); }
    else {
        std::ostringstream _oss;
        _oss << _pattern << std::flush;
        _t_pattern.assign(_oss.str());
    }
    auto _parts = std::count(_t_pattern.begin(), _t_pattern.end(), '/');

    auto _query = std::format(
      "('{}' like pattern) and ((length(pattern) - length(replace(pattern, '/', ''))) = {})",
      _t_pattern,
      _parts);
    if (_provider != "") { _query += std::format(" and (provider_id = '{}')", _provider); }

    auto _result = this
                     ->__catalog //
                     ->find(_query)
                     ->fields({ zpt::array, "_id", "provider_id", "hash", "metadata" })
                     ->execute()
                     ->fetch();

    std::istringstream _iss;
    for (auto&& [_, __, _service] : _result) {
        if (_service("metadata")->ok()) {
            M _metadata;
            _iss.str(_service("metadata")->string());
            _iss >> _metadata;
            _service["metadata"] = _metadata;
        }
    }

    return _result;
}

template<typename K, typename M>
auto zpt::catalog<K, M>::list(std::string const& _provider_id) const -> zpt::json const {
    auto _result =
      this
        ->__catalog //
        ->find({ "provider_id", _provider_id.empty() ? this->__self_id : _provider_id })
        ->fields({ zpt::array, "_id", "provider_id", "metadata" })
        ->execute()
        ->fetch();

    std::istringstream _iss;
    for (auto&& [_, __, _service] : _result) {
        if (_service("metadata")->ok()) {
            M _metadata;
            _iss.str(_service("metadata")->string());
            _iss >> _metadata;
            _service["metadata"] = _metadata;
        }
    }

    return _result;
}

template<typename K, typename M>
auto zpt::catalog<K, M>::add_provider(std::string const& _id, zpt::json const& _info) -> catalog& {
    auto _provider = _info->clone();

    _provider["_id"] = _id;
    if (!_provider("name")->ok()) { _provider["name"] = _id; }

    this
      ->__provider //
      ->add(_provider)
      ->execute();
    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::remove_provider(std::string const& _id) -> catalog& {
    this
      ->__catalog //
      ->remove({ "provider_id", _id })
      ->execute();
    this
      ->__provider //
      ->remove({ "_id", _id })
      ->execute();
    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::get_provider(std::string const& _id) const -> zpt::json {
    auto _providers = this
                        ->__provider //
                        ->find({ "_id", _id })
                        ->execute()
                        ->fetch();

    for (auto&& [_, __, _provider] : _providers) {
        if (_provider("protocols")->ok()) {
            _provider["protocols"] = zpt::json::parse_json_str(_provider("protocols")->string());
        }
    }

    return _providers;
}

template<typename K, typename M>
auto zpt::catalog<K, M>::query(std::string const& _query) const -> zpt::json const {
    return this
      ->__catalog //
      ->find(_query)
      ->execute()
      ->fetch();
}

template<typename K>
auto zpt::catalog_id::separator() -> std::string {
    if constexpr (std::is_same<K, std::string>::value) { return "/"; }
    return "";
}
