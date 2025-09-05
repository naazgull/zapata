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
#include <atomic>
#include <any>
#include <zapata/streams/streams.h>

namespace zpt {
class event_stream : public basic_stream {
  public:
    event_stream();
    event_stream(event_stream const& _rhs) = delete;
    event_stream(event_stream&& _rhs) = delete;
    virtual ~event_stream();

    auto operator=(event_stream const& _rhs) -> event_stream& = delete;
    auto operator=(event_stream&& _rhs) -> event_stream& = delete;

    auto operator=(int _rhs) -> event_stream&;
    template<typename T>
    auto read(T& _out) -> event_stream&;
    template<typename T>
    auto write(T _in) -> event_stream&;
    template<typename T>
    auto operator>>(T& _out) -> event_stream&;
    template<typename T>
    auto operator<<(T _in) -> event_stream&;
    auto operator<<(ostream_manipulator _in) -> event_stream&;

  private:
    std::any __content;
};
} // namespace zpt

template<typename T>
auto zpt::event_stream::read(T& _out) -> zpt::event_stream& {
    _out = std::any_cast<T>(this->__content);
    return (*this);
}

template<typename T>
auto zpt::event_stream::write(T _in) -> zpt::event_stream& {
    this->__content = _in;
    return (*this);
}

template<typename T>
auto zpt::event_stream::operator>>(T& _out) -> zpt::event_stream& {
    return this->read<T>(_out);
}

template<typename T>
auto zpt::event_stream::operator<<(T _in) -> zpt::event_stream& {
    return this->write<T>(_in);
}
