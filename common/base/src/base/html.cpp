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

#include <zapata/text/html.h>

#include <iomanip>
#include <unistd.h>
#include <zapata/text/manip.h>

auto
zpt::html::entities_encode(std::wstring s, std::ostream& out, bool quote, bool tags) -> void {
    for (size_t i = 0; i != s.length(); i++) {
        if (((unsigned char)s[i]) > 127) { out << "&#" << std::dec << ((int)s.at(i)) << ";"; }
        else if (s[i] == '"' && quote) { out << "&quot;"; }
        else if (s[i] == '<' && tags) { out << "&lt;"; }
        else if (s[i] == '>' && tags) { out << "&gt;"; }
        else if (s[i] == '&') { out << "&amp;"; }
        else { out << ((char)s.at(i)); }
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
        else { oss << ((wchar_t)_out[i]); }
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
