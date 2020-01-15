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

#include <zapata/exceptions/ExpectationException.h>

#include <sstream>
#include <zapata/text/convert.h>

zpt::ExpectationException::ExpectationException(std::string _in,
                                                int _http_code,
                                                int _code,
                                                std::string _desc,
                                                int _line,
                                                std::string _file)
  : std::exception()
  , __what(_in)
  , __http_code(_http_code)
  , __code(_code)
  , __description(_desc)
  , __line(_line)
  , __file(_file) {
    zpt::replace(this->__what, "\"", "");
    zpt::replace(this->__description, "\"", "");
    this->__description.insert(0, "'");
    if (this->__line != 0) {
        this->__description.insert(this->__description.length(), "' failed on file ");
        this->__description.insert(this->__description.length(), this->__file);
        this->__description.insert(this->__description.length(), ", line ");
        zpt::tostr(this->__description, this->__line);
    }
}

zpt::ExpectationException::ExpectationException(std::string _in,
                                                int _http_code,
                                                int _code,
                                                std::string _desc,
                                                int _line,
                                                std::string _file,
                                                char** _backtrace,
                                                size_t _backtrace_size)
  : std::exception()
  , __what(_in)
  , __http_code(_http_code)
  , __code(_code)
  , __description(_desc)
  , __line(_line)
  , __file(_file) {
    zpt::replace(this->__what, "\"", "");
    zpt::replace(this->__description, "\"", "");
    this->__description.insert(0, "'");
    if (this->__line != 0) {
        this->__description.insert(this->__description.length(), "' failed on file ");
        this->__description.insert(this->__description.length(), this->__file);
        this->__description.insert(this->__description.length(), ", line ");
        zpt::tostr(this->__description, this->__line);
    }

    for (size_t _i = 0; _i != _backtrace_size; _i++) {
        this->__backtrace.insert(this->__backtrace.length(), "\t");
        this->__backtrace.insert(this->__backtrace.length(), std::string(_backtrace[_i]));
        this->__backtrace.insert(this->__backtrace.length(), "\n");
    }
    free(_backtrace);
}

zpt::ExpectationException::~ExpectationException() throw() {}

const char*
zpt::ExpectationException::what() {
    return this->__what.data();
}

const char*
zpt::ExpectationException::description() {
    return this->__description.data();
}

const char*
zpt::ExpectationException::backtrace() {
    return this->__backtrace.data();
}

int
zpt::ExpectationException::code() {
    return this->__code;
}

int
zpt::ExpectationException::status() {
    return this->__http_code;
}
