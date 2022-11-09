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

#include <stddef.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>
#include <zapata/log/log.h>
#include <zlib.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>

auto zpt::ltrim(std::string& _in_out) -> void {
    _in_out.erase(_in_out.begin(), std::find_if(_in_out.begin(), _in_out.end(), [](char c) {
                      return !std::isspace<char>(c, std::locale::classic());
                  }));
}

auto zpt::rtrim(std::string& _in_out) -> void {
    _in_out.erase(std::find_if(_in_out.rbegin(),
                               _in_out.rend(),
                               [](char c) { return !std::isspace<char>(c, std::locale::classic()); })
                    .base(),
                  _in_out.end());
}

auto zpt::trim(std::string& _in_out) -> void {
    zpt::ltrim(_in_out);
    zpt::rtrim(_in_out);
}

auto zpt::replace(std::string& str, std::string find, std::string replace) -> void {
    if (str.length() == 0) { return; }

    size_t start{ 0 };

    while ((start = str.find(find, start)) != std::string::npos) {
        str.replace(start, find.size(), replace);
        start += replace.length();
    }
}

auto zpt::normalize_path(std::string& _in_out, bool _with_trailing) -> void {
    if (_with_trailing) {
        if (_in_out[_in_out.length() - 1] != '/') { _in_out.insert(_in_out.length(), "/"); }
    }
    else {
        if (_in_out[_in_out.length() - 1] == '/') { _in_out.erase(_in_out.length() - 1, 1); }
    }
}

auto zpt::prettify_header_name(std::string& name) -> void {
    std::transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);

    std::stringstream iss;
    iss << name;

    char line[256] = { 0 };
    size_t pos{ 0 };
    while (iss.good()) {
        iss.getline(line, 256, '-');
        pos += iss.gcount();
        std::transform(name.begin() + pos, name.begin() + pos + 1, name.begin() + pos, ::toupper);
    }
}

auto zpt::r_ltrim(std::string const& _in_out) -> std::string {
    std::string _return{ _in_out.data() };
    _return.erase(_return.begin(),
                  std::find_if(_return.begin(), _return.end(), [](char c) { return std::isspace(c); }));
    return _return;
}

auto zpt::r_rtrim(std::string const& _in_out) -> std::string {
    std::string _return{ _in_out.data() };
    _return.erase(
      std::find_if(_return.rbegin(), _return.rend(), [](char c) { return std::isspace(c); }).base(),
      _return.end());
    return _return;
}

auto zpt::r_trim(std::string const& _in_out) -> std::string {
    std::string _return{ _in_out.data() };
    zpt::ltrim(_return);
    zpt::rtrim(_return);
    return _return;
}

auto zpt::r_replace(std::string str, std::string find, std::string replace) -> std::string {
    std::string _return{ str.data() };
    try {
        if (_return.length() == 0) { return _return; }

        size_t start{ 0 };

        while ((start = _return.find(find, start)) != std::string::npos) {
            _return.replace(start, find.size(), replace);
            start += replace.length();
        }
    }
    catch (std::exception const& _e) {
        std::cout << (_e.what()) << std::endl << std::flush;
    }
    return _return;
}

auto zpt::r_normalize_path(std::string const& _in_out, bool _with_trailing) -> std::string {
    std::string _return{ _in_out.data() };
    if (_with_trailing) {
        if (_return[_return.length() - 1] != '/') { _return.insert(_return.length(), "/"); }
    }
    else {
        if (_return[_return.length() - 1] == '/') { _return.erase(_return.length() - 1, 1); }
    }
    return _return;
}

auto zpt::r_prettify_header_name(std::string name) -> std::string {
    std::string _return{ name.data() };
    std::transform(_return.begin(), _return.begin() + 1, _return.begin(), ::toupper);

    std::stringstream iss;
    iss << _return;

    char line[256];
    size_t pos{ 0 };
    while (iss.good()) {
        iss.getline(line, 256, '-');
        pos += iss.gcount();
        std::transform(_return.begin() + pos, _return.begin() + pos + 1, _return.begin() + pos, ::toupper);
    }
    return _return;
}
