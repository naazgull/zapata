/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

#define BOM8A 0xEF
#define BOM8B 0xBB
#define BOM8C 0xBF

char* zpt::utf8::wstring_to_utf8(std::wstring ws) {
	std::string dest;
	dest.clear();
	for (size_t i = 0; i < ws.size(); i++) {
		wchar_t w = ws[i];
		if (w <= 0x7f)
		dest.push_back((char) w);
		else if (w <= 0x7ff) {
			dest.push_back(0xc0 | ((w >> 6) & 0x1f));
			dest.push_back(0x80 | (w & 0x3f));
		}
		else if (w <= 0xffff) {
			dest.push_back(0xe0 | ((w >> 12) & 0x0f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		}
		else if (w <= 0x10ffff) {
			dest.push_back(0xf0 | ((w >> 18) & 0x07));
			dest.push_back(0x80 | ((w >> 12) & 0x3f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		}
		else
		dest.push_back('?');
	}

	char* c = new char[dest.length() + 1];
	memset(c, 0, dest.length() + 1);
	memcpy(c, dest.c_str(), dest.length());
	return c;
}

wchar_t* zpt::utf8::utf8_to_wstring(std::string s) {
	long b = 0, c = 0;
	const char* _str = s.data();

	if ((uchar) _str[0] == BOM8A && (uchar) _str[1] == BOM8B && (uchar) _str[2] == BOM8C) {
		_str += 3;
	}

	for (const char *a = _str; *a; a++) {
		if (((uchar) *a) < 128 || (*a & 192) == 192) {
			c++;
		}
	}
	wchar_t *res = new wchar_t[c + 1];
	res[c] = 0;
	for (uchar *a = (uchar*) _str; *a; a++) {
		if (!(*a & 128)) {
			res[b] = *a;
		}
		else if ((*a & 192) == 128) {
			continue;
		}
		else if ((*a & 224) == 192) {
			res[b] = ((*a & 31) << 6) | (a[1] & 63);
		}
		else if ((*a & 240) == 224) {
			res[b] = ((*a & 15) << 12) | ((a[1] & 63) << 6) | (a[2] & 63);
		}
		else if ((*a & 248) == 240) {
			res[b] = '?';
		}
		b++;
	}
	return res;
}

int zpt::utf8::length(std::string s) {
	int size = 0;
	for (size_t i = 0; i != s.length(); i++) {
		if (((wchar_t) s[i]) < 0x80) {
			size++;
		}
		else if (((wchar_t) s[i]) < 0x800) {
			size += 2;
		}
		else {
			size += 3;
		}
	}
	return size;
}

void zpt::utf8::encode(std::wstring s, string& _out, bool quote) {
	ostringstream oss;

	for (size_t i = 0; i != s.length(); i++) {
		if (((int) s[i]) > 127) {
			oss << "\\u" <<  setfill('0') << setw(4) << hex << ((int) s.at(i));
		}
		else if (quote && (s[i] == '"')) {
			oss << "\\" << ((char) s.at(i));
		}
		else if (s[i] == '\n') {
			oss << "\\n";
		}
		else if (s[i] == '\r') {
			oss << "\\r";
		}
		else if (s[i] == '\f') {
			oss << "\\f";
		}
		else if (s[i] == '\t') {
			oss << "\\t";
		}
		else if (s[i] == '/') {
			oss << "\\/";
		}
		else if (quote && s[i] == '\\') {
			oss << "\\\\";
		}
		else if (((int) s[i]) <= 31) {
			oss << "";
		}
		else {
			oss << ((char) s.at(i));
		}
	}
	oss << flush;
	_out.assign(oss.str().data(), oss.str().length());
}

void zpt::utf8::encode(std::string& _out, bool quote) {
	long b = 0, c = 0;
	const char* _str = _out.data();

	if ((uchar) _str[0] == BOM8A && (uchar) _str[1] == BOM8B && (uchar) _str[2] == BOM8C) {
		_str += 3;
	}

	for (const char *a = _str; *a; a++) {
		if (((uchar) *a) < 128 || (*a & 192) == 192) {
			c++;
		}
	}
	wchar_t *res = new wchar_t[c + 1];
	res[c] = 0;
	for (uchar *a = (uchar*) _str; *a; a++) {
		if (!(*a & 128)) {
			res[b] = *a;
		}
		else if ((*a & 192) == 128) {
			continue;
		}
		else if ((*a & 224) == 192) {
			res[b] = ((*a & 31) << 6) | (a[1] & 63);
		}
		else if ((*a & 240) == 224) {
			res[b] = ((*a & 15) << 12) | ((a[1] & 63) << 6) | (a[2] & 63);
		}
		else if ((*a & 248) == 240) {
			res[b] = '?';
		}
		b++;
	}

	wstring ws;
	ws.assign(res, c + 1);
	zpt::utf8::encode(ws, _out, quote);
	delete[] res;
}

void zpt::utf8::decode(std::string& _out) {
	wostringstream oss;
	for (size_t i = 0; i != _out.length(); i++) {
		if (_out[i] == '\\' && _out[i + 1] == 'u') {
			std::stringstream ss;
			ss << _out[i + 2] << _out[i + 3] << _out[i + 4] << _out[i + 5];
			int c;
			ss >> hex >> c;
			oss << ((wchar_t) c);
			i += 5;
		}
		else {
			oss << ((wchar_t) _out[i]);
		}
	}
	oss << flush;

	char* c = zpt::utf8::wstring_to_utf8(oss.str());
	std::string os(c);
	_out.assign(c);

	delete[] c;
}

void zpt::unicode::escape(std::string& _out) {
	std::ostringstream _oss;
	for (const auto& c : _out) {
		switch (c) {
			case '"': _oss << "\\\""; break;
			case '\\': _oss << "\\\\"; break;
			case '\b': _oss << "\\b"; break;
			case '\f': _oss << "\\f"; break;
			case '\n': _oss << "\\n"; break;
			case '\r': _oss << "\\r"; break;
			case '\t': _oss << "\\t"; break;
			default:
				if (((uchar) c) > 127) {
					_oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int) ((uchar) c);
				}
				else {
					_oss << c;
				}
		}
	}
	_out.assign(_oss.str());
}
