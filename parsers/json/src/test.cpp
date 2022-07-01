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
#include <unistd.h>
#include <chrono>

#include <semaphore.h>
#include <zapata/json.h>

int
main(int argc, char* argv[]) {
    try {
        if (argc > 1) {
            auto _parameters = zpt::parameters::parse(
              argc, argv, { "--print", { "options", { zpt::array, "optional", "single" } } });

            auto _init = "null"_JSON;

            auto _t = std::chrono::high_resolution_clock::now();
            auto _parsing_duration =
              std::chrono::duration_cast<std::chrono::microseconds>(_t - _t).count();
            auto _stringify_duration =
              std::chrono::duration_cast<std::chrono::microseconds>(_t - _t).count();

            for (auto [_, __, _file] : _parameters["--"]) {
                zpt::json _ptr;
                std::ifstream _in;
                _in.open(static_cast<std::string>(_file));
                expect(_in.is_open(),
                       std::string{ "unable to open provided file: " } +
                         static_cast<std::string>(_file),
                       500,
                       0);
                try {
                    auto _t1 = std::chrono::high_resolution_clock::now();
                    _in >> _ptr;
                    auto _t2 = std::chrono::high_resolution_clock::now();
                    _parsing_duration +=
                      std::chrono::duration_cast<std::chrono::microseconds>(_t2 - _t1).count();
                    zpt::conf::dirs(_ptr);
                    zpt::conf::env(_ptr);
                }
                catch (zpt::SyntaxErrorException const& _e) {
                    std::cout << "syntax error in '" << _file << "', " << _e.what() << std::endl
                              << std::flush;
                    return -1;
                }
                {
                    std::ostringstream _oss;
                    auto _t1 = std::chrono::high_resolution_clock::now();
                    _oss << _ptr << std::flush;
                    auto _t2 = std::chrono::high_resolution_clock::now();
                    _stringify_duration +=
                      std::chrono::duration_cast<std::chrono::microseconds>(_t2 - _t1).count();
                }
                if (_parameters["--print"] != zpt::undefined) {
                    std::cout << (_parameters["--print"] == "pretty"
                                    ? static_cast<std::string>(zpt::pretty{ _ptr })
                                    : static_cast<std::string>(_ptr))
                              << std::flush;
                }
            }
            std::cout << "parsing total time: " << _parsing_duration << "µs" << std::endl
                      << std::flush;
            std::cout << "stringify total time: " << _stringify_duration << "µs" << std::endl
                      << std::flush;
        }
    }
    catch (zpt::SyntaxErrorException const& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    catch (zpt::failed_expectation const& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    return 0;
}
