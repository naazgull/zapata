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

namespace {
static constexpr char const* SEARCH_STMT = "(_id like '{}{}{}%')";
static constexpr char const* EXACT_SEARCH_STMT = "((_id = '{}{}{}') or (_id = '{}{}{}'))";
static constexpr char const* SEARCH_WITH_PROVIDER_STMT =
  "(_id like '{}{}{}%') and (provider_id = '{}')";
static constexpr char const* EXACT_SEARCH_WITH_PROVIDER_STMT =
  "((_id = '{}{}{}') or (_id = '{}{}{}')) and (provider_id = '{}')";
static constexpr char const* RESOLVE_STMT = "(_id like '{}{}{}%') and (provider_id = '{}')";
static constexpr char const* EXACT_RESOLVE_STMT =
  "((_id = '{}{}{}') or (_id = '{}{}{}')) and (provider_id = '{}')";
} // namespace

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

    /** @brief Removes all entries and providers. */
    auto clear() -> catalog&;
    /** @brief Adds an entry using self as provider. */
    auto add(K _key, std::uint64_t hash, M _metadata) -> catalog&;
    /** @brief Adds an entry with an explicit provider ID. */
    auto add(K _key, std::string const& _provider_id, std::uint64_t hash, M _metadata) -> catalog&;
    /** @brief Removes an entry by key. */
    auto remove(K _key) -> catalog&;
    /**
     * @brief Resolves a pattern to matching entries (self provider only).
     * @param _pattern Pattern to match (supports {} placeholders).
     * @return JSON array of matching entries.
     */
    auto resolve(K const& _pattern) const -> zpt::json const;
    /** @brief Searches for entries matching a pattern, optionally filtering by provider. */
    auto search(K const& _pattern, std::string const& _provider = "") const -> zpt::json const;
    /** @brief Lists all entries for a provider (default: self). */
    auto list(std::string const& _provider_id = "") const -> zpt::json const;

    /** @brief Registers a service provider. */
    auto add_provider(std::string const& _id, zpt::json const& _info) -> catalog&;
    /** @brief Unregisters a service provider and its entries. */
    auto remove_provider(std::string const& _id) -> catalog&;
    /** @brief Retrieves provider information by ID. */
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
                 "    _id TEXT PRIMARY KEY,"
                 "    provider_id TEXT NOT NULL,"
                 "    hash INTEGER NOT NULL,"
                 "    metadata TEXT,"
                 "    FOREIGN KEY(provider_id) REFERENCES provider(_id)"
                 ")",
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
    zpt::json _body{ "provider_id", _provider_id, "hash", _hash, "metadata", _oss.str() };

    zlog("Registered " << _t_key, zpt::trace);
    this
      ->__catalog //
      ->replace(_t_key, _body)
      ->execute();

    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::remove(K _key) -> catalog& {
    std::ostringstream _oss;
    _oss << _key << std::flush;
    std::string _t_key{ _oss.str() };

    zlog("Unregistered " << _t_key, zpt::trace);
    this
      ->__catalog //
      ->remove({ "_id", _t_key })
      ->execute();

    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::resolve(K const& _pattern) const -> zpt::json const {
    auto _separator = zpt::catalog_id::separator<K>();
    auto _parts = zpt::catalog_id::split(_pattern);
    zpt::json _result = zpt::json::array();
    zpt::json _prefixes{ zpt::array, "" };

    for (auto const& [_idx, __, _part] : _parts) {
        if (_idx == _parts->size() - 1) {
            for (auto&& [_, __, _prefix] : _prefixes) {
                _result += this
                             ->__catalog                            //
                             ->find(std::format(EXACT_RESOLVE_STMT, //
                                                _prefix->string(),
                                                _separator,
                                                _part->string(),
                                                _prefix->string(),
                                                _separator,
                                                "{}",
                                                this->__self_id))
                             ->fields({ zpt::array, "hash" })
                             ->execute()
                             ->fetch();
            }
            expect(_result->size() != 0, "Pattern '" << _pattern << "' not found.");
        }
        else {
            zpt::json _matching = zpt::json::array();

            for (auto&& [_, __, _prefix] : _prefixes) {
                auto _count = this
                                ->__catalog                      //
                                ->find(std::format(RESOLVE_STMT, //
                                                   _prefix->string(),
                                                   _separator,
                                                   _part->string(),
                                                   this->__self_id))
                                ->fields({ zpt::array, zpt::storage::sql_functions::COUNT })
                                ->execute()
                                ->fetch();
                if (_count->ok() && _count(0)("count(*)")->integer() != 0) {
                    _matching << (_prefix->string() + _separator + static_cast<std::string>(_part));
                }

                _count = this
                           ->__catalog                      //
                           ->find(std::format(RESOLVE_STMT, //
                                              _prefix->string(),
                                              _separator,
                                              "{}",
                                              this->__self_id))
                           ->fields({ zpt::array, zpt::storage::sql_functions::COUNT })
                           ->execute()
                           ->fetch();
                if (_count->ok() && _count(0)("count(*)")->integer() != 0) {
                    _matching << (_prefix->string() + _separator + std::string{ "{}" });
                }
            }

            if (_matching->size() == 0) { continue; }
            _prefixes = _matching;
        }
    }

    return _result;
}

template<typename K, typename M>
auto zpt::catalog<K, M>::search(K const& _pattern, std::string const& _provider) const
  -> zpt::json const {
    auto _separator = zpt::catalog_id::separator<K>();
    auto _parts = zpt::catalog_id::split(_pattern);
    zpt::json _result = zpt::json::array();
    zpt::json _prefixes{ zpt::array, "" };
    std::string _search;
    std::string _exact_search;

    if (_provider.length() != 0) {
        _search = SEARCH_WITH_PROVIDER_STMT;
        _exact_search = EXACT_SEARCH_WITH_PROVIDER_STMT;
    }
    else {
        _search = SEARCH_STMT;
        _exact_search = EXACT_SEARCH_STMT;
    }

    for (auto const& [_idx, __, _part] : _parts) {
        if (_idx == _parts->size() - 1) {
            for (auto&& [_, __, _prefix] : _prefixes) {
                _result += this
                             ->__catalog                        //
                             ->find(std::vformat(_exact_search, //
                                                 std::make_format_args(_prefix->string(),
                                                                       _separator,
                                                                       _part->string(),
                                                                       _prefix->string(),
                                                                       _separator,
                                                                       "{}",
                                                                       _provider)))
                             ->execute()
                             ->fetch();
            }
            expect(_result->size() != 0, "Pattern '" << _pattern << "' not found.");
        }
        else {
            zpt::json _matching = zpt::json::array();

            for (auto&& [_, __, _prefix] : _prefixes) {
                auto _count = this
                                ->__catalog                                                  //
                                ->find(std::vformat(_search,                                 //
                                                    std::make_format_args(_prefix->string(), //
                                                                          _separator,
                                                                          _part->string(),
                                                                          _provider)))
                                ->fields({ zpt::array, zpt::storage::sql_functions::COUNT })
                                ->execute()
                                ->fetch();
                if (_count->ok() && _count(0)("count(*)")->integer() != 0) {
                    _matching << (_prefix->string() + _separator + static_cast<std::string>(_part));
                }

                _count = this
                           ->__catalog                                                  //
                           ->find(std::vformat(_search,                                 //
                                               std::make_format_args(_prefix->string(), //
                                                                     _separator,
                                                                     "{}",
                                                                     _provider)))
                           ->fields({ zpt::array, zpt::storage::sql_functions::COUNT })
                           ->execute()
                           ->fetch();
                if (_count->ok() && _count(0)("count(*)")->integer() != 0) {
                    _matching << (_prefix->string() + _separator + std::string{ "{}" });
                }
            }

            expect(_matching->size() != 0, "Pattern '" << _pattern << "' not found.");
            _prefixes = _matching;
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
        _service->object()->pop("hash");
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
