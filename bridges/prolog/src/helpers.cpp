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

#include <SWI-Stream.h>
#include <zapata/prolog/helpers.h>

zpt::prolog::term::term()
  : __underlying{ PL_new_term_ref() }
  , __references{ std::make_shared<zpt::padded_atomic<size_t>>(1) } {}

zpt::prolog::term::term(term_t _to_assign)
  : __underlying{ _to_assign }
  , __references{ std::make_shared<zpt::padded_atomic<size_t>>(1) } {}

zpt::prolog::term::term(std::string const& _to_parse)
  : __underlying{ PL_new_term_ref() }
  , __references{ std::make_shared<zpt::padded_atomic<size_t>>(1) } {
    expect(PL_chars_to_term(_to_parse.data(), this->__underlying), "unable to parse Prolog string");
}

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

auto zpt::prolog::term::operator=(term_t _rhs) -> term& {
    expect((*this->__references)->load() == 1,
           "can't assign a raw `term_t` to an already shared term");
    PL_free_term_ref(this->__underlying);
    this->__underlying = _rhs;
    return (*this);
}

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

auto zpt::prolog::term::emplace() -> term { return this->__children.emplace_back(); }

auto zpt::prolog::term::add(term const& _to_add) -> term& {
    this->__children.push_back(_to_add);
    return (*this);
}

auto zpt::prolog::term::to_string() const -> std::string {
    if (this->__underlying == 0) { return ""; }
    return zpt::prolog::term_to_string(this->__underlying);
}

auto zpt::prolog::term::null() -> term& {
    static term _return{ 0 };
    return _return;
}

auto zpt::prolog::term_to_string(term_t _to_convert) -> std::string {
    char* _buffer{ nullptr };
    size_t _size{ 0 };
    IOSTREAM* _stream = Sopenmem(&_buffer, &_size, "w");
    expect(_stream, "couldn't open memory stream");

    expect(PL_write_term(_stream, _to_convert, 1200, PL_WRT_QUOTED),
           "couldn't write term to string");
    Sflush(_stream);
    Sclose(_stream);

    std::string _result{ _buffer };
    free(_buffer);
    return _result;
}
