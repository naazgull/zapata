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

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <zapata/mongodb/translate.h>

using namespace bsoncxx::builder::basic;

// ---- append_value ----

auto zpt::storage::mongodb::append_value(bsoncxx::builder::basic::document& _doc,
                                         std::string const& _key,
                                         zpt::json _value) -> void {
    switch (_value->type()) {
        case zpt::JSString:
        case zpt::JSDate:
        case zpt::JSRegex: {
            _doc.append(kvp(_key, static_cast<std::string>(_value)));
            break;
        }
        case zpt::JSInteger: {
            _doc.append(kvp(_key, bsoncxx::types::b_int64{ _value->integer() }));
            break;
        }
        case zpt::JSDouble: {
            _doc.append(kvp(_key, _value->floating()));
            break;
        }
        case zpt::JSBoolean: {
            _doc.append(kvp(_key, _value->boolean()));
            break;
        }
        case zpt::JSObject: {
            _doc.append(kvp(_key, to_bson(_value)));
            break;
        }
        case zpt::JSArray: {
            _doc.append(kvp(_key, [&_value](sub_array _arr) {
                for (auto const& [_, __, _v] : _value) {
                    switch (_v->type()) {
                        case zpt::JSString:
                        case zpt::JSDate:
                        case zpt::JSRegex: {
                            _arr.append(static_cast<std::string>(_v));
                            break;
                        }
                        case zpt::JSInteger: {
                            _arr.append(bsoncxx::types::b_int64{ _v->integer() });
                            break;
                        }
                        case zpt::JSDouble: {
                            _arr.append(_v->floating());
                            break;
                        }
                        case zpt::JSBoolean: {
                            _arr.append(_v->boolean());
                            break;
                        }
                        case zpt::JSObject: {
                            bsoncxx::builder::basic::document _sub;
                            for (auto const& [_i, _k, _vv] : _v) {
                                append_value(_sub, static_cast<std::string>(_k), _vv);
                            }
                            _arr.append(_sub.extract());
                            break;
                        }
                        default: {
                            _arr.append(bsoncxx::types::b_null{});
                            break;
                        }
                    }
                }
            }));
            break;
        }
        default: {
            _doc.append(kvp(_key, bsoncxx::types::b_null{}));
            break;
        }
    }
}

// ---- to_bson ----

auto zpt::storage::mongodb::to_bson(zpt::json _doc) -> bsoncxx::document::value {
    bsoncxx::builder::basic::document _builder;
    if (_doc->ok() && _doc->type() == zpt::JSObject) {
        for (auto const& [_, _key, _value] : _doc) {
            append_value(_builder, static_cast<std::string>(_key), _value);
        }
    }
    return _builder.extract();
}

// ---- from_bson ----

auto zpt::storage::mongodb::from_bson(bsoncxx::document::view _doc) -> zpt::json {
    auto _record = zpt::json::object();

    for (auto const& _elem : _doc) {
        std::string _key{ _elem.key() };

        switch (_elem.type()) {
            case bsoncxx::type::k_string: {
                _record[_key] = std::string{ _elem.get_string().value };
                break;
            }
            case bsoncxx::type::k_int32: {
                _record[_key] = static_cast<long long>(_elem.get_int32().value);
                break;
            }
            case bsoncxx::type::k_int64: {
                _record[_key] = static_cast<long long>(_elem.get_int64().value);
                break;
            }
            case bsoncxx::type::k_double: {
                _record[_key] = _elem.get_double().value;
                break;
            }
            case bsoncxx::type::k_bool: {
                _record[_key] = _elem.get_bool().value;
                break;
            }
            case bsoncxx::type::k_document: {
                _record[_key] = from_bson(_elem.get_document().value);
                break;
            }
            case bsoncxx::type::k_array: {
                auto _arr = zpt::json::array();
                for (auto const& _item : _elem.get_array().value) {
                    switch (_item.type()) {
                        case bsoncxx::type::k_string: {
                            _arr << std::string{ _item.get_string().value };
                            break;
                        }
                        case bsoncxx::type::k_int32: {
                            _arr << static_cast<long long>(_item.get_int32().value);
                            break;
                        }
                        case bsoncxx::type::k_int64: {
                            _arr << static_cast<long long>(_item.get_int64().value);
                            break;
                        }
                        case bsoncxx::type::k_double: {
                            _arr << _item.get_double().value;
                            break;
                        }
                        case bsoncxx::type::k_bool: {
                            _arr << _item.get_bool().value;
                            break;
                        }
                        case bsoncxx::type::k_document: {
                            _arr << from_bson(_item.get_document().value);
                            break;
                        }
                        case bsoncxx::type::k_oid: {
                            _arr << _item.get_oid().value.to_string();
                            break;
                        }
                        default: {
                            _arr << zpt::undefined;
                            break;
                        }
                    }
                }
                _record[_key] = _arr;
                break;
            }
            case bsoncxx::type::k_oid: {
                _record[_key] = _elem.get_oid().value.to_string();
                break;
            }
            case bsoncxx::type::k_date: {
                _record[_key] =
                  std::to_string(_elem.get_date().value.count()); // ms since epoch as string
                break;
            }
            case bsoncxx::type::k_null:
            case bsoncxx::type::k_undefined: {
                _record[_key] = zpt::undefined;
                break;
            }
            default: {
                _record[_key] = zpt::undefined;
                break;
            }
        }
    }

    return _record;
}

// ---- to_filter ----

auto zpt::storage::mongodb::to_filter(zpt::json _filter) -> bsoncxx::document::value {
    if (!_filter->ok() || _filter->type() != zpt::JSObject) {
        return bsoncxx::builder::basic::make_document();
    }
    return to_bson(_filter);
}

// ---- to_update_doc ----

auto zpt::storage::mongodb::to_update_doc(zpt::json _to_update) -> bsoncxx::document::value {
    bsoncxx::builder::basic::document _set;
    bsoncxx::builder::basic::document _unset;
    bool _has_unset{ false };

    for (auto const& [_, _key, _value] : _to_update) {
        auto _k = static_cast<std::string>(_key);
        if (_value->ok()) { append_value(_set, _k, _value); }
        else {
            _unset.append(kvp(_k, std::string{ "" }));
            _has_unset = true;
        }
    }

    bsoncxx::builder::basic::document _update;
    _update.append(kvp("$set", _set.extract()));
    if (_has_unset) { _update.append(kvp("$unset", _unset.extract())); }
    return _update.extract();
}

// ---- to_projection ----

auto zpt::storage::mongodb::to_projection(zpt::json _fields) -> bsoncxx::document::value {
    bsoncxx::builder::basic::document _proj;
    for (auto const& [_, _field, __] : _fields) {
        _proj.append(kvp(static_cast<std::string>(_field), 1));
    }
    return _proj.extract();
}

// ---- to_sort ----

auto zpt::storage::mongodb::to_sort(zpt::json _sort_spec) -> bsoncxx::document::value {
    bsoncxx::builder::basic::document _sort;
    for (auto const& [_, _field, _dir] : _sort_spec) {
        int _order = (_dir->string() == "asc") ? 1 : -1;
        _sort.append(kvp(static_cast<std::string>(_field), _order));
    }
    return _sort.extract();
}
