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

#include <zapata/text/convert.h>

#include <unistd.h>
#include <iomanip>

auto
zpt::quoted_printable::encode(std::string const& _quote,
                              std::string const& _charset,
                              std::string& _out) -> void {
    std::ostringstream _oss;
    _oss << "=?" << _charset << "?Q?" << std::flush;
    std::string _s(_quote);
    for (size_t _i = 0; _i != _s.length(); _i++) {
        if (((unsigned char)_s[_i]) > 127) {
            _oss << "=" << std::uppercase << std::hex << ((int)((unsigned char)_s[_i]));
        }
        else {
            _oss << _s[_i];
        }
    }
    _oss << "?=" << std::flush;
    _out.insert(_out.length(), _oss.str());
}

auto
zpt::quoted_printable::r_encode(std::string const& _quote, std::string const& _charset)
  -> std::string {
    std::string _out;
    zpt::quoted_printable::encode(_quote, _charset, _out);
    return _out;
}

auto
zpt::url::encode(std::string& _out) -> void {
    const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
    const unsigned char* pSrc = (const unsigned char*)_out.c_str();
    const int SRC_LEN = _out.length();
    unsigned char* const pStart = new unsigned char[SRC_LEN * 3];
    unsigned char* pEnd = pStart;
    const unsigned char* const SRC_END = pSrc + SRC_LEN;

    for (; pSrc < SRC_END; ++pSrc) {
        if ((*pSrc > 127) || (*pSrc == '%') || (*pSrc == ' ') || (*pSrc == '&') || (*pSrc == '+') ||
            (*pSrc == '?') || (*pSrc == '#') || (*pSrc == '=') || (*pSrc == '/') ||
            (*pSrc == ':')) {
            *pEnd++ = '%';
            *pEnd++ = DEC2HEX[*pSrc >> 4];
            *pEnd++ = DEC2HEX[*pSrc & 0x0F];
        }
        else {
            *pEnd++ = *pSrc;
        }
    }

    std::string sResult((char*)pStart, (char*)pEnd);
    delete[] pStart;
    _out.assign(sResult);
}

auto
zpt::url::decode(std::string& _out) -> void {
    zpt::replace(_out, "+", "%20");

    const unsigned char* pSrc = (const unsigned char*)_out.c_str();
    const int SRC_LEN = _out.length();
    const unsigned char* const SRC_END = pSrc + SRC_LEN;
    const unsigned char* const SRC_LAST_DEC = SRC_END - 2;

    char* const pStart = new char[SRC_LEN];
    char* pEnd = pStart;

    while (pSrc < SRC_LAST_DEC) {
        if (*pSrc == '%') {
            std::stringstream ss;
            ss << *(pSrc + 1) << *(pSrc + 2);
            int c;
            ss >> std::hex >> c;
            *pEnd++ = (char)c;
            pSrc += 3;
            continue;
        }

        *pEnd++ = *pSrc++;
    }

    // the last 2- chars
    while (pSrc < SRC_END) { *pEnd++ = *pSrc++; }
    std::string sResult(pStart, pEnd);
    delete[] pStart;
    _out.assign(sResult);
}

auto
zpt::url::r_encode(std::string const& _out) -> std::string {
    std::string _return(_out.data());
    zpt::url::encode(_return);
    return _return;
}

auto
zpt::url::r_decode(std::string const& _out) -> std::string {
    std::string _return(_out.data());
    zpt::url::decode(_return);
    return _return;
}
