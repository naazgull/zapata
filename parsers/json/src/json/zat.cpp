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

#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <zapata/json.h>
#include <zapata/log/log.h>

int
main(int argc, char* argv[]) {
    char _c;
    int _level = 8;

    while ((_c = getopt(argc, argv, "l:")) != -1) {
        switch (_c) {
            case 'l': {
                std::string _l(optarg);
                zpt::fromstr(_l, &_level);
                break;
            }
        }
    }

    std::istringstream _iss;
    std::string _line;
    while (std::getline(std::cin, _line)) {
        zpt::trim(_line);
        if (_line.find("{") != 0) { continue; }
        try {
            zpt::json _json;
            _iss.str(_line);
            _iss >> _json;

            if ((int)_json["level"] > _level) { continue; }
            double _intpart;
            double _fracpart = modf((double)_json["timestamp"], &_intpart);
            zpt::timestamp_t _ts = (_intpart * 1000) + (_fracpart * 1000);
            std::string _cmd((std::string)_json["exec"]);
            _cmd.assign(_cmd.substr(_cmd.rfind("/") + 1));
            std::cout << zpt::tostr(_ts, "%b %d %H:%M:%S") << " "
                      << static_cast<std::string>(_json["host"]) << " " << _cmd << "["
                      << static_cast<std::string>(_json["pid"])
                      << "]: " << zpt::log_lvl_names[int(_json["level"])] << " "
                      << static_cast<std::string>(_json["short_message"]) << std::endl
                      << std::flush;
        }
        catch (...) {
        }
    }
    return 0;
}
