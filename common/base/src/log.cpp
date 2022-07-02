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

#include <zapata/log/log.h>

#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <zapata/text/convert.h>

namespace zpt {
short int log_lvl = 8;
std::ostream* log_fd = &std::cout;
long log_pid = 0;
std::unique_ptr<std::string> log_pname = nullptr;
char* log_hname = nullptr;
short log_format = 0;

const char* log_lvl_names[] = { "EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARNING",
                                "NOTICE",    "INFO",  "DEBUG",    "TRACE", "VERBOSE" };
} // namespace zpt

auto
zpt::log(std::string const& _text,
         zpt::LogLevel _level,
         std::string const& _host,
         int _line,
         std::string const& _file) -> int {
    if (zpt::log_fd == nullptr) { return -1; }
    if (!zpt::log_format) {
        std::ostringstream _oss;
        _oss << zpt::log_lvl_names[_level] << " " << _text << std::endl << std::flush;
        (*zpt::log_fd) << _oss.str() << std::flush;
        return 0;
    }

    struct timeval _tp;
    gettimeofday(&_tp, nullptr);

    if (zpt::log_format == 1) {
        std::ostringstream _oss;
        _oss << zpt::tostr(time(nullptr), "%b %d %H:%M:%S") << " " << _host << " "
             << *zpt::log_pname << "[" << zpt::log_pid << "]: " << zpt::log_lvl_names[_level] << " "
             << _text;
        if (_level > 6) { _oss << " " << _file << ":" << _line; }
        _oss << std::endl << std::flush;
        (*zpt::log_fd) << _oss.str() << std::flush;
    }
    else {
        std::ostringstream _oss;
        _oss << "{\"version\":\"1.1\",\"host\":\"" << _host << "\",\"source\":\"" << _host
             << "\",\"short_message\":\"" << _text << "\",\"full_message\":\"" << _file << ":"
             << _line << " | " << zpt::r_replace(zpt::r_replace(_text, "\n", "\\n"), "\"", "\\\"")
             << "\",\"timestamp\":" << _tp.tv_sec << "." << (int)(_tp.tv_usec / 1000)
             << ",\"level\":" << (int)_level << ",\"pid\":" << zpt::log_pid << ",\"exec\":\""
             << *zpt::log_pname << "\",\"file\":\"" << _file << "\",\"line\":" << _line << "}"
             << std::endl
             << std::flush;
        (*zpt::log_fd) << _oss.str() << std::flush;
    }
    return 0;
}

auto
zpt::log_hostname() -> char* {
    if (zpt::log_hname == nullptr) {
        zpt::log_hname = new char[65];
        bzero(zpt::log_hname, 65);
        gethostname(zpt::log_hname, 64);
    }
    return zpt::log_hname;
}

extern "C" auto
zpt_base() -> int {
    return 1;
}
