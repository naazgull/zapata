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

#include <zapata/exceptions/Exception.h>

#include <sstream>
#include <execinfo.h>
#include <ctime>
#include <zapata/text/convert.h>

zpt::exception::exception(std::string const& _what, bool _with_stack)
  : std::exception()
  , __what(_what) {
    zpt::replace(this->__what, "\"", "");
    if (_with_stack) {
        void* _backtrace_array[30];
        size_t _backtrace_size = ::backtrace(_backtrace_array, 30);
        char** _backtrace = ::backtrace_symbols(_backtrace_array, _backtrace_size);
        for (size_t _i = 0; _i != _backtrace_size; _i++) {
            this->__backtrace.insert(this->__backtrace.length(), "\t");
            this->__backtrace.insert(this->__backtrace.length(), std::string{ _backtrace[_i] });
            this->__backtrace.insert(this->__backtrace.length(), "\n");
        }
        free(_backtrace);
    }
}

zpt::exception::~exception() throw() {}

auto
zpt::exception::what() const noexcept -> const char* {
    return this->__what.data();
}

auto
zpt::exception::backtrace() const -> const char* {
    return this->__backtrace.length() == 0 ? nullptr : this->__backtrace.data();
}
