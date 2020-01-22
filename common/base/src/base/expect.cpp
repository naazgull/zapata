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
#include <zapata/base/expect.h>

namespace zpt {

std::string* tz = nullptr;
}

std::string
zpt::get_tz() {
    if (zpt::tz == nullptr) {
        zpt::tz = new std::string();
        std::ifstream _tzf;
        _tzf.open("/etc/timezone");
        if (_tzf.is_open()) {
            _tzf >> (*zpt::tz);
            _tzf.close();
        }
        else {
            zpt::tz->assign("UTC");
        }
    }
    return *zpt::tz;
}

zpt::tm_ptr
zpt::get_time(time_t _t) {
    std::tm* _tm = new std::tm();
    std::memcpy(_tm, localtime(&_t), sizeof(std::tm));
    return zpt::tm_ptr(_tm);
}
