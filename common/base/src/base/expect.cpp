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

#include <fstream>
#include <zapata/base/expect.h>

auto
zpt::to_string(zpt::JSONType _type) -> std::string {
    switch (_type) {
        case JSObject: {
            return "object";
        }
        case JSArray: {
            return "array";
        }
        case JSString: {
            return "string";
        }
        case JSInteger: {
            return "int";
        }
        case JSDouble: {
            return "double";
        }
        case JSBoolean: {
            return "bool";
        }
        case JSNil: {
            return "null";
        }
        case JSDate: {
            return "date-time";
        }
        case JSLambda: {
            return "lambda";
        }
        case JSRegex: {
            return "regexp";
        }
    }
    return "undefined";
}

auto
non_static_get_tz() -> std::string {
    std::string _to_return;
    std::ifstream _tzf;
    _tzf.open("/etc/timezone");
    if (_tzf.is_open()) {
        _tzf >> _to_return;
        _tzf.close();
    }
    else { _to_return.assign("UTC"); }
    return _to_return;
}

auto
zpt::get_tz() -> std::string const& {
    static std::string tz = non_static_get_tz();
    return tz;
}

auto
zpt::get_time(time_t _t) -> zpt::tm_ptr {
    std::tm* _tm = new std::tm();
    std::memcpy(_tm, localtime(&_t), sizeof(std::tm));
    return zpt::tm_ptr(_tm);
}
