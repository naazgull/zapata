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

#include <zapata/prolog/prolog.h>

zpt::prolog::bridge::bridge(std::string const& _cmd)
  : __engine_args{ _cmd } {
    this->initialize();
}

zpt::prolog::bridge::~bridge() throw() { PL_cleanup(0); }

auto zpt::prolog::bridge::name() const -> std::string { return "prolog"; }

auto zpt::prolog::bridge::setup_module(zpt::json _conf, std::string _external_path)
  -> zpt::prolog::bridge& {
    std::unique_lock _guard{ this->__underlying_mutex };
    return (*this);
}

auto zpt::prolog::bridge::setup_module(zpt::json _conf, callback_type _callback)
  -> zpt::prolog::bridge& {
    std::unique_lock _guard{ this->__underlying_mutex };
    return (*this);
}

auto zpt::prolog::bridge::setup_lambda(zpt::json _conf, lambda_type _callback)
  -> zpt::prolog::bridge& {
    std::unique_lock _guard{ this->__underlying_mutex };
    return (*this);
}

auto zpt::prolog::bridge::find(zpt::json _to_locate) -> object_type {
    std::unique_lock _guard{ this->__underlying_mutex };
    return zpt::prolog::term::null();
}

auto zpt::prolog::bridge::to_json(object_type _to_convert) -> zpt::json {
    if (_to_convert == zpt::prolog::term::null()) { return zpt::undefined; }
    return zpt::prolog::to_json(*_to_convert);
}

auto zpt::prolog::bridge::to_ref(object_type _to_convert) -> zpt::json {
    std::shared_lock _guard{ this->__underlying_mutex };
    return zpt::undefined;
}

auto zpt::prolog::bridge::to_object(zpt::json _to_convert) -> object_type {
    std::shared_lock _guard{ this->__underlying_mutex };
    return zpt::prolog::term::null();
}

auto zpt::prolog::bridge::from_ref(zpt::json _to_convert, object_type _return) -> object_type {
    std::shared_lock _guard{ this->__underlying_mutex };
    return zpt::prolog::term::null();
}

auto zpt::prolog::bridge::execute(zpt::json _func, zpt::json _args)
  -> zpt::prolog::bridge::object_type {
    std::unique_lock _guard{ this->__underlying_mutex };
    return zpt::prolog::term::null();
}

auto zpt::prolog::bridge::initialize() -> zpt::prolog::bridge& {
    if (this->__initialized.exchange(true)) { return (*this); }

    std::unique_lock _guard{ this->__underlying_mutex };
    char* _arg = const_cast<char*>(this->__engine_args.data());
    expect(PL_initialise(1, &_arg), "couldn't initialise Prolog engine");
    return (*this);
}

auto zpt::prolog::to_json(term_t _to_convert) -> zpt::json {
    bool _supported_type{ false };

    switch (PL_term_type(_to_convert)) {
        case PL_VARIABLE: {
            expect(_supported_type, "unsupported type PL_VARIABLE");
            break;
        }
        case PL_NIL: {
            return zpt::undefined;
        }
        case PL_BLOB: {
            expect(_supported_type, "unsupported type PL_VARIABLE");
            break;
        }
        case PL_ATOM:
        case PL_STRING: {
            char* _buffer{ nullptr };
            expect(PL_get_chars(_to_convert, &_buffer, CVT_ATOM | CVT_STRING | BUF_MALLOC),
                   "counldn't extract the string from the term");
            auto _str = zpt::json::string(std::string{ const_cast<char const*>(_buffer) });
            PL_free(_buffer);
            return _str;
        }
        case PL_INTEGER: {
            std::int64_t _int{ 0 };
            expect(PL_get_int64(_to_convert, &_int), "couldn't extract the integer from the term");
            return zpt::json::integer(_int);
        }
        case PL_RATIONAL:
        case PL_FLOAT: {
            double _floating{ 0 };
            expect(PL_get_float(_to_convert, &_floating),
                   "couldn't extract the float from the term");
            return zpt::json::floating(_floating);
        }
        case PL_TERM: {
            if (PL_is_compound(_to_convert)) {
                atom_t _name{ 0 };
                size_t _arity{ 0 };
                expect(PL_get_name_arity(_to_convert, &_name, &_arity),
                       "couldn't get name and arity from the compound term");
                size_t _{ 0 };
                std::string _functor{ PL_atom_nchars(_name, &_) };

                auto _composed = zpt::json::object();
                for (size_t _idx = 1; _idx != _arity + 1; ++_idx) {
                    zpt::prolog::term _term;
                    PL_get_arg(_idx, _to_convert, *_term);
                    auto _element = zpt::prolog::to_json(*_term);
                    if (_element->type() == zpt::JSObject) { _composed += _element; }
                    else { _composed << std::format("{}", _idx) << _element; }
                }

                auto _return = zpt::json::object();
                _return << _functor << _composed;
                return _return;
            }
            break;
        }
        case PL_LIST_PAIR: {
            zpt::prolog::term _head;
            term_t _tail = PL_copy_term_ref(_to_convert);
            auto _return = zpt::json::array();
            while (PL_get_list_ex(_tail, _head, _tail)) { _return << zpt::prolog::to_json(_head); }
            return _return;
        }
        case PL_DICT: {
            break;
        }
    }

    return zpt::undefined;
}

auto zpt::prolog::to_object(zpt::json _to_convert) -> zpt::prolog::term {
    switch (_to_convert->type()) {
        case zpt::JSObject: {
            zpt::prolog::term _t;
            for (auto&& [_, _key, _value] : _to_convert) {}
        }
        case zpt::JSArray: {
        }
        case zpt::JSString: {
        }
        case zpt::JSInteger: {
        }
        case zpt::JSDouble: {
        }
        case zpt::JSBoolean: {
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
        }
        case zpt::JSDate: {
        }
        case zpt::JSLambda: {
        }
        case zpt::JSRegex: {
        }
    }
}

auto zpt::PROLOG_BRIDGE(std::string const& _cmd) -> zpt::prolog::bridge& {
    static zpt::prolog::bridge _bridge{ _cmd };
    return _bridge;
}
