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

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <sys/types.h>
#include <sys/ipc.h>
#include <zapata/log/log.h>
#include <zapata/text/manip.h>

#define OUTSIDE 0
#define HEADER 1
#define LIST 2
#define CODE 3
#define PARAGRAPH 4

auto
main(int _argc, char* _argv[]) -> int {
    for (int _idx = 1; _idx != _argc; _idx++) {
        std::string _input_file_name{ _argv[_idx] };
        std::ifstream _iss;
        _iss.open(_input_file_name);
        short _state = OUTSIDE;
        if (_iss.is_open()) {
            std::string _line;
            size_t _to_trim = 0;
            while (std::getline(_iss, _line)) {
                if (_state == OUTSIDE) {
                    if ((_to_trim = _line.find("/***")) != std::string::npos) {
                        _to_trim += 3;
                        _state = PARAGRAPH;
                    }
                }
                else if (_line.find("***/") != std::string::npos) {
                    _state = OUTSIDE;
                }
                else {
                    if (_line.length() >= _to_trim) {
                        _line.assign(_line.substr(_to_trim));
                        if (_line.length() != 0) {
                            if (_line[0] == '#') {
                                std::cout << std::endl << std::flush;
                                _state = HEADER;
                            }
                            else if (_line[0] == '-') {
                                if (_state != LIST) { std::cout << std::endl << std::flush; }
                                _state = LIST;
                            }
                            else if (_line[0] == '`') {
                                if (_state == CODE) { _state = PARAGRAPH; }
                                else {
                                    std::cout << std::endl << std::flush;
                                    _state = CODE;
                                }
                            }
                            else {
                                if (_state != CODE && _state != PARAGRAPH) {
                                    std::cout << std::endl << std::flush;
                                }
                                _state = PARAGRAPH;
                            }
                            std::cout << _line << std::endl << std::flush;
                        }
                        else if (_state == CODE || _state == PARAGRAPH) {
                            std::cout << std::endl << std::flush;
                        }
                    }
                    else if (_state == CODE || _state == PARAGRAPH) {
                        std::cout << std::endl << std::flush;
                    }
                }
            }
            _iss.close();
        }
    }
    return 0;
}
