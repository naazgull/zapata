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

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <zapata/log/log.h>
#include <zapata/text/convert.h>

namespace zpt {
short int log_lvl = 0;
std::ostream* log_fd = &std::cout;
long log_pid = 0;
std::unique_ptr<std::string> log_pname = nullptr;
std::string log_hname;
short log_format = 0;

const char* log_lvl_names[] = { "EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARNING",
                                "NOTICE",    "INFO",  "DEBUG",    "TRACE", "VERBOSE" };
} // namespace zpt

namespace {

/**
 * @brief Returns the ANSI SGR color code for a log level.
 * @param _l The severity level.
 * @return The color sequence (or an empty string for unknown levels).
 */
auto level_sgr(zpt::LogLevel _l) -> const char* {
    switch (_l) {
        case zpt::emergency:                     // fallthrough
        case zpt::alert:                         // fallthrough
        case zpt::critical: return "\033[0;31m"; // red
        case zpt::error: return "\033[0;35m";    // magenta
        case zpt::warning: return "\033[0;33m";  // yellow
        case zpt::notice: return "\033[1;37m";   // bold white
        case zpt::info: return "\033[0;37m";     // white
        case zpt::debug: return "\033[0;34m";    // blue
        case zpt::trace:                         // fallthrough
        case zpt::verbose: return "\033[0;90m";  // grey
        default: return "";
    }
}

/**
 * @brief Decides whether the current output should be colored.
 * @param _fmt The active log format (2 = JSON).
 * @return True when color is enabled.
 *
 * Color is disabled for the JSON format (which must stay machine-parseable),
 * when NO_COLOR is set, and when stdout is not an interactive terminal.
 */
auto color_on(short _fmt) -> bool {
    if (_fmt == 2) { return false; }
    if (std::getenv("NO_COLOR") != nullptr) { return false; }
    return ::isatty(::fileno(stdout));
}

} // namespace

auto zpt::log(std::string const& _text,
              zpt::LogLevel _level,
              std::string const& _host,
              int _line,
              std::string const& _file) -> int {
    if (zpt::log_fd == nullptr) { return -1; }

    bool _colored = color_on(zpt::log_format);

    if (!zpt::log_format) {
        std::ostringstream _oss;
        if (_colored) { _oss << level_sgr(_level); }
        _oss << "[" << zpt::log_lvl_names[_level] << "] " << _text << std::endl << std::flush;
        if (_colored) { _oss << "\033[0m"; }
        (*zpt::log_fd) << _oss.str() << std::flush;
        return 0;
    }

    struct timeval _tp;
    gettimeofday(&_tp, nullptr);

    if (zpt::log_format == 1) {
        std::ostringstream _oss;
        if (_colored) { _oss << level_sgr(_level); }
        _oss << zpt::tostr(time(nullptr), "%b %d %H:%M:%S") << " " << _host << " "
             << *zpt::log_pname << "[" << zpt::log_pid << "]: [" << zpt::log_lvl_names[_level]
             << "] " << _text << " (" << std::filesystem::path(_file).filename().string() << ":"
             << _line << ")" << std::endl
             << std::flush;
        if (_colored) { _oss << "\033[0m"; }
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

auto zpt::log_hostname() -> std::string {
    if (zpt::log_hname.length() == 0) {
        char buffer[65];
        bzero(buffer, 65);
        gethostname(buffer, 64);
        zpt::log_hname.assign(buffer);
    }
    return zpt::log_hname;
}

extern "C" auto zpt_base() -> int { return 1; }
