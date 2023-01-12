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

#include <sys/time.h>
#include <time.h>
#include <sstream>
#include <execinfo.h>
#include <ctime>
#include <cxxabi.h>

#include <zapata/base/expect.h>
#include <zapata/text/convert.h>

auto zpt::tostr(std::string& s, int i) -> void {
    std::ostringstream _oss;
    _oss << i << std::flush;
    s.insert(s.length(), _oss.str());
}

auto zpt::tostr(std::string& s, bool b) -> void { s.insert(s.length(), b ? "true" : "false"); }

auto zpt::tostr(std::string& s, int i, std::ios_base& (&hex)(std::ios_base&)) -> void {
    char oss[512];
    sprintf(oss, "%x", i);
    s.insert(s.length(), oss);
}

#ifdef __LP64__
auto zpt::tostr(std::string& s, unsigned int i) -> void {
    std::ostringstream _oss;
    _oss << i << std::flush;
    s.insert(s.length(), _oss.str());
}
#endif

auto zpt::tostr(std::string& s, size_t i) -> void {
    std::ostringstream _oss;
    _oss << i << std::flush;
    s.insert(s.length(), _oss.str());
}

auto zpt::tostr(std::string& s, long i) -> void {
    std::ostringstream _oss;
    _oss << i << std::flush;
    s.insert(s.length(), _oss.str());
}

auto zpt::tostr(std::string& s, long long i) -> void {
    std::ostringstream _oss;
    _oss << i << std::flush;
    s.insert(s.length(), _oss.str());
}

auto zpt::tostr(std::string& s, float i, int precision) -> void {
    std::ostringstream _oss;
    _oss << std::fixed << std::setprecision(precision) << i << std::flush;
    s.insert(s.length(), _oss.str());
}

auto zpt::tostr(std::string& s, double i, int precision) -> void {
    std::ostringstream _oss;
    _oss << std::fixed << std::setprecision(precision) << i << std::flush;
    s.insert(s.length(), _oss.str());
}

auto zpt::tostr(std::string& s, char i) -> void {
    std::ostringstream _oss;
    _oss << i << std::flush;
    s.insert(s.length(), _oss.str());
}

auto zpt::tostr(std::string& s, time_t i, const char* f) -> void {
    struct tm _ptm;
    char _buffer_date[80];
    bzero(_buffer_date, 80);
    localtime_r(&i, &_ptm);
    strftime(_buffer_date, 80, f, &_ptm);
    s.insert(s.length(), _buffer_date);
}

auto zpt::tostr(int i) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}

auto zpt::tostr(bool b) -> std::string { return b ? "true" : "false"; }

auto zpt::tostr(int i, std::ios_base& (&hex)(std::ios_base&)) -> std::string {
    char oss[512];
    sprintf(oss, "%x", i);
    return std::string(oss);
}

#ifdef __LP64__
auto zpt::tostr(unsigned int i) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}
#endif

auto zpt::tostr(size_t i) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}

auto zpt::tostr(long i) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}

auto zpt::tostr(long long i) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}

auto zpt::tostr(float i, int precision) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}

auto zpt::tostr(double i, int precision) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}

auto zpt::tostr(char i) -> std::string {
    std::ostringstream _oss;
    _oss << i << std::flush;
    return _oss.str();
}

auto zpt::tostr(time_t i, const char* f) -> std::string {
    struct tm _ptm;
    char _buffer_date[80];
    bzero(_buffer_date, 80);
    localtime_r(&i, &_ptm);
    strftime(_buffer_date, 80, f, &_ptm);
    return std::string(_buffer_date);
}

auto zpt::fromstr(std::string s, int* i) -> void {
    size_t sz = 0;
    try {
        auto r = std::stoi(s, &sz);
        if (sz != s.length()) { return; }
        *i = r;
    }
    catch (std::exception const& _e) {
    }
}

#ifdef __LP64__
auto zpt::fromstr(std::string s, unsigned int* i) -> void {
    size_t sz = 0;
    try {
        auto r = std::stoul(s, &sz);
        if (sz != s.length()) { return; }
        *i = r;
    }
    catch (std::exception const& _e) {
    }
}
#endif

auto zpt::fromstr(std::string s, size_t* i) -> void {
    size_t sz = 0;
    try {
        auto r = std::stoull(s, &sz);
        if (sz != s.length()) { return; }
        *i = r;
    }
    catch (std::exception const& _e) {
    }
}

auto zpt::fromstr(std::string s, long* i) -> void {
    size_t sz = 0;
    try {
        auto r = std::stol(s, &sz);
        if (sz != s.length()) { return; }
        *i = r;
    }
    catch (std::exception const& _e) {
    }
}

auto zpt::fromstr(std::string s, long long* i) -> void {
    size_t sz = 0;
    try {
        int r = std::stoll(s, &sz);
        if (sz != s.length()) { return; }
        *i = r;
    }
    catch (std::exception const& _e) {
    }
}

auto zpt::fromstr(std::string s, float* i) -> void {
    size_t sz = 0;
    try {
        auto r = std::stof(s, &sz);
        if (sz != s.length()) { return; }
        *i = r;
    }
    catch (std::exception const& _e) {
    }
}

auto zpt::fromstr(std::string s, double* i) -> void {
    size_t sz = 0;
    try {
        auto r = std::stod(s, &sz);
        if (sz != s.length()) { return; }
        *i = r;
    }
    catch (std::exception const& _e) {
    }
}

auto zpt::fromstr(std::string s, char* i) -> void {
    std::istringstream _in;
    _in.str(s);
    _in >> (*i);
}

auto zpt::fromstr(std::string s, bool* i) -> void { *i = s == std::string("true"); }

auto zpt::fromstr(std::string s, time_t* i, const char* f, bool _no_timezone) -> void {
    /*
      setenv("TZ", "UTC", 1);
      tzset();
    */
    if (_no_timezone) {
        auto _localtime = time(nullptr);
        struct tm* local_tm = std::localtime(&_localtime);
        struct tm tm[1] = { { 0 } };
        strptime(s.data(), f, tm);
        *i = std::mktime(tm) - (local_tm->tm_isdst ? 3600 : 0);
    }
    else {
        auto _localtime = time(nullptr);
        struct tm* local_tm = std::localtime(&_localtime);
        struct tm tm[1] = { { 0 } };
        strptime(s.data(), f, tm);
        auto _local_offset = local_tm->tm_gmtoff - (local_tm->tm_isdst ? 3600 : 0);
        auto _target_offset = tm->tm_gmtoff - (tm->tm_isdst ? 3600 : 0);
        *i = std::mktime(tm) - (_target_offset - _local_offset);
    }
}

auto zpt::timezone_offset() -> time_t {
    time_t t{ 0 };
    tm* ptr{ nullptr };
    int day{ 0 };
    unsigned long num[2];
    t = time(nullptr);
    ptr = gmtime(&t); // Standard UTC time
    // Get difference in seconds
    num[0] = (ptr->tm_hour * 3600) + (ptr->tm_min * 60);
    day = ptr->tm_mday;
    ptr = localtime(&t); // Local time w/ time zone
    num[1] = (ptr->tm_hour * 3600) + (ptr->tm_min * 60);
    // If not the same then get difference
    if (day == ptr->tm_mday) { // No date difference
        if (num[0] < num[1]) {
            return (num[1] - num[0]); // Positive ex. CUT +1
        }
        else if (num[0] > num[1]) {
            return -(num[0] - num[1]); // Negative ex. Pacific -8
        }
    }
    else if (day < ptr->tm_mday) { // Ex. 1: 30 am Jan 1 : 11: 30 pm Dec 31
        return (86400 - num[0]) + num[1];
    }
    else {
        return -((86400 - num[1]) + num[0]); // Opposite
    }
    return 0;
}

auto zpt::demangle(std::string const& _mangled) -> std::string {
    int _state{ -4 };
    auto _demangled = abi::__cxa_demangle(_mangled.data(), nullptr, nullptr, &_state);
    std::string _return{ _state == 0 ? _demangled : _mangled };
    ::free(_demangled);
    return _return;
}

auto zpt::get_backtrace(unsigned int _skip) -> std::string {
    static const std::regex _line_rgx{ "([^(]+)\\(([^)]+)\\) \\[([^\\]]+)\\](.*)",
                                       std::regex_constants::ECMAScript |
                                         std::regex_constants::optimize };
    std::ostringstream _oss;
    void* _backtrace_array[30];
    size_t _backtrace_size = ::backtrace(_backtrace_array, 30);
    char** _backtrace = ::backtrace_symbols(_backtrace_array, _backtrace_size);
    for (size_t _i = 0; _i != _backtrace_size; ++_i, --_skip) {
        if (_skip > 0) {
            continue;
        }
        std::smatch _matches;
        std::string _line{ _backtrace[_i] };
        if (std::regex_match(_line, _matches, _line_rgx)) {
            auto _plus_idx = _matches[2].str().find("+");
            _oss << "\t" << zpt::demangle(_matches[2].str().substr(0, _plus_idx)) << " ["
                 << _matches[3] << "+" << _matches[2].str().substr(_plus_idx) << "] " << _matches[1]
                 << std::endl;
        }
        else { _oss << "\t" << _line << std::endl; }
    }
    _oss << std::flush;
    free(_backtrace);
    return _oss.str();
}
