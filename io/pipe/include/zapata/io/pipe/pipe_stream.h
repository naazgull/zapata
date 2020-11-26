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

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <istream>
#include <ostream>
#include <streambuf>
#include <stdio.h>
#include <unistd.h>
#include <zapata/base/expect.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/log/log.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>
#include <zapata/streams.h>

namespace zpt {

template<typename Char>
class basic_pipestream;

template<typename Char>
class basic_pipebuf : public std::basic_streambuf<Char> {
  public:
    using char_type = Char;
    using buf_type = std::basic_streambuf<char_type>;
    using int_type = typename buf_type::int_type;
    using traits_type = typename buf_type::traits_type;

    friend class basic_pipestream<char_type>;

    basic_pipebuf(bool initialize = true);
    basic_pipebuf(const basic_pipebuf<char_type>&);
    basic_pipebuf(basic_pipebuf<char_type>&&) = delete;
    virtual ~basic_pipebuf() override;

    auto operator=(const basic_pipebuf<char_type>&) -> basic_pipebuf<char_type>&;
    auto operator=(basic_pipebuf<char_type>&&) -> basic_pipebuf<char_type>& = delete;

    virtual auto open() -> void;
    virtual auto close() -> void;
    virtual auto output_fd() -> int;
    virtual auto input_fd() -> int;

  protected:
    // virtual std::streamsize xsputn(const char_type *to_put,
    //                                std::streamsize count) override;
    virtual auto overflow(int_type c) -> int_type override;
    virtual auto underflow() -> int_type override;
    virtual auto sync() -> int override;
    // virtual traits_pos_type seekoff(
    //     traits_off_type off, std::ios_base::seekdir dir,
    //     std::ios_base::openmode which = std::ios_base::in |
    //                                     std::ios_base::out) override;
    // virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir,
    //                          std::ios_base::openmode which = std::ios_base::in |
    //                                                          std::ios_base::out);
    // virtual traits_pos_type seekpos(
    //     traits_pos_type pos,
    //     std::ios_base::openmode which = std::ios_base::in |
    //                                     std::ios_base::out) override;
    // virtual pos_type seekpos(pos_type pos,
    //                          std::ios_base::openmode which = std::ios_base::in |
    //                                                          std::ios_base::out);
    // virtual basic_pipebuf<Char> &pbump(pos_type to_bump, int direction);

  private:
    char_type __obuf[1024] = { 0 };
    char_type __ibuf[1024] = { 0 };
    int __fds[2] = { 0 };

    auto dump() -> int_type;
};

template<typename Char>
class basic_pipestream : public std::basic_iostream<Char> {
  public:
    using char_type = Char;
    using stream_type = std::basic_iostream<char_type>;
    using buf_type = basic_pipebuf<char_type>;
    using int_type = typename buf_type::int_type;
    using pos_type = typename buf_type::pos_type;

    basic_pipestream();
    basic_pipestream(std::string const& _pipe_name);
    basic_pipestream(const basic_pipestream<char_type>&);
    basic_pipestream(basic_pipestream<char_type>&&) = delete;
    virtual ~basic_pipestream() override;

    auto operator=(const basic_pipestream<char_type>&) -> basic_pipestream<char_type>&;
    auto operator=(basic_pipestream<char_type>&&) -> basic_pipestream<char_type>& = delete;

    operator int();
    operator std::string();

    virtual auto open(std::string const& _pipe_name) -> void;
    virtual auto close() -> void;
    virtual auto is_open() -> bool;

  private:
    buf_type __buf{ false };
    std::string __uuid;
};

using pipestream = zpt::basic_pipestream<char>;

} // namespace zpt

template<typename Char>
zpt::basic_pipebuf<Char>::basic_pipebuf(bool initialize) {
    if (initialize) { this->open(); }
    buf_type::setp(this->__obuf, this->__obuf + 1023);
    buf_type::setg(this->__ibuf, this->__ibuf, this->__ibuf);
}

template<typename Char>
zpt::basic_pipebuf<Char>::basic_pipebuf(const basic_pipebuf<char_type>& _rhs)
  : basic_pipebuf{ false } {
    this->__fds[0] = _rhs.__fds[0];
    this->__fds[1] = _rhs.__fds[1];
}

template<typename Char>
zpt::basic_pipebuf<Char>::~basic_pipebuf() {}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::operator=(const basic_pipebuf<char_type>& _rhs)
  -> basic_pipebuf<char_type>& {
    this->__fds[0] = _rhs.__fds[0];
    this->__fds[1] = _rhs.__fds[1];
    return (*this);
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::open() -> void {
    expect(::pipe(this->__fds) != -1, "unable to create the pipe", 500, 0);
    fcntl(this->__fds[0], F_SETFL, O_NONBLOCK);
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::close() -> void {
    if (this->__fds[0]) { ::close(this->__fds[0]); }
    if (this->__fds[1]) { ::close(this->__fds[1]); }
    this->__fds[0] = 0;
    this->__fds[1] = 0;
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::output_fd() -> int {
    return this->__fds[1];
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::input_fd() -> int {
    return this->__fds[0];
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::overflow(int_type _c) -> int_type {
    if (_c != traits_type::eof()) {
        *buf_type::pptr() = _c;
        buf_type::pbump(1);
    }
    return this->dump();
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::underflow() -> int_type {
    if (buf_type::gptr() < buf_type::egptr()) { return *buf_type::gptr(); }
    if (!this->__fds[0]) { return traits_type::eof(); }

    auto _actually_read = -1;
    if ((_actually_read = ::read(this->__fds[0], this->__ibuf, 1024)) <= 0) {
        return traits_type::eof();
    }
    if (_actually_read == 0) { return traits_type::eof(); }
    buf_type::setg(this->__ibuf, this->__ibuf, this->__ibuf + _actually_read);
    return *buf_type::gptr();
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::sync() -> int {
    if (this->dump() == traits_type::eof()) { return traits_type::eof(); }
    return 0;
}

template<typename Char>
auto
zpt::basic_pipebuf<Char>::dump() -> int_type {
    if (!this->__fds[1]) { return traits_type::eof(); }
    auto _num = buf_type::pptr() - buf_type::pbase();
    auto _actually_written = -1;
    if ((_actually_written = ::write(this->__fds[1], this->__obuf, _num)) < 0) {
        return traits_type::eof();
    }
    buf_type::pbump(-_actually_written);
    return _actually_written;
}

template<typename Char>
zpt::basic_pipestream<Char>::basic_pipestream()
  : stream_type{ &__buf } {}

template<typename Char>
zpt::basic_pipestream<Char>::basic_pipestream(std::string const& _pipe_name)
  : stream_type{ &__buf }
  , __uuid{ _pipe_name } {
    this->__buf.open();
}

template<typename Char>
zpt::basic_pipestream<Char>::basic_pipestream(const basic_pipestream<char_type>& _rhs)
  : stream_type{ &__buf }
  , __uuid{ _rhs.__uuid } {
    this->__buf = _rhs.__buf;
}

template<typename Char>
auto
zpt::basic_pipestream<Char>::operator=(const basic_pipestream<char_type>& _rhs)
  -> basic_pipestream<char_type>& {
    this->__uuid = _rhs.__uuid;
    this->__buf = _rhs.__buf;
}

template<typename Char>
zpt::basic_pipestream<Char>::~basic_pipestream() {}

template<typename Char>
zpt::basic_pipestream<Char>::operator int() {
    return this->__buf.input_fd();
}

template<typename Char>
zpt::basic_pipestream<Char>::operator std::string() {
    return std::string{ "pipe:/" } + this->__uuid;
}

template<typename Char>
auto
zpt::basic_pipestream<Char>::open(std::string const& _pipe_name) -> void {
    this->__uuid.assign(_pipe_name);
    this->__buf.close();
    this->__buf.open();
}

template<typename Char>
auto
zpt::basic_pipestream<Char>::close() -> void {
    this->__buf.close();
}

template<typename Char>
auto
zpt::basic_pipestream<Char>::is_open() -> bool {
    return this->__buf.output_fd() != 0 && this->__buf.input_fd() != 0;
}
