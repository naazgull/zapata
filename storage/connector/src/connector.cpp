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
 * @file connector.cpp
 * @brief Implementation of storage connector interfaces and helper functions.
 *
 * Implements the wrapper class constructors/operators for connection, session,
 * database, collection, action, and result types. Also provides SQL generation
 * helpers and JSON-to-SQL conversion utilities.
 *
 * @see zpt::storage::connection
 * @see zpt::storage::session
 * @see zpt::storage::database
 * @see zpt::storage::collection
 * @see zpt::storage::action
 * @see zpt::storage::result
 */

#include <sstream>
#include <zapata/connector/connector.h>
#include <zapata/functional.h>

namespace {
/** @brief Outputs a variable/column name to the SQL output stream.
 * @param _variable JSON value containing the variable/column name string.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto variable_name(zpt::json _variable, std::ostream& _find) -> void;
/** @brief Outputs a value to the SQL stream, quoting strings/objects/arrays with single quotes.
 * @param _value JSON value to output.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto value_output(zpt::json _value, std::ostream& _find) -> void;
/** @brief Default functor handler: generates `column = functor(value, ...)` SQL.
 *
 * Used when a functor name is not found in the registered functors map.
 *
 * @param _functor Functor name (e.g., "my_func").
 * @param _params JSON array: index 0 is the column, remaining indices are arguments.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_default(std::string const& _functor, zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL lowercase comparison: `lower(col) = ?` (two params) or `lower(col)` (one param).
 *
 * @param _params JSON array: index 0 is the column, index 1 is the value to compare.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_lower(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL uppercase comparison: `upper(col) = ?` (two params) or `upper(col)` (one param).
 *
 * @param _params JSON array: index 0 is the column, index 1 is the value to compare.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_upper(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL boolean cast: `col = cast(? as boolean)` (two params) or `cast(? as boolean)` (one
 * param).
 *
 * @param _params JSON array: index 0 is the column (comparison only), index 1 is the value to cast.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_boolean(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL datetime cast: `col = cast(? as datetime(3))` (two params) or `cast(? as
 * datetime(3))` (one param).
 *
 * @param _params JSON array: index 0 is the column (comparison only), index 1 is the value to cast.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_date(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL integer cast: `col = cast(? as integer)` (two params) or `cast(? as integer)` (one
 * param).
 *
 * @param _params JSON array: index 0 is the column (comparison only), index 1 is the value to cast.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_integer(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL double cast: `col = cast(? as double)` (two params) or `cast(? as double)` (one
 * param).
 *
 * @param _params JSON array: index 0 is the column (comparison only), index 1 is the value to cast.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_floating(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL char cast: `col = cast(? as char)` (two params) or `cast(? as char)` (one param).
 *
 * @param _params JSON array: index 0 is the column (comparison only), index 1 is the value to cast.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_string(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL not-equal comparison: `(col <> ?)`.
 *
 * @param _params JSON array: index 0 is the column, index 1 is the value to compare.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_ne(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL greater-than comparison: `(col > ?)`.
 *
 * @param _params JSON array: index 0 is the column, index 1 is the value to compare.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_gt(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL greater-than-or-equal comparison: `(col >= ?)`.
 *
 * @param _params JSON array: index 0 is the column, index 1 is the value to compare.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_gte(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL less-than comparison: `(col < ?)`.
 *
 * @param _params JSON array: index 0 is the column, index 1 is the value to compare.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_lt(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL less-than-or-equal comparison: `(col <= ?)`.
 *
 * @param _params JSON array: index 0 is the column, index 1 is the value to compare.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_lte(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL between clause: `(col > ? and col < ?)`.
 *
 * @param _params JSON array: index 0 is the column, index 1 is the lower bound, index 2 is the
 * upper bound.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_between(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL LIKE operator: case-sensitive `(col like ?)` or case-insensitive `(lower(col) like
 * lower(?))`.
 *
 * When `_params[2]` equals `"i"`, the comparison is case-insensitive.
 *
 * @param _params JSON array: index 0 is the column, index 1 is the pattern, index 2 is the case
 * flag (`"i"` for case-insensitive).
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_like(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL IN clause: `(col in (?, ?, ...))`.
 *
 * @param _params JSON array: index 0 is the column, remaining indices are the values.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_in(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL IS clause: `(col is NULL)`.
 *
 * @param _params JSON array: index 0 is the column, remaining index is value.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_is(zpt::json _params, std::ostream& _find) -> void;
/** @brief SQL NOT clause: `not(expr)`.
 *
 * @param _params JSON value.
 * @param _find Output stream to write the SQL fragment to.
 * @return void */
auto func_not(zpt::json _params, std::ostream& _find) -> void;
/** @brief Returns the static map of registered SQL functor implementations.
 *
 * Maps functor names to their corresponding SQL generation functions.
 * Includes: lower, upper, boolean, date, integer, float, double, string,
 * ne, gt, gte, lt, lte, between, like, in.
 *
 * @return Reference to the static functor map. */
auto functors() -> std::map<std::string, zpt::storage::functor>&;
/** @brief Returns the static map of registered SQL connector providers.
 *
 * @return Reference to the static functor map. */
auto providers() -> std::map<std::string, std::function<zpt::storage::connection(zpt::json)>>&;
} // namespace

zpt::storage::connection::connection(zpt::storage::connection const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::connection::connection(zpt::storage::connection&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::connection::connection(zpt::storage::connection::type* _underlying)
  : __underlying{ _underlying } {}

auto zpt::storage::connection::operator=(zpt::storage::connection const& _rhs)
  -> zpt::storage::connection& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::storage::connection::operator=(zpt::storage::connection&& _rhs)
  -> zpt::storage::connection& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto zpt::storage::connection::operator->() -> zpt::storage::connection::type* {
    return this->__underlying.get();
}

auto zpt::storage::connection::operator*() -> zpt::storage::connection::type& {
    return *this->__underlying.get();
}

zpt::storage::session::session(zpt::storage::session const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::session::session(zpt::storage::session&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::session::session(zpt::storage::session::type* _underlying)
  : __underlying{ _underlying } {}

auto zpt::storage::session::operator=(zpt::storage::session const& _rhs) -> zpt::storage::session& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::storage::session::operator=(zpt::storage::session&& _rhs) -> zpt::storage::session& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto zpt::storage::session::operator->() -> zpt::storage::session::type* {
    return this->__underlying.get();
}

auto zpt::storage::session::operator*() -> zpt::storage::session::type& {
    return *this->__underlying.get();
}

zpt::storage::database::database(zpt::storage::database const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::database::database(zpt::storage::database&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::database::database(zpt::storage::database::type* _underlying)
  : __underlying{ _underlying } {}

auto zpt::storage::database::operator=(zpt::storage::database const& _rhs)
  -> zpt::storage::database& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::storage::database::operator=(zpt::storage::database&& _rhs) -> zpt::storage::database& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto zpt::storage::database::operator->() -> zpt::storage::database::type* {
    return this->__underlying.get();
}

auto zpt::storage::database::operator*() -> zpt::storage::database::type& {
    return *this->__underlying.get();
}

zpt::storage::collection::collection(zpt::storage::collection const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::collection::collection(zpt::storage::collection&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::collection::collection(zpt::storage::collection::type* _underlying)
  : __underlying{ _underlying } {}

auto zpt::storage::collection::operator=(zpt::storage::collection const& _rhs)
  -> zpt::storage::collection& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::storage::collection::operator=(zpt::storage::collection&& _rhs)
  -> zpt::storage::collection& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto zpt::storage::collection::operator->() -> zpt::storage::collection::type* {
    return this->__underlying.get();
}

auto zpt::storage::collection::operator*() -> zpt::storage::collection::type& {
    return *this->__underlying.get();
}

zpt::storage::action::action(zpt::storage::action const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::action::action(zpt::storage::action&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::action::action(zpt::storage::action::type* _underlying)
  : __underlying{ _underlying } {}

auto zpt::storage::action::operator=(zpt::storage::action const& _rhs) -> zpt::storage::action& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::storage::action::operator=(zpt::storage::action&& _rhs) -> zpt::storage::action& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto zpt::storage::action::operator->() -> zpt::storage::action::type* {
    return this->__underlying.get();
}

auto zpt::storage::action::operator*() -> zpt::storage::action::type& {
    return *this->__underlying.get();
}

zpt::storage::result::result(zpt::storage::result const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::storage::result::result(zpt::storage::result&& _rhs)
  : __underlying{ nullptr } {
    this->__underlying.swap(_rhs.__underlying);
}

zpt::storage::result::result(zpt::storage::result::type* _underlying)
  : __underlying{ _underlying } {}

auto zpt::storage::result::operator=(zpt::storage::result const& _rhs) -> zpt::storage::result& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::storage::result::operator=(zpt::storage::result&& _rhs) -> zpt::storage::result& {
    this->__underlying.swap(_rhs.__underlying);
    return (*this);
}

auto zpt::storage::result::operator->() -> zpt::storage::result::type* {
    return this->__underlying.get();
}

auto zpt::storage::result::operator*() -> zpt::storage::result::type& {
    return *this->__underlying.get();
}

auto zpt::storage::filter_remove(zpt::storage::collection& _collection, zpt::json _to_remove)
  -> zpt::storage::action {
    if (_to_remove->ok()) {
        auto _remove = _collection->remove(zpt::storage::extract_find(_to_remove));
        if (_to_remove("page_size")->ok()) {
            _remove //
              ->limit(_to_remove("page_size"));
        }
        if (_to_remove("page_start_index")->ok()) {
            _remove //
              ->offset(_to_remove("page_start_index"));
        }
        if (_to_remove("order_by")->ok()) {
            auto _sort = zpt::split(_to_remove("order_by")->string(), ",");
            for (auto&& [_, __, _expr] : _sort) {
                auto _name = _expr->string();
                bool _asc{ true };
                if (_name[0] == '-') {
                    _asc = false;
                    _name = _name.substr(1);
                }
                else if (_name[0] == '+') { _name = _name.substr(1); }
                _remove //
                  ->sort(_name, _asc);
            }
        }
        return _remove;
    }
    return _collection->remove({});
}

auto zpt::storage::filter_modify(zpt::storage::collection& _collection, zpt::json _to_modify)
  -> zpt::storage::action {
    if (_to_modify->ok()) {
        auto _modify = _collection->modify(zpt::storage::extract_find(_to_modify));
        return _modify;
    }
    return _collection->modify({});
}

auto zpt::storage::filter_find(zpt::storage::collection& _collection, zpt::json _to_find)
  -> zpt::storage::action {
    if (_to_find->ok()) {
        auto _find = _collection->find(zpt::storage::extract_find(_to_find));
        if (_to_find("page_size")->ok()) {
            _find //
              ->limit(_to_find("page_size"));
        }
        if (_to_find("page_start_index")->ok()) {
            _find //
              ->offset(_to_find("page_start_index"));
        }
        if (_to_find("order_by")->ok()) {
            auto _sort = zpt::split(_to_find("order_by")->string(), ",");
            for (auto&& [_, __, _expr] : _sort) {
                auto _name = _expr->string();
                bool _asc{ true };
                if (_name[0] == '-') {
                    _asc = false;
                    _name = _name.substr(1);
                }
                else if (_name[0] == '+') { _name = _name.substr(1); }
                _find //
                  ->sort(_name, _asc);
            }
        }
        return _find;
    }
    return _collection->find({});
}

auto zpt::storage::reply_find(zpt::json& _reply, zpt::json _params) -> void {
    if (_params->ok()) {
        if (_params("page_size")->ok()) {
            _reply << "page_size" << static_cast<long long>(_params("page_size"));
        }
        if (_params("page_start_index")->ok()) {
            _reply << "page_start_index" << static_cast<long long>(_params("page_start_index"));
        }
    }
}

auto zpt::storage::extract_find(zpt::json _to_process) -> std::string {
    if (!_to_process->is_object()) { return static_cast<std::string>(_to_process); }

    std::ostringstream _find;
    bool _first{ true };
    for (auto&& [_, _key, _value] : _to_process) {
        if (_key == "page_size" || _key == "page_start_index" || _key == "fields" ||
            _key == "order_by") {
            continue;
        }
        if (!_first) { _find << " and "; }
        _first = false;

        if (_value->is_string()) {
            auto _string = _value->string();
            if (_string.find("{.") == 0) {
                try {
                    auto _to_eval = _string.substr(2, _string.length() - 4);
                    auto _function = zpt::functional::parse(_to_eval);
                    auto _params = _function("params");
                    if (_params->ok()) {
                        (**_params->array())
                          .insert((**_params->array()).begin(), zpt::json::string(_key));
                    }
                    else { _function << "params" << _key; }
                    zpt::storage::functional_to_sql(_function, _find, ::variable_name);
                }
                catch (...) {
                }
            }
            else {
                _find << "(" << _key << " = ";
                ::value_output(_value, _find);
                _find << ")";
            }
        }
        else {
            _find << "(" << _key << " = ";
            ::value_output(_value, _find);
            _find << ")";
        }
    }
    _find << std::flush;
    return _find.str();
}

auto zpt::storage::functional_to_sql(zpt::json _function,
                                     std::ostream& _find,
                                     zpt::storage::string_output _str_output) -> void {
    if (!_function->ok()) { return; }
    if (!_function->is_object()) {
        _str_output(_function, _find);
        return;
    }
    zpt::storage::functor_to_sql(_function("functor")->string(), _function("params"), _find);
}

auto zpt::storage::functor_to_sql(std::string const& _functor,
                                  zpt::json _params,
                                  std::ostream& _find) -> void {
    auto _found = ::functors().find(_functor);
    if (_found != ::functors().end()) { _found->second(_params, _find); }
    else { ::func_default(_functor, _params, _find); }
}

auto zpt::storage::default_connector(zpt::json const& _config) -> std::string {
    return _config("storage")("order")("find")(0)->string();
}

auto zpt::storage::default_database(zpt::json const& _config, std::string const& _connector)
  -> std::string {
    return _config("storage")(_connector.empty() ? _config("storage")("order")("find")(0)->string()
                                                 : _connector)("database")
      ->string();
}

auto zpt::register_connector(std::string const& _key,
                             std::function<zpt::storage::connection(zpt::json)> _callback) -> void {
    auto& _providers = ::providers();
    _providers.insert(std::make_pair(_key, _callback));
}

auto zpt::make_connection(zpt::json const& _config, std::string const& _connector)
  -> zpt::storage::connection {
    auto& _providers = ::providers();
    auto _found = _providers.find(
      _connector.empty() ? _config("storage")("order")("find")(0)->string() : _connector);
    expect(_found != _providers.end(),
           "No such connector identified by " << _config("storage")("default"));
    return _found->second(_config);
}

namespace {
auto variable_name(zpt::json _variable, std::ostream& _find) -> void {
    _find << static_cast<std::string>(_variable) << std::flush;
}

auto value_output(zpt::json _value, std::ostream& _find) -> void {
    if (_value->is_string() || _value->is_object() || _value->is_array()) {
        auto _sv = static_cast<std::string>(_value);
        if (_sv == "null") { _find << _sv << std::flush; }
        else { _find << "'" << _sv << "'" << std::flush; }
    }
    else if (!_value->ok()) { _find << "null"; }
    else { _find << _value << std::flush; }
}

auto func_default(std::string const& _functor, zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " = " << _functor << "(";
    size_t _idx{ 0 };
    for (auto&& [_, __, _value] : _params) {
        if (_idx != 0) {
            if (_idx != 1) { _find << ", "; }
            zpt::storage::functional_to_sql(_value, _find, ::value_output);
        }
        ++_idx;
    }
    _find << "))" << std::flush;
}

auto func_lower(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() > 1) {
        _find << "(lower(" << static_cast<std::string>(_params(0)) << ") = ";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << ")" << std::flush;
    }
    else {
        _find << "lower(";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << ")" << std::flush;
    }
}

auto func_upper(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() > 1) {
        _find << "(upper(" << static_cast<std::string>(_params(0)) << ") = ";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << ")" << std::flush;
    }
    else {
        _find << "upper(";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << ")" << std::flush;
    }
}

auto func_boolean(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() > 1) {
        _find << "(" << static_cast<std::string>(_params(0)) << " = cast(";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << " as boolean))" << std::flush;
    }
    else {
        _find << "cast(";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << " as boolean)" << std::flush;
    }
}

auto func_date(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() > 1) {
        _find << "(" << static_cast<std::string>(_params(0)) << " = cast(";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << " as datetime(3)))" << std::flush;
    }
    else {
        _find << "cast(";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << " as datetime(3))" << std::flush;
    }
}

auto func_integer(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() > 1) {
        _find << "(" << static_cast<std::string>(_params(0)) << " = cast(";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << " as integer))" << std::flush;
    }
    else {
        _find << "cast(";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << " as integer)" << std::flush;
    }
}

auto func_floating(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() > 1) {
        _find << "(" << static_cast<std::string>(_params(0)) << " = cast(";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << " as double))" << std::flush;
    }
    else {
        _find << "cast(";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << " as double)" << std::flush;
    }
}

auto func_string(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() > 1) {
        _find << "(" << static_cast<std::string>(_params(0)) << " = cast(";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << " as char))" << std::flush;
    }
    else {
        _find << "cast(";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << " as char)" << std::flush;
    }
}

auto func_ne(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " <> ";
    zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
    _find << ")" << std::flush;
}

auto func_gt(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " > ";
    zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
    _find << ")" << std::flush;
}

auto func_gte(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " >= ";
    zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
    _find << ")" << std::flush;
}

auto func_lt(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " < ";
    zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
    _find << ")" << std::flush;
}

auto func_lte(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " <= ";
    zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
    _find << ")" << std::flush;
}

auto func_between(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " > ";
    zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
    _find << " and ";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " < ";
    zpt::storage::functional_to_sql(_params(2), _find, ::value_output);
    _find << ")" << std::flush;
}

auto func_like(zpt::json _params, std::ostream& _find) -> void {
    if (_params(2) == "i") {
        _find << "(lower(";
        zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
        _find << ") like lower(";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << "))" << std::flush;
    }
    else {
        _find << "(";
        zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
        _find << " like ";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << ")" << std::flush;
    }
}

auto func_in(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " in (";
    size_t _idx{ 0 };
    for (auto&& [_, __, _value] : _params) {
        if (_idx != 0) {
            if (_idx != 1) { _find << ", "; }
            zpt::storage::functional_to_sql(_value, _find, ::value_output);
        }
        ++_idx;
    }
    _find << "))" << std::flush;
}

auto func_is(zpt::json _params, std::ostream& _find) -> void {
    _find << "(";
    zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
    _find << " is ";
    zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
    _find << ")" << std::flush;
}

auto func_not(zpt::json _params, std::ostream& _find) -> void {
    if (_params->size() == 1) {
        _find << "not ";
        zpt::storage::functional_to_sql(_params(0), _find, ::value_output);
        _find << std::flush;
    }
    else if (_params(1)->is_object() && _params(1)("params")->ok()) {
        (**_params[1]["params"]->array())
          .insert((**_params[1]["params"]->array()).begin(), _params(0));
        _find << "(";
        _find << "not(";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << "))" << std::flush;
    }
    else {
        _find << "(";
        zpt::storage::functional_to_sql(_params(0), _find, ::variable_name);
        _find << " not ";
        zpt::storage::functional_to_sql(_params(1), _find, ::value_output);
        _find << ")" << std::flush;
    }
}

auto functors() -> std::map<std::string, zpt::storage::functor>& {
    static std::map<std::string, zpt::storage::functor> _funcs = {
        { "lower", ::func_lower },     { "upper", ::func_upper },     { "boolean", ::func_boolean },
        { "date", ::func_date },       { "integer", ::func_integer }, { "float", ::func_floating },
        { "double", ::func_floating }, { "string", ::func_string },   { "ne", ::func_ne },
        { "gt", ::func_gt },           { "gte", ::func_gte },         { "lt", ::func_lt },
        { "lte", ::func_lte },         { "between", ::func_between }, { "like", ::func_like },
        { "in", ::func_in },           { "is", ::func_is },           { "not", ::func_not }
    };
    return _funcs;
}

auto providers() -> std::map<std::string, std::function<zpt::storage::connection(zpt::json)>>& {
    static std::map<std::string, std::function<zpt::storage::connection(zpt::json)>> _global;
    return _global;
}
} // namespace
