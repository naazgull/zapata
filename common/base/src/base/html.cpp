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

#include <zapata/text/html.h>

#include <iomanip>
#include <unistd.h>
#include <zapata/text/manip.h>

auto
zpt::html::entities_encode(std::wstring s, std::ostream& out, bool quote, bool tags) -> void {
    for (size_t i = 0; i != s.length(); i++) {
        if (((unsigned char)s[i]) > 127) { out << "&#" << std::dec << ((int)s.at(i)) << ";"; }
        else if (s[i] == '"' && quote) {
            out << "&quot;";
        }
        else if (s[i] == '<' && tags) {
            out << "&lt;";
        }
        else if (s[i] == '>' && tags) {
            out << "&gt;";
        }
        else if (s[i] == '&') {
            out << "&amp;";
        }
        else {
            out << ((char)s.at(i));
        }
    }
    out << std::flush;
}

auto
zpt::html::entities_encode(std::string& _out, bool quote, bool tags) -> void {
    auto wc = zpt::utf8::utf8_to_wstring(_out);
    std::wstring ws{ wc };
    std::ostringstream out;
    zpt::html::entities_encode(ws, out, quote, tags);
    delete[] wc;
    _out.assign(out.str());
}

auto
zpt::html::entities_decode(std::string& _out) -> void {
    std::wostringstream oss;
    for (size_t i = 0; i != _out.length(); i++) {
        if (_out[i] == '&' && _out[i + 1] == '#') {
            std::stringstream ss;
            int j = i + 2;
            while (_out[j] != ';') {
                ss << _out[j];
                j++;
            }
            int c;
            ss >> c;
            oss << ((wchar_t)c);
            i = j;
        }
        else {
            oss << ((wchar_t)_out[i]);
        }
    }
    oss << std::flush;

    auto c = zpt::utf8::wstring_to_utf8(oss.str());
    _out.assign(std::string{ c });
    delete[] c;
}

auto
zpt::html::content_boundary(std::string& _in, std::string& _out) -> void {
    auto _idx = _in.find("boundary=");
    if (_idx != std::string::npos) { _out.assign(_in.substr(_idx + 9)); }
}
