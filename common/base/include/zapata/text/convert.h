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
#include <cmath>
#include <errno.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory.h>
#include <mutex>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <wchar.h>
#include <wctype.h>
#include <zapata/text/manip.h>

#include <ossp/uuid++.hh>

namespace zpt {
extern uuid uuid_gen;
extern std::mutex uuid_mtx;

auto tostr(std::string& s, int i) -> void;
auto tostr(std::string& s, bool i) -> void;
auto tostr(std::string&, int, std::ios_base& (&)(std::ios_base&)) -> void;
#ifdef __LP64__
auto tostr(std::string& s, unsigned int i) -> void;
#endif
auto tostr(std::string& s, size_t i) -> void;
auto tostr(std::string& s, long i) -> void;
auto tostr(std::string& s, long long i) -> void;
auto tostr(std::string& s, float i, int precision = 3) -> void;
auto tostr(std::string& s, double i, int precision = 3) -> void;
auto tostr(std::string& s, char i) -> void;
auto tostr(std::string& s, time_t i, const char* f) -> void;

auto tostr(int i) -> std::string;
auto tostr(bool i) -> std::string;
auto tostr(int, std::ios_base& (&)(std::ios_base&)) -> std::string;
#ifdef __LP64__
auto tostr(unsigned int i) -> std::string;
#endif
auto tostr(size_t i) -> std::string;
auto tostr(long i) -> std::string;
auto tostr(long long i) -> std::string;
auto tostr(float i, int precision = 3) -> std::string;
auto tostr(double i, int precision = 3) -> std::string;
auto tostr(char i) -> std::string;
auto tostr(time_t i, const char* f) -> std::string;

auto fromstr(std::string s, int* i) -> void;
#ifdef __LP64__
auto fromstr(std::string s, unsigned int* i) -> void;
#endif
auto fromstr(std::string s, size_t* i) -> void;
auto fromstr(std::string s, long* i) -> void;
auto fromstr(std::string s, long long* i) -> void;
auto fromstr(std::string s, float* i) -> void;
auto fromstr(std::string s, double* i) -> void;
auto fromstr(std::string s, char* i) -> void;
auto fromstr(std::string s, bool* i) -> void;
auto fromstr(std::string s, time_t* i, const char* f, bool _no_timezone = false) -> void;
template<typename T>
auto fromstr(T _in) -> std::string;

const char encodeCharacterTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const signed char decodeCharacterTable[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
const char encodeCharacterTableUrl[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
const signed char decodeCharacterTableUrl[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63, -1, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
const std::wstring iso = L"\u00e1\u00e0\u00e2\u00e3\u00e4\u00e9\u00e8\u00ea\u1ebd\u00eb\u00ed\u00ec"
                         L"\u00ee\u0129\u00ef\u00f3"
                         L"\u00f2\u00f4\u00f5\u00f6\u00fa\u00f9\u00fb\u0169\u00fc\u00e7\u00c1\u00c0"
                         L"\u00c2\u00c3\u00c4\u00c9"
                         L"\u00c8\u00ca\u1ebc\u00cb\u00cd\u00cc\u00ce\u0128\u00cf\u00d3\u00d2\u00d4"
                         L"\u00d5\u00d6\u00da\u00d9"
                         L"\u00db\u0168\u00dc\u00c7";
const std::wstring plain = L"aaaaaeeeeeiiiiiooooouuuuucAAAAAEEEEEIIIIIOOOOOUUUUUC";

time_t timezone_offset();

auto get_backtrace() -> std::string;

namespace base64 {
void encode(std::string& _out);
void decode(std::string& _out);
void encode(std::istream& _in, std::ostream& _out);
void decode(std::istream& _in, std::ostream& _out);
void url_encode(std::string& _out);
void url_decode(std::string& _out);

auto r_encode(std::string const& _in) -> std::string;
auto r_decode(std::string const& _in) -> std::string;
auto r_url_encode(std::string const& _in) -> std::string;
auto r_url_decode(std::string const& _in) -> std::string;
} // namespace base64

using uchar = unsigned char;

namespace utf8 {
char* wstring_to_utf8(std::wstring ws);
wchar_t* utf8_to_wstring(std::string s);
int length(std::string s);
void encode(std::wstring s, std::string& _out, bool quote = true);
void encode(std::string& _out, bool quote = true);
void decode(std::string& _out);
} // namespace utf8

namespace unicode {
void escape(std::string& _out);
}

namespace quoted_printable {
auto encode(std::string const& _quote, std::string const& _charset, std::string& _out) -> void;
auto r_encode(std::string const& _quote, std::string const& _charset) -> std::string;
} // namespace quoted_printable

namespace url {
auto encode(std::wstring s, std::ostream& out) -> void;
auto encode(std::string& out) -> void;
auto decode(std::string& out) -> void;
auto r_encode(std::string const& _out) -> std::string;
auto r_decode(std::string const& _out) -> std::string;
} // namespace url

namespace ascii {
void encode(std::string& out, bool quote = true);
}

namespace generate {
auto key(std::string& _out, size_t _size = 24) -> void;
auto r_key(size_t _size) -> std::string;
auto r_key() -> std::string;
auto hash(std::string& _out) -> void;
auto r_hash() -> std::string;
auto uuid(std::string& _out) -> void;
auto r_uuid() -> std::string;
} // namespace generate

namespace test {
auto uuid(std::string const& _uuid) -> bool;
auto utf8(std::string const& _uri) -> bool;
auto ascii(std::string const& _ascii) -> bool;
auto token(std::string const& _token) -> bool;
auto uri(std::string _uri) -> bool;
auto email(std::string const& _email) -> bool;
auto phone(std::string const& _phone) -> bool;
auto regex(std::string const& _target, std::string const& _regex) -> bool;
auto timestamp(std::string const& _timestamp) -> bool;
} // namespace test
} // namespace zpt
