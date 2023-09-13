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

#pragma once

#include <string>
#include <deque>
#include <zapata/sqlite.h>
#include <zapata/json.h>

namespace {
static constexpr char const* SEARCH_STMT = "(_id like '{}{}{}%')";
static constexpr char const* EXACT_SEARCH_STMT = "((_id = '{}{}{}') or (_id = '{}{}{}'))";
} // namespace

namespace zpt {
auto CATALOG() -> ssize_t&;

template<typename K, typename M>
class catalog {
  public:
    catalog(std::string const& _catalog_name);
    virtual ~catalog() = default;

    auto clear() -> catalog&;
    auto add(K _key, M _metadata) -> catalog&;
    auto search(K const& _pattern) const -> zpt::json const;

  private:
    mutable zpt::storage::connection __connection;
    mutable zpt::storage::collection __catalog;

    auto query(std::string const& _query) const -> zpt::json const;
};

namespace catalog_id {
template<typename K>
auto separator() -> std::string;
auto split(std::string const& _pattern) -> zpt::json;
} // namespace catalog_id
} // namespace zpt

template<typename K, typename M>
zpt::catalog<K, M>::catalog(std::string const& _catalog_name) {
    this->__connection = zpt::make_connection<zpt::storage::sqlite::connection>(zpt::undefined);
    auto _session = this->__connection->session();
    auto _database = _session->database(_catalog_name);

    sqlite3_exec(static_cast<zpt::storage::sqlite::database*>(&(*_database))->connection().get(), //
                 "CREATE TABLE IF NOT EXISTS catalog ("
                 "    _id TEXT PRIMARY KEY,"
                 "    metadata TEXT NOT NULL"
                 ")",
                 nullptr,
                 nullptr,
                 nullptr);

    this->__catalog = _database->collection("catalog");
}

template<typename K, typename M>
auto zpt::catalog<K, M>::clear() -> catalog& {
    this
      ->__catalog //
      ->remove({})
      ->execute();
    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::add(K _key, M _metadata) -> catalog& {
    std::ostringstream _oss;
    _oss << _key << std::flush;
    std::string _t_key{ _oss.str() };
    _oss.str("");
    _oss << _metadata << std::flush;

    this
      ->__catalog //
      ->replace(_key, { "metadata", _oss.str() })
      ->execute();

    return (*this);
}

template<typename K, typename M>
auto zpt::catalog<K, M>::search(K const& _pattern) const -> zpt::json const {
    auto _separator = zpt::catalog_id::separator<K>();
    auto _parts = zpt::catalog_id::split(_pattern);
    zpt::json _result = zpt::json::array();
    zpt::json _prefixes{ zpt::array, "" };

    for (auto const& [_idx, __, _part] : _parts) {
        if (_idx == _parts->size() - 1) {
            for (auto [_, __, _prefix] : _prefixes) {
                _result += this->query(zpt::format(EXACT_SEARCH_STMT, //
                                                   _prefix->string(),
                                                   _separator,
                                                   _part->string(),
                                                   _prefix->string(),
                                                   _separator));
            }
            expect(_result->size() != 0, "Pattern '" << _pattern << "' not found.");
        }
        else {
            zpt::json _matching = zpt::json::array();

            for (auto [_, __, _prefix] : _prefixes) {
                if (this
                      ->query(zpt::format(SEARCH_STMT, //
                                          _prefix->string(),
                                          _separator,
                                          _part->string()))
                      ->size() != 0) {
                    _matching << (_prefix->string() + _separator + static_cast<std::string>(_part));
                }
                if (this
                      ->query(zpt::format(SEARCH_STMT, //
                                          _prefix->string(),
                                          _separator))
                      ->size() != 0) {
                    _matching << (_prefix->string() + _separator + std::string{ "{}" });
                }
            }

            expect(_matching->size() != 0, "Pattern '" << _pattern << "' not found.");
            _prefixes = _matching;
        }
    }

    std::istringstream _iss;
    for (auto [_, __, _row] : _result) {
        M _metadata;
        _iss.str(_row("metadata")->string());
        _iss >> _metadata;
        _iss.str("");
        _row["metadata"] = _metadata;
    }
    return _result;
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
