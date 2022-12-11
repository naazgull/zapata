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

#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdio.h>
#include <string.h>

namespace zpt {

void ltrim(std::string& _in_out);
void rtrim(std::string& _in_out);
void trim(std::string& _in_out);
auto replace(std::string& str, std::string find, std::string replace) -> void;

void normalize_path(std::string& _in_out, bool _with_trailing);

void prettify_header_name(std::string& name);

std::string r_ltrim(std::string const& _in_out);
std::string r_rtrim(std::string const& _in_out);
std::string r_trim(std::string const& _in_out);
std::string r_replace(std::string str, std::string find, std::string replace);

std::string r_normalize_path(std::string const& _in_out, bool _with_trailing);

std::string r_prettify_header_name(std::string name);

template<typename... Args>
auto format(std::string _to_format, Args... _params) -> std::string;

} // namespace zpt

namespace {
#pragma GCC diagnostic ignored "-Wunused-function"
auto ___format(std::istringstream& _in, std::ostringstream& _out) -> void {
    char _c{ '\0' };
    do {
        _in >> std::noskipws >> _c;
        if (!_in.eof()) { _out << _c; }
    } while (_in.good());
}
#pragma GCC diagnostic pop
template<typename P, typename... Args>
auto ___format(std::istringstream& _in, std::ostringstream& _out, P _param, Args... _rest) -> void {
    char _c{ '\0' };
    do {
        _in >> std::noskipws >> _c;
        if (!_in.eof()) {
            if (_c == '{') {
                _in >> std::noskipws >> _c;
                if (!_in.eof()) {
                    if (_c == '}') {
                        _out << _param;
                        ::___format(_in, _out, _rest...);
                        return;
                    }
                    else { _out << '{' << _c; }
                }
                else { _out << '{'; }
            }
            else { _out << _c; }
        }
    } while (_in.good());
}
} // namespace

template<typename... Args>
auto zpt::format(std::string _to_format, Args... _params) -> std::string {
    std::istringstream _iss;
    _iss.str(_to_format);
    std::ostringstream _oss;
    ::___format(_iss, _oss, _params...);
    _oss << std::flush;
    return _oss.str();
}
