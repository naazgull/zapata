/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <zapata/json/json.h>
#include <zapata/lisp/Bridge.h>
#include <zapata/log/log.h>

auto
zpt::lisp::from_lisp(cl_object _in) -> zpt::json {
    zpt::json _parent;
    zpt::lisp::from_lisp(_in, _parent);
    return _parent;
}

auto
zpt::lisp::from_lisp(cl_object _exp, zpt::json& _parent) -> void {
    switch (ecl_t_of(_exp)) {
        case t_start: {
            break;
        }
        case t_list: {
            if (Null(_exp)) {
                if (_parent->ok()) {
                    _parent << zpt::undefined;
                }
            }
            else {
                zpt::json _ret = zpt::json::array();
                cl_object _list = _exp;
                for (; !Null(_list); _list = ecl_cdr(_list)) {
                    auto _item = ecl_car(_list);
                    zpt::lisp::from_lisp(_item, _ret);
                }
                if (_parent->ok()) {
                    _parent << _ret;
                }
                else {
                    _parent = _ret;
                }
            }
            break;
        }
        case t_character: {
            char _o = (char)ECL_CHAR_CODE(_exp);
            std::string _s(&_o, 1);
            if (_parent->ok()) {
                _parent << _s;
            }
            else {
                _parent = zpt::json::string(_s);
            }
            return;
        }
        case t_fixnum: {
            int _o = (int)ecl_to_fixnum(_exp);
            if (_parent->ok()) {
                _parent << _o;
            }
            else {
                _parent = zpt::json::integer(_o);
            }
            return;
        }
        case t_bignum: {
            double _o = ecl_to_double(_exp);
            if (_parent->ok()) {
                _parent << _o;
            }
            else {
                _parent = zpt::json::floating(_o);
            }
            break;
        }
        case t_ratio: {
            double _o = ecl_to_double(_exp);
            if (_parent->ok()) {
                _parent << _o;
            }
            else {
                _parent = zpt::json::floating(_o);
            }
            break;
        }
        case t_singlefloat: {
            float _o = ecl_to_float(_exp);
            if (_parent->ok()) {
                _parent << _o;
            }
            else {
                _parent = zpt::json::floating(_o);
            }
            break;
        }
        case t_doublefloat: {
            double _o = ecl_to_double(_exp);
            if (_parent->ok()) {
                _parent << _o;
            }
            else {
                _parent = zpt::json::floating(_o);
            }
            break;
        }
#ifdef ECL_LONG_FLOAT
        case t_longfloat: {
            double _o = (double)ecl_to_long_double(_exp);
            if (_parent->ok()) {
                _parent << _o;
            }
            else {
                _parent = zpt::json::floating(_o);
            }
            break;
        }
#endif
        case t_complex: {
            return;
        }
        case t_symbol: {
            if (_exp->symbol.value != nullptr) {
                if (ecl_t_of(_exp->symbol.value) == t_symbol) {
                    std::string _str;
                    for (size_t _i = 0; _i != _exp->symbol.name->base_string.fillp; _i++) {
                        _str.push_back((char)ecl_char(_exp->symbol.name, _i));
                    }
                    if (_str == "FALSE") {
                        if (_parent->ok()) {
                            _parent << false;
                        }
                        else {
                            _parent = zpt::json::boolean(false);
                        }
                    }
                    else if (_str == "ARRAY") {
                        if (_parent->ok()) {
                            _parent << zpt::json::array();
                        }
                        else {
                            _parent = zpt::json::array();
                        }
                    }
                    else {
                        if (_parent->ok()) {
                            _parent << true;
                        }
                        else {
                            _parent = zpt::json::boolean(true);
                        }
                    }
                }
                else {
                    zpt::lisp::from_lisp(_exp->symbol.value, _parent);
                }
            }
            else {
                if (_parent->ok()) {
                    _parent << zpt::undefined;
                }
                // expect(false, "symbol is pair", 500, 0);
            }
            break;
        }
        case t_package: {
            return;
        }
        case t_hashtable: {
            zpt::json _ret = zpt::json::object();
            for (cl_index _i = 0; _i != _exp->hash.size; _i++) {
                struct ecl_hashtable_entry _e = _exp->hash.data[_i];
                if (_e.key != OBJNULL) {
                    zpt::json _key;
                    zpt::lisp::from_lisp(_e.key, _key);
                    zpt::json _value;
                    zpt::lisp::from_lisp(_e.value, _value);
                    if (_value->ok()) {
                        _ret << ((std::string)_key) << _value;
                    }
                }
            }
            if (_parent->ok()) {
                _parent << _ret;
            }
            else {
                _parent = _ret;
            }
            break;
        }
        case t_array: {
            zpt::json _ret = zpt::json::array();
            unsigned int _arr_dim = ecl_array_dimension(_exp, 0);
            for (unsigned int _i = 0; _i != _arr_dim; _i++) {
                zpt::lisp::from_lisp(ecl_aref1(_exp, _i), _ret);
            }
            if (_parent->ok()) {
                _parent << _ret;
            }
            else {
                _parent = _ret;
            }
            break;
        }
        case t_vector: {
            zpt::json _ret = zpt::json::array();
            unsigned int _arr_dim = ecl_array_dimension(_exp, 0);
            for (unsigned int _i = 0; _i != _arr_dim; _i++) {
                zpt::lisp::from_lisp(ecl_aref1(_exp, _i), _ret);
            }
            if (_parent->ok()) {
                _parent << _ret;
            }
            else {
                _parent = _ret;
            }
            break;
        }
#ifdef ECL_UNICODE
        case t_string: {
            std::string _str;
            for (size_t _i = 0; _i != _exp->string.fillp; _i++) {
                _str.push_back((char)ecl_char(_exp, _i));
            }
            zpt::replace(_str, "[[*nl*]]", "\\n");
            zpt::replace(_str, "[*nl*]", "\n");
            if (_parent->ok()) {
                _parent << _str;
            }
            else {
                _parent = zpt::json::string(_str);
            }
            break;
        }
#endif
        case t_base_string: {
            std::string _str;
            for (size_t _i = 0; _i != _exp->base_string.fillp; _i++) {
                _str.push_back((char)ecl_char(_exp, _i));
            }
            zpt::replace(_str, "[[*nl*]]", "\\n");
            zpt::replace(_str, "[*nl*]", "\n");
            if (_parent->ok()) {
                _parent << _str;
            }
            else {
                _parent = zpt::json::string(_str);
            }
            break;
        }
        case t_bitvector: {
            return;
        }
        case t_stream: {
            return;
        }
        case t_random: {
            return;
        }
        case t_readtable: {
            return;
        }
        case t_pathname: {
            // std::string _pathname(((std::string) this->tojson(_exp->pathname.type)));
            //_pathname += std::string(":") + ((std::string)
            // this->tojson(_exp->pathname.host)) +
            // std::string("/") +
            //((std::string) this->tojson(_exp->pathname.directory)) + std::string("/")
            //+ ((std::string)
            // this->tojson(_exp->pathname.name));
            //_parent << _str;
            break;
        }
        case t_bytecodes: {
            return;
        }
        case t_bclosure: {
            return;
        }
        case t_cfun: {
            return;
        }
        case t_cfunfixed: {
            return;
        }
        case t_cclosure: {
            return;
        }
#ifdef CLOS
        case t_instance: {
            return;
        }
#else
        case t_structure: {
            break;
        }
#endif /* CLOS */
#ifdef ECL_THREADS
        case t_process: {
            return;
        }
        case t_lock: {
            return;
        }
        case t_rwlock: {
            return;
        }
        case t_condition_variable: {
            return;
        }
        case t_semaphore: {
            return;
        }
        case t_barrier: {
            return;
        }
        case t_mailbox: {
            return;
        }
#endif
        case t_codeblock: {
            return;
        }
        case t_foreign: {
            return;
        }
        case t_frame: {
            return;
        }
        case t_weak_pointer: {
            return;
        }
#ifdef ECL_SSE2
        case t_sse_pack: {
            return;
        }
#endif
        case t_end: {
            return;
        }
        case t_other: {
            return;
        }
        case t_contiguous: {
            return;
        }
        default: {
            return;
        }
    }
}

auto
zpt::lisp::to_lisp(zpt::json _in, zpt::lisp::bridge* _bridge) -> zpt::lisp::object {
    std::string _exp("(eval (read-from-string \"");
    std::string _str = zpt::lisp::to_lisp_string(_in);
    std::ostringstream _ss;
    for (auto _iter = _str.cbegin(); _iter != _str.cend(); _iter++) {
        switch (*_iter) {
            case '\\':
                _ss << "\\\\";
                break;
            case '"':
                _ss << "\\\"";
                break;
            case '\b':
                _ss << "\\b";
                break;
            case '\f':
                _ss << "\\f";
                break;
            case '\n':
                _ss << "\\n";
                break;
            case '\r':
                _ss << "\\r";
                break;
            case '\t':
                _ss << "\\t";
                break;
            default:
                _ss << *_iter;
                break;
        }
    }
    _str.assign(_ss.str());
    _exp += _str + std::string("\"))");
    return _bridge->eval(_exp);
}

auto
zpt::lisp::to_lisp_string(zpt::json _json) -> std::string {
    std::string _ret;
    switch (_json->type()) {
        case zpt::JSObject: {
            _ret += std::string("(json ");
            for (auto _o : _json->object()) {
                _ret += std::string(" \"") + _o.first + std::string("\" ") +
                        zpt::lisp::to_lisp_string(_o.second);
            }
            _ret += std::string(")");
            break;
        }
        case zpt::JSArray: {
            _ret += std::string("(list ");
            for (auto _o : _json->array()) {
                _ret += std::string(" ") + zpt::lisp::to_lisp_string(_o) + std::string(" ");
            }
            _ret += std::string(")");
            break;
        }
        case zpt::JSString: {
            std::string _str(_json);
            _ret += std::string("(coerce (list ");

            for (std::string::size_type _i = 0; _i < _str.size(); _i++) {
                unsigned char _c = _str[_i];
                _ret =
                  _ret + std::string("(code-char ") + std::to_string((int)_c) + std::string(")");
            }
            _ret += std::string(") 'string)");

            break;
        }
        case zpt::JSDate: {
            _ret += std::string("\"") + (std::string)_json + std::string("\"");
            break;
        }
        case zpt::JSBoolean: {
            _ret += ((bool)_json ? std::string("t") : std::string(":false"));
            break;
        }
        case zpt::JSInteger:
        case zpt::JSDouble: {
            _ret += (std::string)_json;
            break;
        }
        case zpt::JSLambda: {
            break;
        }
        case zpt::JSNil: {
            _ret += "nil";
            break;
        }
        default: {
            break;
        }
    }
    return _ret;
}
