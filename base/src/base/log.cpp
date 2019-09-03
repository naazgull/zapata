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

#include <zapata/log/log.h>

#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <zapata/text/convert.h>

namespace zpt {
short int log_lvl = 8;
std::ostream* log_fd = &std::cout;
long log_pid = 0;
std::string* log_pname = nullptr;
char* log_hname = nullptr;
short log_format = 0;

const char* log_lvl_names[] = {
    "\033[4;31m emergency \033[0m | ", "\033[4;31m   alert   \033[0m | ",
    "\033[4;35m critical  \033[0m | ", "\033[4;35m   error   \033[0m | ",
    "\033[4;35m  warning  \033[0m | ", "\033[4;36m  notice   \033[0m | ",
    "\033[4;37m   info    \033[0m | ", "\033[4;33m   debug   \033[0m | ",
    "\033[4;32m   trace   \033[0m | ", "\033[4;32m  verbose  \033[0m | "
};
} // namespace zpt

auto
zpt::to_string(const char* _in) -> std::string {
    return std::string(_in);
}

int
zpt::log(std::string _text, zpt::LogLevel _level, std::string _host, int _line, std::string _file) {
    if (zpt::log_fd == nullptr) {
        return -1;
    }
    if (!zpt::log_format) {
        (*zpt::log_fd) << _text << std::endl << std::flush;
        return 0;
    }

    struct timeval _tp;
    gettimeofday(&_tp, nullptr);

    std::string _log;
    if (zpt::log_format == 1) {
        (*zpt::log_fd) << zpt::log_lvl_names[_level] << "\033[1;37m"
                       << zpt::tostr(time(nullptr), "%Y-%m-%dT%H:%M:%S") << "\033[0m | " << _text
                       << " | " << *zpt::log_pname << ":" << zpt::log_pid << " " << _file << ":"
                       << _line << std::endl
                       << std::flush;
    }
    else {
        zpt::replace(_text, "\n", "\\n");
        zpt::replace(_text, "\"", "\\\"");

        _log.assign("{\"version\":\"1.1\",\"host\":\"");
        _log.insert(_log.length(), _host);
        _log.insert(_log.length(), "\",\"source\":\"");
        _log.insert(_log.length(), _host);
        _log.insert(_log.length(), "\",\"short_message\":\"");
        _log.insert(_log.length(), _text);
        _log.insert(_log.length(), "\",\"full_message\":\"");
        _log.insert(_log.length(), _file);
        _log.insert(_log.length(), ":");
        zpt::tostr(_log, _line);
        _log.insert(_log.length(), " | ");
        _log.insert(_log.length(), _text);
        _log.insert(_log.length(), "\",\"timestamp\":");
        zpt::tostr(_log, _tp.tv_sec);
        _log.insert(_log.length(), ".");
        zpt::tostr(_log, (int)(_tp.tv_usec / 1000));
        _log.insert(_log.length(), ",\"level\":");
        zpt::tostr(_log, (int)_level);
        _log.insert(_log.length(), ",\"pid\":");
        zpt::tostr(_log, zpt::log_pid);
        _log.insert(_log.length(), ",\"exec\":\"");
        _log.insert(_log.length(), *zpt::log_pname);
        _log.insert(_log.length(), "\",\"file\":\"");
        _log.insert(_log.length(), _file);
        _log.insert(_log.length(), "\",\"line\":");
        zpt::tostr(_log, _line);
        _log.insert(_log.length(), "}");
        (*zpt::log_fd) << _log << std::endl << std::flush;
    }
    return 0;
}

char*
zpt::log_hostname() {
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
