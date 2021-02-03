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

#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <zapata/log/log.h>

namespace zpt {

class exception : public std::exception {
  public:
    exception(std::string const& _what);
    exception(std::string const& _what, char** _backtrace, size_t _backtrace_size);
    virtual ~exception() throw();

    virtual auto what() const noexcept -> const char* override;
    virtual auto backtrace() const -> const char*;

    friend auto operator<<(std::ostream& _out, zpt::exception const& _in) -> std::ostream& {
        _out << _in.what() << std::flush;
        if (zpt::log_lvl >= zpt::debug) { _out << std::endl << _in.backtrace() << std::flush; }
        return _out;
    }

  private:
    std::string __what;
    std::string __backtrace;
};

} // namespace zpt
