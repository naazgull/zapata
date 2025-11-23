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

// #include <mysqlx/xdevapi.h>
#include <cassert>
#include <zapata/mysqlx/translate.h>

zpt::storage::mysqlx::column_bind::column_bind(MYSQL_STMT* _statement) {
    if (_statement != nullptr) {
        this->__metadata = mysql_stmt_result_metadata(_statement);
        if (this->__metadata != nullptr) {
            this->__column_count = mysql_num_fields(this->__metadata);
            this->__bind = std::make_unique<MYSQL_BIND[]>(this->__column_count);

            for (size_t _idx = 0; _idx != this->__column_count; ++_idx) {
                auto _col = mysql_fetch_field_direct(this->__metadata, _idx);

                std::memset(&this->__bind[_idx], 0, sizeof(MYSQL_BIND));

                size_t _buffer_length = 0;
                switch (_col->type) {
                    case MYSQL_TYPE_DECIMAL:
                    case MYSQL_TYPE_NEWDECIMAL: {
                        _buffer_length = 68;
                        break;
                    }
                    case MYSQL_TYPE_TINY:
                    case MYSQL_TYPE_BOOL: {
                        _buffer_length = sizeof(std::int8_t);
                        break;
                    }
                    case MYSQL_TYPE_SHORT:
                    case MYSQL_TYPE_YEAR: {
                        _buffer_length = sizeof(std::int16_t);
                        break;
                    }
                    case MYSQL_TYPE_LONG:
                    case MYSQL_TYPE_INT24: {
                        _buffer_length = sizeof(std::int32_t);
                        break;
                    }
                    case MYSQL_TYPE_LONGLONG:
                    case MYSQL_TYPE_BIT: {
                        _buffer_length = sizeof(std::int64_t);
                        break;
                    }
                    case MYSQL_TYPE_FLOAT: {
                        _buffer_length = sizeof(float);
                        break;
                    }
                    case MYSQL_TYPE_DOUBLE: {
                        _buffer_length = sizeof(double);
                        break;
                    }
                    case MYSQL_TYPE_TIMESTAMP:
                    case MYSQL_TYPE_TIMESTAMP2:
                    case MYSQL_TYPE_DATE:
                    case MYSQL_TYPE_NEWDATE:
                    case MYSQL_TYPE_TIME:
                    case MYSQL_TYPE_TIME2:
                    case MYSQL_TYPE_DATETIME:
                    case MYSQL_TYPE_DATETIME2: {
                        _buffer_length = sizeof(MYSQL_TIME);
                        break;
                    }
                    case MYSQL_TYPE_VARCHAR:
                    case MYSQL_TYPE_VAR_STRING:
                    case MYSQL_TYPE_JSON:
                    case MYSQL_TYPE_ENUM:
                    case MYSQL_TYPE_TINY_BLOB:
                    case MYSQL_TYPE_MEDIUM_BLOB:
                    case MYSQL_TYPE_LONG_BLOB:
                    case MYSQL_TYPE_BLOB:
                    case MYSQL_TYPE_STRING: {
                        const size_t _buffer_max = 64;
                        _buffer_length = std::min<size_t>(_buffer_max, _col->length);
                        break;
                    }
                    case MYSQL_TYPE_GEOMETRY:
                    case MYSQL_TYPE_INVALID:
                    case MYSQL_TYPE_NULL:
                    case MYSQL_TYPE_TYPED_ARRAY:
                    case MYSQL_TYPE_SET: {
                        assert(false);
                        return;
                    }
                }

                this->__bind[_idx].buffer = std::malloc(_buffer_length);
                this->__bind[_idx].buffer_length = _buffer_length;
                this->__bind[_idx].buffer_type = _col->type;

                this->__bind[_idx].length =
                  static_cast<unsigned long*>(std::malloc(sizeof(unsigned long)));
                this->__bind[_idx].is_null = static_cast<bool*>(std::malloc(sizeof(bool)));
                this->__bind[_idx].error = static_cast<bool*>(std::malloc(sizeof(bool)));
                this->__bind[_idx].is_unsigned = ((_col->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG);

                *this->__bind[_idx].length = 0;
                *this->__bind[_idx].is_null = false;
                *this->__bind[_idx].error = 0;
            }
        }
    }
}

zpt::storage::mysqlx::column_bind::column_bind(column_bind&& rhs)
  : __bind{ std::move(rhs.__bind) }
  , __column_count{ rhs.__column_count }
  , __metadata{ std::move(rhs.__metadata) } {}

zpt::storage::mysqlx::column_bind::~column_bind() {
    if (this->__column_count != 0) {
        for (size_t _idx = 0; _idx != this->__column_count; ++_idx) {
            std::free(this->__bind[_idx].buffer);
            std::free(this->__bind[_idx].length);
            std::free(this->__bind[_idx].is_null);
            std::free(this->__bind[_idx].error);
        }
        mysql_free_result(this->__metadata);
    }
}

auto zpt::storage::mysqlx::column_bind::operator=(column_bind&& rhs) -> column_bind& {
    this->__bind = std::move(rhs.__bind);
    this->__column_count = rhs.__column_count;
    this->__metadata = std::move(rhs.__metadata);
    return (*this);
}

auto zpt::storage::mysqlx::column_bind::name(size_t _column) const -> std::string {
    auto _info = mysql_fetch_field_direct(this->__metadata, _column);
    return std::string{ _info->name, _info->name_length };
}

auto zpt::storage::mysqlx::column_bind::type(size_t _column) const -> enum_field_types {
    return mysql_fetch_field_direct(this->__metadata, _column)->type;
}

auto zpt::storage::mysqlx::column_bind::flags(size_t _column) const -> unsigned int {
    return mysql_fetch_field_direct(this->__metadata, _column)->flags;
}

auto zpt::storage::mysqlx::column_bind::charset(size_t _column) const -> unsigned int {
    return mysql_fetch_field_direct(this->__metadata, _column)->charsetnr;
}

auto zpt::storage::mysqlx::convert(MYSQL_STMT* _statement,
                                   zpt::storage::mysqlx::column_bind const& _cols) -> zpt::json {
    auto _record = zpt::json::object();
    for (size_t _col_idx = 0; _col_idx != _cols.__column_count; ++_col_idx) {
        auto _name = _cols.name(_col_idx);
        if (*_cols.__bind[_col_idx].is_null) {
            _record[_name] = zpt::undefined;
            continue;
        }

        switch (_cols.type(_col_idx)) {
            case MYSQL_TYPE_NULL: {
                _record[_name] = zpt::undefined;
                break;
            }
            case MYSQL_TYPE_BIT: {
                _record[_name] = _cols.get<std::uint64_t>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_BOOL: {
                _record[_name] = _cols.get<bool>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_TINY:
                if (_cols.__bind[_col_idx].is_unsigned) {
                    _record[_name] = _cols.get<std::uint8_t>(_statement, _col_idx);
                }
                else { _record[_name] = _cols.get<std::int8_t>(_statement, _col_idx); }
                break;
            case MYSQL_TYPE_SHORT:
                if (_cols.__bind[_col_idx].is_unsigned) {
                    _record[_name] = _cols.get<std::uint16_t>(_statement, _col_idx);
                }
                else { _record[_name] = _cols.get<std::int16_t>(_statement, _col_idx); }
                break;
            case MYSQL_TYPE_INT24:
            case MYSQL_TYPE_LONG: {
                if (_cols.__bind[_col_idx].is_unsigned) {
                    _record[_name] = _cols.get<std::uint32_t>(_statement, _col_idx);
                }
                else { _record[_name] = _cols.get<std::int32_t>(_statement, _col_idx); }
                break;
            }
            case MYSQL_TYPE_LONGLONG: {
                if (_cols.__bind[_col_idx].is_unsigned) {
                    _record[_name] = _cols.get<std::uint64_t>(_statement, _col_idx);
                }
                else { _record[_name] = _cols.get<std::int64_t>(_statement, _col_idx); }
                break;
            }
            case MYSQL_TYPE_FLOAT: {
                _record[_name] = _cols.get<float>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_DECIMAL:
            case MYSQL_TYPE_NEWDECIMAL: {
                _record[_name] = _cols.get<std::string_view>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_DOUBLE: {
                _record[_name] = _cols.get<double>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_ENUM:
            case MYSQL_TYPE_VARCHAR:
            case MYSQL_TYPE_VAR_STRING:
            case MYSQL_TYPE_STRING: {
                _record[_name] = _cols.get<std::string_view>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_MEDIUM_BLOB:
            case MYSQL_TYPE_LONG_BLOB:
            case MYSQL_TYPE_BLOB: {
                _record[_name] = _cols.get<std::string_view>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_TYPED_ARRAY:
            case MYSQL_TYPE_GEOMETRY:
            case MYSQL_TYPE_JSON: // JSON is always propagated in native binary format, we
                                  // should never get actual JSON type here
            case MYSQL_TYPE_SET: {
                _record[_name] = nullptr;
                break;
            }
            case MYSQL_TYPE_TIME2:
            case MYSQL_TYPE_TIME: {
                auto _time = _cols.get<MYSQL_TIME>(_statement, _col_idx);
                _record[_name] = std::format("{}{}:{}:{}.{}",
                                             (_time.neg ? "-" : ""), //
                                             _time.hour,             //
                                             _time.minute,           //
                                             _time.second,           //
                                             _time.second_part);
                break;
            }
            case MYSQL_TYPE_YEAR: {
                _record[_name] = _cols.get<short>(_statement, _col_idx);
                break;
            }
            case MYSQL_TYPE_NEWDATE:
            case MYSQL_TYPE_DATE: {
                auto date = _cols.get<MYSQL_TIME>(_statement, _col_idx);
                _record[_name] = std::format("{}-{}-{}",
                                             date.year,  //
                                             date.month, //
                                             date.day);
                break;
            }
            case MYSQL_TYPE_TIMESTAMP2:
            case MYSQL_TYPE_TIMESTAMP: {
                auto _timestamp = _cols.get<MYSQL_TIME>(_statement, _col_idx);
                _record[_name] = std::format("{}-{}-{}T{}:{}:{}.{}",
                                             _timestamp.year,   //
                                             _timestamp.month,  //
                                             _timestamp.day,    //
                                             _timestamp.hour,   //
                                             _timestamp.minute, //
                                             _timestamp.second, //
                                             _timestamp.second_part);
                break;
            }
            case MYSQL_TYPE_DATETIME2:
            case MYSQL_TYPE_DATETIME: {
                auto _datetime = _cols.get<MYSQL_TIME>(_statement, _col_idx);
                _record[_name] = std::format("{}-{}-{}T{}:{}:{}.{}",
                                             _datetime.year,   //
                                             _datetime.month,  //
                                             _datetime.day,    //
                                             _datetime.hour,   //
                                             _datetime.minute, //
                                             _datetime.second, //
                                             _datetime.second_part);
                break;
            }
            case MYSQL_TYPE_INVALID: {
                break;
            }
        }
    }

    return _record;
}

// auto zpt::storage::mysqlx::translate_from_db(::mysqlx::Value const& _rhs) -> zpt::json {
//     switch (_rhs.getType()) {
//         case ::mysqlx::Value::Type::VNULL: {
//             return zpt::undefined;
//         }
//         case ::mysqlx::Value::Type::UINT64: {
//             return zpt::json{ static_cast<size_t>(static_cast<uint64_t>(_rhs)) };
//         }
//         case ::mysqlx::Value::Type::INT64: {
//             return zpt::json{ static_cast<long long>(static_cast<int64_t>(_rhs)) };
//         }
//         case ::mysqlx::Value::Type::FLOAT: {
//             return zpt::json{ static_cast<double>(static_cast<float>(_rhs)) };
//         }
//         case ::mysqlx::Value::Type::DOUBLE: {
//             return zpt::json{ static_cast<double>(_rhs) };
//         }
//         case ::mysqlx::Value::Type::BOOL: {
//             return zpt::json{ static_cast<bool>(_rhs) };
//         }
//         case ::mysqlx::Value::Type::STRING: {
//             return zpt::json{ static_cast<std::string>(_rhs) };
//         }
//         case ::mysqlx::Value::Type::DOCUMENT: {
//             auto _doc = static_cast<::mysqlx::DbDoc const&>(_rhs);
//             return zpt::storage::mysqlx::translate_object_from_db(_doc);
//         }
//         case ::mysqlx::Value::Type::RAW: {
//             return zpt::storage::mysqlx::translate_bytes_from_db(_rhs.getRawBytes());
//         }
//         case ::mysqlx::Value::Type::ARRAY: {
//             return zpt::storage::mysqlx::translate_array_from_db(_rhs);
//         }
//     }
//     return zpt::undefined;
// }

// auto zpt::storage::mysqlx::translate_object_from_db(::mysqlx::DbDoc& _rhs) -> zpt::json {
//     auto _to_return = zpt::json::object();
//     for (auto _it = _rhs.begin(); _it != _rhs.end(); ++_it) {
//         _to_return << *_it << zpt::storage::mysqlx::translate_from_db(_rhs[*_it]);
//     }
//     return _to_return;
// }

// auto zpt::storage::mysqlx::translate_array_from_db(::mysqlx::Value const& _rhs) -> zpt::json {
//     auto _to_return = zpt::json::array();
//     for (auto _it : _rhs) { _to_return << zpt::storage::mysqlx::translate_from_db(_it); }
//     return _to_return;
// }

// auto zpt::storage::mysqlx::translate_bytes_from_db(::mysqlx::bytes const&) -> zpt::json {
//     return zpt::undefined;
// }
