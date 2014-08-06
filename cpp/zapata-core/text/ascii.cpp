/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <text/convert.h>

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
	gettimeofday (&_tv, NULL);

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
	zapata::tostr(_ts, time(NULL), "%y%m%d");
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
	gettimeofday (&_tv, NULL);

	srand(_tv.tv_usec);
	for (int i = 0; i < 45; i++) {
		if (i % 10 == 0) {
			srand(_tv.tv_usec * i);
		}
		_randompass[i] = _charset[rand() % _charset.length()];
	};
	_out.insert(_out.length(), _randompass);
}
