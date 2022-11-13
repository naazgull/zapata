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

namespace zpt {
auto CATALOGUE() -> ssize_t&;

template<typename K, typename M>
class catalogue {
  public:
    catalogue();
    virtual ~catalogue() = default;

    auto clear() -> catalogue&;
    auto add(K _key, M _metadata) -> catalogue&;
    auto search(K const& _pattern) -> std::deque<std::tuple<K, M>>;

  private:
    zpt::storage::connection __connection;
    zpt::storage::collection __catalogue;
};
} // namespace zpt

template<typename K, typename M>
zpt::catalogue<K, M>::catalogue() {
    this->__connection = zpt::make_connection<zpt::storage::sqlite::connection>(zpt::undefined);
    auto _session = this->__connection->session();
    auto _database = _session->database("catalogue");

    sqlite3_exec(static_cast<zpt::storage::sqlite::database*>(&(*_database))->connection().get(), //
                 "CREATE TABLE IF NOT EXISTS catalogue ("
                 "    _id TEXT PRIMARY KEY,"
                 "    metadata TEXT NOT NULL"
                 ")",
                 nullptr,
                 nullptr,
                 nullptr);

    this->__catalogue = _database->collection("catalogue");
}

template<typename K, typename M>
auto zpt::catalogue<K, M>::clear() -> catalogue& {
    this
      ->__catalogue //
      ->remove({})
      ->execute();
    return (*this);
}

template<typename K, typename M>
auto zpt::catalogue<K, M>::add(K _key, M _metadata) -> catalogue& {
    std::ostringstream _oss;
    _oss << _key << std::flush;
    std::string _t_key{ _oss.str() };
    _oss.str("");
    _oss << _metadata << std::flush;

    this
      ->__catalogue //
      ->replace(_key, { "metadata", _oss.str() })
      ->execute();

    return (*this);
}

template<typename K, typename M>
auto zpt::catalogue<K, M>::search(K const& _pattern) -> std::deque<std::tuple<K, M>> {
    auto _result = this
                     ->__catalogue //
                     ->find({ "_id", _pattern })
                     ->execute();

    std::deque<std::tuple<K, M>> _to_return;
    for (auto _row = _result->fetch(1); _row != zpt::undefined; _row = _result->fetch(1)) {
        M _value;
        std::stringstream _ss;
        _ss << _row["metadata"]->string() << std::flush;
        _ss >> _value;
        _to_return.push_back({ static_cast<K>(_row["_id"]), _value });
    }
    return _to_return;
}
