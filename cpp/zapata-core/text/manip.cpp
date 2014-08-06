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

#include <stddef.h>
#include <text/convert.h>
#include <text/manip.h>
#include <zconf.h>
#include <zlib.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>

using namespace std;
using namespace __gnu_cxx;

void zapata::ltrim(std::string &_in_out) {
        _in_out.erase(_in_out.begin(), std::find_if(_in_out.begin(), _in_out.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void zapata::rtrim(std::string &_in_out) {
        _in_out.erase(std::find_if(_in_out.rbegin(), _in_out.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _in_out.end());
}

void zapata::trim(std::string &_in_out) {
        zapata::ltrim(_in_out);
        zapata::rtrim(_in_out);
}

void zapata::replace(string& str, string find, string replace) {
	if (str.length() == 0) {
		return;
	}

	size_t start = 0;

	while ((start = str.find(find, start)) != string::npos) {
		str.replace(start, find.size(), replace);
		start += replace.length();
	}
}

void zapata::normalize_path(string& _in_out, bool _with_trailing) {
	if (_with_trailing) {
		if (_in_out[_in_out.length() - 1] != '/') {
			_in_out.insert(_in_out.length(), "/");
		}
	}
	else {
		if (_in_out[_in_out.length() - 1] == '/') {
			_in_out.erase(_in_out.length() - 1, 1);
		}
	}
}


void zapata::cipher(string _in, string _key, string& _out) {
	unsigned int _ikey = _key.length(), iIn = _in.length(), x = 0;
	string _s_encrypted(_in);

	for (unsigned int i = 0; i < iIn; i++) {
		_s_encrypted[i] = _in[i] ^ (_key[x] & 10);
		if (++x == _ikey) {
			x = 0;
		}
	}
	_out.assign(_s_encrypted);
}

void zapata::decipher(string _in, string _key, string& _out) {
	zapata::cipher(_in, _key, _out);
}

void zapata::encrypt(string& _out, string _in, string _key) {
	Bytef* src = new Bytef[_in.length()];
	size_t destLen = (size_t) (_in.length() * 1.1 + 12);
	Bytef* dest = new Bytef[destLen];

	for (size_t i = 0; i != _in.length(); i++) {
		src[i]  = (Bytef) _in[i];
	}

	compress(dest, &destLen, src, _in.length());

	ostringstream cos;
	for (size_t i = 0; i != destLen; i++) {
		cos << dest[i];
	}
	cos << flush;

	string _encrypted;
	zapata::cipher(cos.str(), _key, _encrypted);

	zapata::tostr(_out, _in.length());
	_out.insert(_out.length(), ".");
	zapata::base64url_encode(_encrypted);

	delete[] src;
	delete[] dest;

	_out.assign(_encrypted);
}

void zapata::decrypt(string& _out, string _in, string _key) {
	int _idx = _in.find('.');
	string _length(_in.substr(0, _idx));
	size_t _size = 0;
	zapata::fromstr(_length, &_size);
	if (_size == 0) {
		return;
	}

	string _encrypted = _in.substr(_idx + 1);
	zapata::base64url_decode(_encrypted);

	string _decrypted;
	zapata::decipher(_encrypted, _key,  _decrypted);

	Bytef* src = new Bytef[_decrypted.length()];
	size_t destLen = _size;
	Bytef* dest = new Bytef[destLen];

	for (size_t i = 0; i != _decrypted.length(); i++) {
		src[i]  = (Bytef) _decrypted[i];
	}

	uncompress(dest, &destLen, src, _decrypted.length());

	ostringstream cos;
	for (size_t i = 0; i != destLen; i++) {
		cos << dest[i];
	}
	cos << flush;

	delete[] src;
	delete[] dest;

	_out.assign(cos.str());
}
