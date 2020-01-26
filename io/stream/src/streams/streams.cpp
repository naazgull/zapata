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

#include <zapata/streams/streams.h>

zpt::stream::stream(zpt::stream const& _rhs)
  : __underlying{ _rhs.__underlying }
  , __fd{ _rhs.__fd } {}

zpt::stream::stream(zpt::stream&& _rhs)
  : __underlying{ std::move(_rhs.__underlying) }
  , __fd{ _rhs.__fd } {}

auto
zpt::stream::operator=(zpt::stream const& _rhs) -> zpt::stream& {
    this->__underlying = _rhs.__underlying;
    this->__fd = _rhs.__fd;
    return (*this);
}

auto
zpt::stream::operator=(zpt::stream&& _rhs) -> zpt::stream& {
    this->__underlying = std::move(_rhs.__underlying);
    this->__fd = _rhs.__fd;
    _rhs.__fd = -1;
    return (*this);
}

auto
zpt::stream::operator=(int _rhs) -> zpt::stream& {
    this->__fd = _rhs;
    return (*this);
}

auto
zpt::stream::operator<<(ostream_manipulator _in) -> zpt::stream& {
    (*this->__underlying.get()) << _in;
    return (*this);
}

auto zpt::stream::operator-> () -> std::iostream* {
    return this->__underlying.get();
}

auto zpt::stream::operator*() -> std::iostream& {
    return *this->__underlying.get();
}

zpt::stream::operator int() {
    return this->__fd;
}

zpt::stream::stream(std::unique_ptr<std::iostream> _underlying)
  : __underlying{ _underlying.release() }
  , __fd{ -1 } {}
