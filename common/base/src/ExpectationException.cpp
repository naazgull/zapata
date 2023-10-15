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

#include <zapata/exceptions/ExpectationException.h>

#include <sstream>
#include <zapata/text/convert.h>

zpt::ExpectationException::ExpectationException(std::string const& _what,
                                                std::string _desc,
                                                int _line,
                                                std::string _file)
  : zpt::exception{ _what }
  , __description(_desc)
  , __line(_line)
  , __file(_file) {
    zpt::replace(this->__description, "\"", "");
    this->__description.insert(0, "expected `");
    if (this->__line != 0) {
        this->__description.insert(this->__description.length(), "` to be true on file ");
        this->__description.insert(this->__description.length(), this->__file);
        this->__description.insert(this->__description.length(), ", line ");
        zpt::tostr(this->__description, this->__line);
    }
}

zpt::ExpectationException::~ExpectationException() throw() {}

auto zpt::ExpectationException::description() const -> const char* {
    return this->__description.data();
}
