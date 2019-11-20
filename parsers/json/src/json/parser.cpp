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
            zpt::json _parameters = zpt::parameters::parse(
              argc, argv, { "--print", { zpt::array, "optional", "single" } });

            zpt::json _init;
            std::istringstream _iss;
            _iss.str("{}");
            _iss >> _init;

            auto _t = std::chrono::high_resolution_clock::now();
            auto _parsing_duration =
              std::chrono::duration_cast<std::chrono::milliseconds>(_t - _t).count();
            auto _stringify_duration =
              std::chrono::duration_cast<std::chrono::milliseconds>(_t - _t).count();

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
                      std::chrono::duration_cast<std::chrono::milliseconds>(_t2 - _t1).count();
                }
                catch (zpt::SyntaxErrorException& _e) {
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
                      std::chrono::duration_cast<std::chrono::milliseconds>(_t2 - _t1).count();
                }
                if (_parameters["--print"] != zpt::undefined) {
                    std::cout << (_parameters["--print"] == "pretty"
                                    ? static_cast<std::string>(zpt::pretty{ _ptr })
                                    : static_cast<std::string>(_ptr))
                              << std::endl
                              << std::endl
                              << std::flush;
                }
            }
            std::cout << "parsing total time: " << _parsing_duration << "ms" << std::endl
                      << std::flush;
            std::cout << "stringify total time: " << _stringify_duration << "ms" << std::endl
                      << std::flush;
        }
    }
    catch (zpt::SyntaxErrorException& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    catch (zpt::missed_expectation& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return -1;
    }
    return 0;
}
