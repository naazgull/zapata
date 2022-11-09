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

#include <cstdarg>
#include <ostream>
#include <regex>
#include <zapata/json/JSONClass.h>

/*JSON POINTER TO REGEX*/
zpt::JSONRegex::JSONRegex()
  : zpt::JSONRegex::JSONRegex{ "" } {}

zpt::JSONRegex::JSONRegex(const JSONRegex& _rhs) { (*this) = _rhs; }

zpt::JSONRegex::JSONRegex(JSONRegex&& _rhs) { (*this) = _rhs; }

zpt::JSONRegex::JSONRegex(std::string const& _target)
  : __underlying_original{ _target }
  , __underlying{ std::make_shared<std::regex>(_target) } {}

zpt::JSONRegex::~JSONRegex() {}

auto zpt::JSONRegex::operator=(const zpt::JSONRegex& _rhs) -> zpt::JSONRegex& {
    this->__underlying_original = _rhs.__underlying_original;
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::JSONRegex::operator=(zpt::JSONRegex&& _rhs) -> zpt::JSONRegex& {
    this->__underlying_original = std::move(_rhs.__underlying_original);
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::JSONRegex::operator->() -> std::regex* { return this->__underlying.get(); }

auto zpt::JSONRegex::operator*() -> std::regex& { return *this->__underlying.get(); }

auto zpt::JSONRegex::operator->() const -> std::regex const* { return this->__underlying.get(); }

auto zpt::JSONRegex::operator*() const -> std::regex const& { return *this->__underlying.get(); }

zpt::JSONRegex::operator std::regex&() { return (*this->__underlying.get()); }

auto zpt::JSONRegex::JSONRegex::operator==(zpt::regex _rhs) const -> bool {
    return this->__underlying_original == _rhs.__underlying_original;
}

auto zpt::JSONRegex::JSONRegex::operator==(zpt::json _rhs) const -> bool {
    if (_rhs->type() == zpt::JSRegex) { return (*this) == _rhs->regex(); }
    else { return std::regex_match(static_cast<std::string>(_rhs), (*this->__underlying.get())); }
}

auto zpt::JSONRegex::JSONRegex::operator==(std::string const& _rhs) const -> bool {
    return std::regex_match(_rhs, (*this->__underlying.get()));
}

auto zpt::JSONRegex::JSONRegex::operator!=(zpt::regex _rhs) const -> bool { return !((*this) == _rhs); }

auto zpt::JSONRegex::JSONRegex::operator!=(zpt::json _rhs) const -> bool { return !((*this) == _rhs); }

auto zpt::JSONRegex::JSONRegex::operator!=(std::string const& _rhs) const -> bool {
    return !((*this) == _rhs);
}

auto zpt::JSONRegex::JSONRegex::to_string() const -> std::string const& {
    return this->__underlying_original;
}
