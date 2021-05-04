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

#include <zapata/mysqlx/translate.h>
#include <mysqlx/xdevapi.h>

auto
zpt::storage::mysqlx::translate_from_db(::mysqlx::Value const& _rhs) -> zpt::json {
    switch (_rhs.getType()) {
        case ::mysqlx::Value::Type::VNULL: {
            return zpt::undefined;
        }
        case ::mysqlx::Value::Type::UINT64: {
            return zpt::json{ static_cast<size_t>(static_cast<uint64_t>(_rhs)) };
        }
        case ::mysqlx::Value::Type::INT64: {
            return zpt::json{ static_cast<long long>(static_cast<int64_t>(_rhs)) };
        }
        case ::mysqlx::Value::Type::FLOAT: {
            return zpt::json{ static_cast<double>(static_cast<float>(_rhs)) };
        }
        case ::mysqlx::Value::Type::DOUBLE: {
            return zpt::json{ static_cast<double>(_rhs) };
        }
        case ::mysqlx::Value::Type::BOOL: {
            return zpt::json{ static_cast<long long>(static_cast<int64_t>(_rhs)) };
        }
        case ::mysqlx::Value::Type::STRING: {
            return zpt::json{ static_cast<std::string>(_rhs) };
        }
        case ::mysqlx::Value::Type::DOCUMENT: {
            auto _doc = static_cast<::mysqlx::DbDoc const&>(_rhs);
            return zpt::storage::mysqlx::translate_object_from_db(_doc);
        }
        case ::mysqlx::Value::Type::RAW: {
            return zpt::storage::mysqlx::translate_bytes_from_db(_rhs.getRawBytes());
        }
        case ::mysqlx::Value::Type::ARRAY: {
            return zpt::storage::mysqlx::translate_array_from_db(_rhs);
        }
    }
    return zpt::undefined;
}

auto
zpt::storage::mysqlx::translate_object_from_db(::mysqlx::DbDoc& _rhs) -> zpt::json {
    auto _to_return = zpt::json::object();
    for (auto _it = _rhs.begin(); _it != _rhs.end(); ++_it) {
        _to_return << *_it << zpt::storage::mysqlx::translate_from_db(_rhs[*_it]);
    }
    return _to_return;
}

auto
zpt::storage::mysqlx::translate_array_from_db(::mysqlx::Value const& _rhs) -> zpt::json {
    auto _to_return = zpt::json::array();
    for (auto _it : _rhs) { _to_return << zpt::storage::mysqlx::translate_from_db(_it); }
    return _to_return;
}

auto
zpt::storage::mysqlx::translate_bytes_from_db(::mysqlx::bytes const& _rhs) -> zpt::json {
    return zpt::undefined;
}
