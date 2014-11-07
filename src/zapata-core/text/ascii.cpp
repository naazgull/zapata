/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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
#include <sys/time.h>

using namespace std;
using namespace __gnu_cxx;

void zapata::ascii_encode(string& _out, bool quote) {
	wchar_t* wc = zapata::utf8_to_wstring(_out);
	wstring ws(wc);

	for (size_t i = 0; i != iso.length(); i++) {
		std::replace(ws.begin(), ws.end(), iso[i], ascii[i]);
	}

	ostringstream _oss;
	delete[] wc;
	for (size_t i = 0; i != ws.length(); i++) {
		if (((int) ws[i]) <= 127) {
			_oss << ((char) ws[i]) << flush;
		}
		else {
			_oss << " " << flush;
		}
	}
	_out.assign(_oss.str());
}

void zapata::generate_key(string& _out) {
	timeval _tv;
	gettimeofday (&_tv, nullptr);

	static string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string code1;
	code1.resize(2);
	srand(_tv.tv_usec);
	code1[0] = charset[rand() % charset.length()];
	code1[1] = charset[rand() % charset.length()];
	string code2;
	code2.resize(2);
	srand(_tv.tv_usec * 2);
	code2[0] = charset[rand() % charset.length()];
	code2[1] = charset[rand() % charset.length()];
	string code3;
	code3.resize(2);
	srand(_tv.tv_usec * 3);
	code3[0] = charset[rand() % charset.length()];
	code3[1] = charset[rand() % charset.length()];

	string _ts;
	zapata::tostr(_ts, time(nullptr), "%y%m%d");
	string _usec;
	zapata::tostr(_usec, _tv.tv_usec);

	_out.insert(_out.length(), code3);
	_out.insert(_out.length(), _ts);
	_out.insert(_out.length(), code1);
	_out.insert(_out.length(), _usec);
	_out.insert(_out.length(), code2);
}

void zapata::generate_hash(string& _out) {
	static string _charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	string _randompass;
	_randompass.resize(45);
	timeval _tv;
	gettimeofday (&_tv, nullptr);

	srand(_tv.tv_usec);
	for (int i = 0; i < 45; i++) {
		if (i % 10 == 0) {
			srand(_tv.tv_usec * i);
		}
		_randompass[i] = _charset[rand() % _charset.length()];
	};
	_out.insert(_out.length(), _randompass);
}
