/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <memory>
#include <zapata/base/config.h>
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

using std::to_string;

auto
to_string(const char* _in) -> std::string;

int
log(std::string const& _text,
    zpt::LogLevel _level,
    std::string const& _host,
    int _line,
    std::string const& _file);
template<typename T>
int
log(T _text, zpt::LogLevel _level, std::string const& _host, int _line, std::string const& _file) {
    return zpt::log(to_string(_text), _level, _host, _line, _file);
}
char*
log_hostname();

namespace this_thread {
template<typename T>
auto
adaptive_sleep_for(T _iteration_counter, T _upper_limit = -1) -> T;

} // namespace this_thread
} // namespace zpt

template<typename T>
auto
zpt::this_thread::adaptive_sleep_for(T _iteration_counter, T _upper_limit) -> T {
    static constexpr T _max{ std::numeric_limits<T>::max() - 1 };
    if (_iteration_counter != _max) {
        ++_iteration_counter;
    }
    std::this_thread::sleep_for(std::chrono::duration<T, std::micro>{
      _upper_limit > 0 ? std::min(_iteration_counter, _upper_limit) : _iteration_counter });
    return _iteration_counter;
}
