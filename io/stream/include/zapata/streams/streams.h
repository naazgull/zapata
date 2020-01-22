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

#include <iostream>
#include <memory>

namespace zpt {

class stream {
  public:
    typedef std::ostream& (*ostream_manipulator)(std::ostream&);

    stream(zpt::stream const& _rhs);
    stream(zpt::stream&& _rhs);
    virtual ~stream() = default;

    auto operator=(zpt::stream const& _rhs) -> zpt::stream&;
    auto operator=(zpt::stream&& _rhs) -> zpt::stream&;
    auto operator=(int _rhs) -> zpt::stream&;
    template<typename T>
    auto operator>>(T& _out) -> zpt::stream&;
    template<typename T>
    auto operator<<(T _in) -> zpt::stream&;
    auto operator<<(ostream_manipulator _in) -> zpt::stream&;
    auto operator-> () -> std::iostream*;
    auto operator*() -> std::iostream&;

    operator int();

    template<typename T, typename... Args>
    static auto alloc(Args... _args) -> zpt::stream;

  private:
    stream(std::unique_ptr<std::iostream> _underlying);

    std::shared_ptr<std::iostream> __underlying;
    int __fd{ -1 };
};

#define CRLF "\r\n"
} // namespace zpt

template<typename T>
auto
stream_cast(zpt::stream& _rhs) -> T& {
    return *static_cast<T*>(&(*_rhs));
}

template<typename T>
auto
zpt::stream::operator>>(T& _out) -> zpt::stream& {
    (*this->__underlying.get()) >> _out;
    return (*this);
}

template<typename T>
auto
zpt::stream::operator<<(T _in) -> zpt::stream& {
    (*this->__underlying.get()) << _in;
    return (*this);
}

template<typename T, typename... Args>
auto
zpt::stream::alloc(Args... _args) -> zpt::stream {
    zpt::stream _to_return{ std::make_unique<T>(_args...) };
    if constexpr (std::is_convertible<T, int>::value) {
        _to_return = static_cast<int>(static_cast<T&>(*_to_return));
    }
    return _to_return;
}
