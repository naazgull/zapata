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

#include <stddef.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>
#include <zconf.h>
#include <zlib.h>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

void zpt::ltrim(std::string &_in_out) {
        _in_out.erase(_in_out.begin(), std::find_if(_in_out.begin(), _in_out.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void zpt::rtrim(std::string &_in_out) {
        _in_out.erase(std::find_if(_in_out.rbegin(), _in_out.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _in_out.end());
}

void zpt::trim(std::string &_in_out) {
        zpt::ltrim(_in_out);
        zpt::rtrim(_in_out);
}

void zpt::replace(string& str, string find, string replace) {
	if (str.length() == 0) {
		return;
	}

	size_t start = 0;

	while ((start = str.find(find, start)) != string::npos) {
		str.replace(start, find.size(), replace);
		start += replace.length();
	}
}

void zpt::normalize_path(string& _in_out, bool _with_trailing) {
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


void zpt::cipher(string _in, string _key, string& _out) {
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

void zpt::decipher(string _in, string _key, string& _out) {
	zpt::cipher(_in, _key, _out);
}

void zpt::encrypt(string& _out, string _in, string _key) {
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
	zpt::cipher(cos.str(), _key, _encrypted);

	zpt::tostr(_out, _in.length());
	_out.insert(_out.length(), ".");
	zpt::base64::encode(_encrypted);

	delete[] src;
	delete[] dest;

	_out.assign(_encrypted);
}

void zpt::decrypt(string& _out, string _in, string _key) {
	int _idx = _in.find('.');
	string _length(_in.substr(0, _idx));
	size_t _size = 0;
	zpt::fromstr(_length, &_size);
	if (_size == 0) {
		return;
	}

	string _encrypted = _in.substr(_idx + 1);
	zpt::base64::decode(_encrypted);

	string _decrypted;
	zpt::decipher(_encrypted, _key,  _decrypted);

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

void zpt::prettify_header_name(string& name) {
	std::transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);

	stringstream iss;
	iss << name;

	char line[256];
	size_t pos = 0;
	while (iss.good()) {
		iss.getline(line, 256, '-');
		pos += iss.gcount();
		std::transform(name.begin() + pos, name.begin() + pos + 1, name.begin() + pos, ::toupper);
	}
}