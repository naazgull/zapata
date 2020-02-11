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

#include <zapata/json.h>
#include <zapata/streams.h>

namespace zpt {

auto
cout() -> zpt::stream&;
auto
cerr() -> zpt::stream&;

namespace log {
// template<typename Char>
// class basic_jsonbuf : public std::basic_streambuf<Char> {
//   public:
//     typedef Char __char_type;
//     typedef std::basic_streambuf<__char_type> __buf_type;
//     typedef std::basic_ostream<__char_type> __stream_type;
//     typedef typename __buf_type::int_type __int_type;
//     typedef typename std::basic_streambuf<Char>::traits_type __traits_type;

//     basic_jsonbuf();
//     virtual ~basic_jsonbuf();

//   protected:
//     static constexpr int char_size{ sizeof(__char_type) };
//     static constexpr int SIZE{ 1024 };
//     __char_type obuf[SIZE];
//     __char_type ibuf[SIZE];

//     auto output_buffer() -> __int_type;
//     virtual auto overflow(__int_type c) -> __int_type;
//     virtual auto sync() -> int;
//     virtual auto underflow() -> __int_type;
// };

template<typename Char>
class basic_jsonstream : public std::basic_iostream<Char> {
  public:
    using __char_type = Char;
    using __stream_type = std::basic_iostream<__char_type>;
    using __buf_type = std::basic_streambuf<__char_type>;

    basic_jsonstream();
    basic_jsonstream(const zpt::log::basic_jsonstream&) = delete;
    basic_jsonstream(zpt::log::basic_jsonstream&&) = delete;
    virtual ~basic_jsonstream();

    auto operator=(const basic_jsonstream&) -> zpt::log::basic_jsonstream& = delete;
    auto operator=(basic_jsonstream &&) -> zpt::log::basic_jsonstream& = delete;

    auto operator<<(T _to_output) -> __stream_type&;

  protected:
    __buf_type __buf;
};

using jsonstream = zpt::basic_jsonstream<char>;

} // namespace log
} // namespace zpt

template<typename Char>
zpt::basic_jsonstream<Char>::basic_jsonstream()
  : __stream_type(&__buf) {}

template<typename Char>
zpt::basic_jsonstream<Char>::~basic_jsonstream() {
    this->close();
}
