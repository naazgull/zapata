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

#include <zapata/base/expect.h>
#include <zapata/text/convert.h>
#include <zapata/uuid.h>

zpt::uuid::uuid() { this->from_string(zpt::generate::r_uuid()); }

zpt::uuid::uuid(std::string const& _str) { this->from_string(_str); }

zpt::uuid::uuid(uuid const& _rhs)
  : __base{ _rhs.__base } {}

zpt::uuid::uuid(uuid&& _rhs)
  : __base{ _rhs.__base } {
    _rhs.__base = 0;
}

auto zpt::uuid::operator=(uuid const& _rhs) -> uuid& {
    this->__base = _rhs.__base;
    return (*this);
}

auto zpt::uuid::operator=(uuid&& _rhs) -> uuid& {
    this->__base = _rhs.__base;
    _rhs.__base = 0;
    return (*this);
}

auto zpt::uuid::operator==(uuid const& _rhs) -> bool { return (this->__base == _rhs.__base); }

auto zpt::uuid::operator!=(uuid const& _rhs) -> bool { return !((*this) == _rhs); }

auto zpt::uuid::to_string() const -> std::string {
    std::ostringstream _oss;
    this->to_stream(_oss);
    return _oss.str();
}

auto zpt::uuid::from_string(std::string const& _str) -> uuid& {
    std::istringstream _iss;
    _iss.str(_str);
    return this->from_stream(_iss);
}

auto zpt::uuid::to_stream(std::ostream& _out) const -> uuid const& {
    static constexpr unsigned int _positions[] = { 4, 6, 8, 10, 16 };
    unsigned _cur{ 0 };

    _out << std::hex << std::setfill('0');
    for (size_t _idx = 0; _idx != 16; ++_idx) {
        if (_idx == _positions[_cur]) {
            _out << "-";
            ++_cur;
        }
        auto _hexa = static_cast<unsigned>((this->__base << (_idx * 8)) >> ((16 - 1) * 8));
        _out << std::setw(2) << _hexa;
    }

    return (*this);
}

auto zpt::uuid::from_stream(std::istream& _in) -> uuid& {
    static constexpr unsigned int _positions[] = { 8, 13, 18, 23, 36 };
    unsigned _cur{ 0 };
    std::stringstream _ss;

    this->__base = 0;
    for (size_t _iidx = 0, _oidx = 0; _iidx != 36; _iidx += 2, ++_oidx, _ss.str(""), _ss.clear()) {
        if (_iidx == _positions[_cur]) {
            ++_cur;
            ++_iidx;
            _in.get();
        }

        signed char _c1 = '\0';
        signed char _c2 = '\0';
        _in >> std::skipws >> _c1;
        _in >> std::skipws >> _c2;
        expect(_c1 != std::istream::traits_type::eof() && _c2 != std::istream::traits_type::eof(),
               "Parse error, reached end of UUID string");
        _ss << _c1 << _c2 << std::flush;
        unsigned short _us{ 0 };
        _ss >> std::hex >> _us;
        __uint128_t _part{ _us };
        this->__base += (_part << ((16 - 1 - _oidx) * 8));
    }

    return (*this);
}
