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

zpt::prolog::bridge::bridge()
  : __underlying{ zpt::GLOBAL_CONFIG()("self")("command")->string().data() } {
    this->initialize();
}

zpt::prolog::bridge::~bridge() {}

auto zpt::prolog::bridge::name() const -> std::string { return "prolog"; }

auto zpt::prolog::bridge::setup_module(zpt::json _conf,
                                       std::string _external_path,
                                       bool _persist = true) -> zpt::prolog::bridge& {
    std::unique_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::setup_module(zpt::json _conf,
                                       callback_type _callback,
                                       bool _persist = true) -> zpt::prolog::bridge& {
    std::unique_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::setup_lambda(zpt::json _conf, lambda_type _callback)
  -> zpt::prolog::bridge& {
    std::unique_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::find(zpt::json _to_locate) -> object_type {
    std::unique_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::to_json(object_type _to_convert) -> zpt::json {
    if (object_type == nullptr) { return zpt::undefined; }

    switch (_to_convert->type()) {
        case PL_VARIABLE: {
            auto _name = _to_convert->as_string();
            
            break;
        }
        case PL_ATOM: {
            break;
        }
        case PL_INTEGER: {
            break;
        }
        case PL_RATIONAL: {
            break;
        }
        case PL_FLOAT: {
            break;
        }
        case PL_STRING: {
            break;
        }
        case PL_TERM: {
            break;
        }
        case PL_NIL: {
            break;
        }
        case PL_BLOB: {
            break;
        }
        case PL_LIST_PAIR: {
            break;
        }
        case PL_FUNCTOR: {
            break;
        }
        case PL_LIST: {
            break;
        }
        case PL_CHARS: {
            break;
        }
        case PL_POINTER: {
            break;
        }
        case PL_CODE_LIST: {
            break;
        }
        case PL_CHAR_LIST: {
            break;
        }
        case PL_BOOL: {
            break;
        }
        case PL_FUNCTOR_CHARS: {
            break;
        }
        case _PL_PREDICATE_INDICATOR: {
            break;
        }
        case PL_SHORT: {
            break;
        }
        case PL_INT: {
            break;
        }
        case PL_LONG: {
            break;
        }
        case PL_DOUBLE: {
            break;
        }
        case PL_NCHARS: {
            break;
        }
        case PL_UTF8_CHARS: {
            break;
        }
        case PL_UTF8_STRING: {
            break;
        }
        case PL_INT64: {
            break;
        }
        case PL_NUTF8_CHARS: {
            break;
        }
        case PL_NUTF8_CODES: {
            break;
        }
        case PL_NUTF8_STRING: {
            break;
        }
        case PL_NWCHARS: {
            break;
        }
        case PL_NWCODES: {
            break;
        }
        case PL_NWSTRING: {
            break;
        }
        case PL_MBCHARS: {
            break;
        }
        case PL_MBCODES: {
            break;
        }
        case PL_MBSTRING: {
            break;
        }
        case PL_INTPTR: {
            break;
        }
        case PL_CHAR: {
            break;
        }
        case PL_CODE: {
            break;
        }
        case PL_BYTE: {
            break;
        }
        case PL_PARTIAL_LIST: {
            break;
        }
        case PL_CYCLIC_TERM: {
            break;
        }
        case PL_NOT_A_LIST: {
            break;
        }
        case PL_DICT: {
            break;
        }
        case PL_SWORD: {
            break;
        }
    }
}

auto zpt::prolog::bridge::to_ref(object_type _to_convert, int _index = 1) -> zpt::json {
    std::shared_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::to_object(zpt::json _to_convert) -> object_type {
    std::shared_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::from_ref(zpt::json _to_convert, object_type _return) -> object_type {
    std::shared_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::execute(zpt::json _func, zpt::json _args)
  -> zpt::prolog::bridge::object_type {
    std::unique_lock _guard{ this->__underlying_mutex };
}

auto zpt::prolog::bridge::initialize() -> zpt::prolog::bridge& {
    if (this->__initialized.exchange(true)) { return (*this); }

    std::unique_lock _guard{ this->__underlying_mutex };
}

auto zpt::PROLOG_BRIDGE() -> zpt::prolog::bridge& {
    static zpt::prolog::bridge _bridge{};
    return _bridge;
}
