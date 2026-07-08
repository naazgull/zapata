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

#include <zapata/prolog/helpers.h>

zpt::prolog::term::term()
  : __underlying{ PL_new_term_ref() }
  , __references{ std::make_shared<zpt::padded_atomic<size_t>>(1) } {}

zpt::prolog::term::term(term const& _rhs)
  : __underlying{ _rhs.__underlying }
  , __references{ _rhs.__references } {
    (*this->__references)->fetch_add(1);
}

zpt::prolog::term::term(term&& _rhs)
  : __underlying{ _rhs.__underlying }
  , __references{ std::move(_rhs.__references) } {
    _rhs.__underlying = 0;
}

zpt::prolog::term::~term() {
    if (this->__underlying != 0) {
        (*this->__references)->fetch_sub(1);
        if ((*this->__references)->load() == 0) { PL_free_term_ref(this->__underlying); }
    }
}

zpt::prolog::term::operator term_t() { return this->__underlying; }

auto zpt::prolog::term::operator*() -> term_t& { return this->__underlying; }

auto zpt::prolog::term::operator=(term const& _rhs) -> term& {
    this->__underlying = _rhs.__underlying;
    this->__references = std::move(_rhs.__references);
    (*this->__references)->fetch_add(1);
    return (*this);
}

auto zpt::prolog::term::operator=(term&& _rhs) -> term& {
    this->__underlying = _rhs.__underlying;
    this->__references = std::move(_rhs.__references);
    _rhs.__underlying = 0;
    return (*this);
}

auto zpt::prolog::term::operator==(term const& _rhs) -> bool {
    return this->__underlying == _rhs.__underlying;
}

auto zpt::prolog::term::operator!=(term const& _rhs) -> bool { return !((*this) == _rhs); }

auto zpt::prolog::term::null() -> term& {
    static term _return{ true };
    return _return;
}

auto zpt::prolog::term::to_string() const -> std::string {
    std::ostringstream _oss;
    return _oss.str();
}

zpt::prolog::term::term(bool)
  : __underlying{ 0 }
  , __references{ std::make_shared<zpt::padded_atomic<size_t>>(1) } {}
