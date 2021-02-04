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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <memory>
#include <thread>
#include <sstream>

#define __HOST__ std::string(zpt::log_hostname())
#define zlog(x, y)                                                                                 \
    if (y <= zpt::log_lvl) {                                                                       \
        std::ostringstream __OSS__;                                                                \
        __OSS__ << x << std::flush;                                                                \
        zpt::log(__OSS__.str(), y, __HOST__, __LINE__, __FILE__);                                  \
    }
#define zdbg(x) zlog(x, zpt::debug)
#define ztrace(x) zlog(x, zpt::trace)
#define zverbose(x) zlog(x, zpt::verbose)

namespace zpt {
extern short int log_lvl;
extern std::ostream* log_fd;
extern long log_pid;
extern std::unique_ptr<std::string> log_pname;
extern char* log_hname;
extern short log_format;

extern const char* log_lvl_names[];

enum LogLevel {
    emergency = 0,
    alert = 1,
    critical = 2,
    error = 3,
    warning = 4,
    notice = 5,
    info = 6,
    debug = 7,
    trace = 8,
    verbose = 9
};

auto
log(std::string const& _text,
    zpt::LogLevel _level,
    std::string const& _host,
    int _line,
    std::string const& _file) -> int;

template<typename T>
auto
log(T _text, zpt::LogLevel _level, std::string const& _host, int _line, std::string const& _file)
  -> int {
    return zpt::log(to_string(_text), _level, _host, _line, _file);
}
auto
log_hostname() -> char*;

namespace this_thread {
template<typename T, int Step>
class adaptive_timer {
    static_assert(std::is_integral<T>::value,
                  "Type `T` in `zpt::this_thread::adaptive_timer` must be an integral type.");

  public:
    adaptive_timer() = default;
    virtual ~adaptive_timer() = default;

    auto reset() -> zpt::this_thread::adaptive_timer<T, Step>&;
    auto sleep_for(T _upper_limit) -> T;

  private:
    T __sleep_tics{ 1 };
};

} // namespace this_thread
} // namespace zpt

template<typename T, int Step>
auto
zpt::this_thread::adaptive_timer<T, Step>::reset() -> zpt::this_thread::adaptive_timer<T, Step>& {
    this->__sleep_tics = 1;
    return (*this);
}

template<typename T, int Step>
auto
zpt::this_thread::adaptive_timer<T, Step>::sleep_for(T _upper_limit) -> T {
    static constexpr T _max{ std::numeric_limits<T>::max() - 1 };
    this->__sleep_tics += Step;
    if (this->__sleep_tics > _upper_limit) { this->__sleep_tics = _upper_limit; }
    if (this->__sleep_tics == 0) { std::this_thread::yield(); }
    else {
        std::this_thread::sleep_for(std::chrono::duration<T, std::micro>{ this->__sleep_tics });
    }
    return this->__sleep_tics;
}
