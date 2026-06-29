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

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <zapata/pgsql/translate.h>

// ---- result_set_metadata (lightweight, for PGresult columns) ----

zpt::storage::pgsql::result_set_metadata::result_set_metadata(PGresult* _result)
  : __metadata{ _result } {
    if (_result != nullptr) { this->__column_count = PQnfields(_result); }
}

zpt::storage::pgsql::result_set_metadata::result_set_metadata(result_set_metadata&& rhs)
  : __column_count{ rhs.__column_count } {}

zpt::storage::pgsql::result_set_metadata::~result_set_metadata() {}

auto zpt::storage::pgsql::result_set_metadata::operator=(result_set_metadata&& rhs)
  -> result_set_metadata& {
    this->__column_count = rhs.__column_count;
    return (*this);
}

auto zpt::storage::pgsql::result_set_metadata::name(size_t _column) const -> std::string {
    return std::string{ PQfname(this->__metadata, static_cast<int>(_column)) };
}

auto zpt::storage::pgsql::result_set_metadata::type(size_t _column) const -> Oid {
    return PQftype(this->__metadata, static_cast<int>(_column));
}

auto zpt::storage::pgsql::result_set_metadata::tableoid([[maybe_unused]] size_t _column) const
  -> Oid {
    // libpq 13+ has PQftablecollation; older versions have PQftableoid.
    // This function is only needed for type introspection and not used in core paths.
    return 0;
}

auto zpt::storage::pgsql::result_set_metadata::column_size(size_t _column) const -> int16_t {
    return PQfsize(this->__metadata, static_cast<int>(_column));
}

auto zpt::storage::pgsql::result_set_metadata::is_binary(size_t _column) const -> bool {
    return PQfformat(this->__metadata, static_cast<int>(_column)) == 1;
}

auto zpt::storage::pgsql::result_set_metadata::get_string(PGresult* _result,
                                                          int _row,
                                                          int _column) const -> std::string {
    if (PQgetisnull(_result, _row, _column)) { return {}; }
    auto* _val = PQgetvalue(_result, _row, _column);
    int _len = PQgetlength(_result, _row, _column);
    return std::string{ _val, static_cast<size_t>(_len) };
}

// ---- to_json: convert PGresult row to JSON object ----

auto zpt::storage::pgsql::to_json(
  PGresult* _result,
  [[maybe_unused]] zpt::storage::pgsql::result_set_metadata const& _cols,
  int _row) -> zpt::json {
    auto _record = zpt::json::object();
    int _ncols = PQnfields(_result);

    for (int _col_idx = 0; _col_idx < _ncols; ++_col_idx) {
        auto _name = std::string{ PQfname(_result, _col_idx) };

        if (PQgetisnull(_result, _row, _col_idx)) {
            _record[_name] = zpt::undefined;
            continue;
        }

        auto* _val = PQgetvalue(_result, _row, _col_idx);
        int _len = PQgetlength(_result, _row, _col_idx);

        // OID type codes (from postgres_ext.h)
        Oid _type = PQftype(_result, _col_idx);

        // Try to detect JSON/JSONB columns and parse them
        if (_type == 114 || _type == 3802) { // JSON or JSONB
            std::string _str{ _val, static_cast<size_t>(_len) };
            _record[_name] = zpt::json::parse_json_str(_str);
            continue;
        }

        // String-like types
        if (_type == 25 || _type == 1043 || _type == 1009 ||
            _type == 17) { // text, varchar, name, bytea
            _record[_name] = std::string{ _val, static_cast<size_t>(_len) };
            continue;
        }

        // Boolean
        if (_type == 16) {
            _record[_name] = (_val[0] == 't');
            continue;
        }

        // Integer types
        if (_type == 20) { // int8 / bigint
            _record[_name] = std::stoll(std::string{ _val });
            continue;
        }
        if (_type == 21) { // int2 / smallint
            _record[_name] = std::stoi(std::string{ _val });
            continue;
        }
        if (_type == 23) { // int4 / integer
            _record[_name] = std::stoi(std::string{ _val });
            continue;
        }
        if (_type == 600 || _type == 601) { // point
            _record[_name] = std::string{ _val, static_cast<size_t>(_len) };
            continue;
        }

        // Float types
        if (_type == 700) { // float4
            _record[_name] = std::stof(std::string{ _val });
            continue;
        }
        if (_type == 701) { // float8
            _record[_name] = std::stod(std::string{ _val });
            continue;
        }

        // Date/time types (always returned as strings by libpq)
        if (_type == 1082 || _type == 1083 || _type == 1114 || _type == 1184 ||
            _type == 1186) { // date, time, timestamp, timestamptz, interval
            _record[_name] = std::string{ _val, static_cast<size_t>(_len) };
            continue;
        }

        // Default: treat as string
        _record[_name] = std::string{ _val, static_cast<size_t>(_len) };
    }

    return _record;
}

// Convenience overload: no metadata needed since PQfname/PQftype work directly
auto zpt::storage::pgsql::to_json(PGresult* _result, int _row) -> zpt::json {
    return to_json(_result, result_set_metadata{ _result }, _row);
}

// ---- SQL generation ----

auto zpt::storage::pgsql::to_query(zpt::json _fields, zpt::json _filter) -> std::string {
    std::ostringstream _oss;

    _oss << "SELECT ";
    if (_fields->ok() && _fields->size() != 0) {
        bool _first{ true };
        for (auto const& [_, __, _field] : _fields) {
            if (!_first) { _oss << ", "; }
            _first = false;
            _oss << "\"" << static_cast<std::string>(_field) << "\"";
        }
    }
    else { _oss << "*"; }
    _oss << " FROM \"{}\".\"{}\"";

    if (_filter->ok() && _filter->string().length() != 0) {
        _oss << " WHERE " << _filter->string();
    }

    return _oss.str();
}

auto zpt::storage::pgsql::to_insert(zpt::json _to_insert) -> std::string {
    std::ostringstream _oss;

    // Collect column names and values
    _oss << "INSERT INTO \"{}\".\"{}\" (";
    bool _first{ true };
    for (auto const& [_, _key, _value] : _to_insert) {
        if (!_first) { _oss << ", "; }
        _first = false;
        _oss << "\"" << static_cast<std::string>(_key) << "\"";
    }
    _oss << ") VALUES (";
    _first = true;
    for (auto const& [_, _key, _value] : _to_insert) {
        if (!_first) { _oss << ", "; }
        _first = false;
        _oss << quote(_value);
    }
    _oss << ");";

    return _oss.str();
}

auto zpt::storage::pgsql::to_update(zpt::json _to_update, zpt::json _pattern) -> std::string {
    std::ostringstream _oss;

    _oss << "UPDATE \"{}\".\"{}\" SET ";
    to_assignment_list(_to_update, _oss, ", ");
    if (_pattern->ok() && _pattern->string().length() != 0) {
        _oss << " WHERE " << _pattern->string();
    }
    _oss << ";";

    return _oss.str();
}

auto zpt::storage::pgsql::to_upsert(zpt::json _to_upsert) -> std::string {
    std::ostringstream _oss;

    // Collect column names
    std::vector<std::string> _keys;
    bool _first{ true };
    for (auto const& [_, _key, _value] : _to_upsert) {
        _keys.push_back(static_cast<std::string>(_key));
        if (!_first) { _oss << ", "; }
        _first = false;
    }

    _oss << "INSERT INTO \"{}\".\"{}\" (";
    _first = true;
    for (auto const& _k : _keys) {
        if (!_first) { _oss << ", "; }
        _first = false;
        _oss << "\"" << _k << "\"";
    }
    _oss << ") VALUES (";
    _first = true;
    for (auto const& [_, _key, _value] : _to_upsert) {
        if (!_first) { _oss << ", "; }
        _first = false;
        _oss << quote(_value);
    }
    _oss << ") ON CONFLICT (_id) DO UPDATE SET ";

    _first = true;
    for (auto const& [_, _key, _value] : _to_upsert) {
        if (_key == "_id") { continue; }
        if (!_first) { _oss << ", "; }
        _first = false;
        _oss << "\"" << _key << "\" = EXCLUDED.\"";
        _oss << _key << "\"";
    }
    _oss << ";";

    return _oss.str();
}

auto zpt::storage::pgsql::to_delete(zpt::json _pattern) -> std::string {
    std::ostringstream _oss;

    _oss << "DELETE FROM \"{}\".\"{}\"";
    if (_pattern->ok() && _pattern->string().length() != 0) {
        _oss << " WHERE " << _pattern->string();
    }
    _oss << ";";

    return _oss.str();
}

auto zpt::storage::pgsql::to_assignment_list(zpt::json _to_convert,
                                             std::ostream& _out,
                                             std::string_view _separator) -> void {
    bool _first{ true };
    for (auto const& [_, _key, _value] : _to_convert) {
        if (!_first) { _out << _separator; }
        _first = false;
        _out << "\"" << static_cast<std::string>(_key) << "\" = " << quote(_value);
    }
}

// ---- quoting ----

auto zpt::storage::pgsql::quote([[maybe_unused]] PGconn* _conn, zpt::json _to_quote)
  -> std::string {
    bool _needs = _to_quote->type() == zpt::JSString || _to_quote->type() == zpt::JSDate ||
                  _to_quote->type() == zpt::JSRegex || _to_quote->type() == zpt::JSArray ||
                  _to_quote->type() == zpt::JSObject;

    if (!_to_quote->ok()) { return "NULL"; }

    std::string _str = static_cast<std::string>(_to_quote);

    if (!_needs) {
        // Numeric, boolean, etc. — pass through
        return _str;
    }

    std::ostringstream _oss;
    _oss << (_needs ? "'" : "")
         << (_to_quote->ok() ? zpt::r_replace_multiple(static_cast<std::string>(_to_quote),
                                                       { "'", "{", "}" },
                                                       { "''", "{{", "}}" })
                             : "NULL")
         << (_needs ? "'" : "") << std::flush;

    return _oss.str();
}

auto zpt::storage::pgsql::quote(zpt::json _to_quote) -> std::string {
    return quote(nullptr, _to_quote);
}
