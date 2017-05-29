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
#include <sys/time.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {
	uuid uuid_gen;
}

auto zpt::ascii::encode(std::string& _out, bool quote) -> void {
	wchar_t* wc = zpt::utf8::utf8_to_wstring(_out);
	wstring ws(wc);

	for (size_t i = 0; i != iso.length(); i++) {
		std::replace(ws.begin(), ws.end(), iso[i], plain[i]);
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

auto zpt::generate::key(std::string& _out, size_t _size) -> void {
	static string charset = "abcdefghijklmnopqrstuvwxyz0123456789";
	timeval _tv;

	for (size_t _idx = 0; _idx != _size; _idx++) {
		gettimeofday (&_tv, nullptr);
		srand(_tv.tv_usec);
		_out.append(1, charset[rand() % charset.length()]);
	}
}

auto zpt::generate::r_key(size_t _size) -> std::string{
	std::string _out;
	zpt::generate::key(_out, _size);
	return _out;
}

auto zpt::generate::r_key() -> std::string {
	std::string _out;
	zpt::generate::key(_out);
	return _out;
}

auto zpt::generate::hash(std::string& _out) -> void {
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

auto zpt::generate::r_hash() -> std::string {
	std::string _out;
	zpt::generate::hash(_out);
	return _out;
}

auto zpt::generate::uuid(std::string& _out) -> void {
	zpt::uuid_gen.make(UUID_MAKE_V1);
	_out.append(zpt::uuid_gen.string());
}

auto zpt::generate::r_uuid() -> std::string {
	zpt::uuid_gen.make(UUID_MAKE_V1);
	return zpt::uuid_gen.string();
}

auto zpt::test::uuid(std::string _uuid) -> bool {
	static const std::regex _uuid_rgx(
		"^([a-fA-f0-9]{8})-"
		"([a-fA-f0-9]{4})-"
		"([a-fA-f0-9]{4})-"
		"([a-fA-f0-9]{4})-"
		"([a-fA-f0-9]{12})$"
	);
	return std::regex_match(_uuid, _uuid_rgx);
}

auto zpt::test::utf8(std::string _uri) -> bool {
	return true;
}

auto zpt::test::ascii(std::string _ascii) -> bool {
	static const std::regex _ascii_rgx(
		"^([a-zA-Z0-9_@:;./+*|-]+)$"
	);
	return std::regex_match(_ascii, _ascii_rgx);
}

auto zpt::test::token(std::string _token) -> bool {
	return true;
}

auto zpt::test::uri(std::string _uri) -> bool {
	if (_uri.find(":") >= _uri.find("/")) {
		_uri = std::string("zpt:") + _uri; 
	}
	static const std::regex _uri_rgx(
		"([@>]{0,1})([a-zA-Z][a-zA-Z0-9+.-]+):"  // scheme:
		"([^?#]*)"                    // authority and path
		"(?:\\?([^#]*))?"             // ?query
		"(?:#(.*))?"		      // #fragment
	);
	return std::regex_match(_uri, _uri_rgx);
}

auto zpt::test::email(std::string _email) -> bool {
	static const std::regex _email_rgx(
		"([a-zA-Z0-9])([a-zA-Z0-9+._-]*)@"
		"([a-zA-Z0-9])([a-zA-Z0-9+._-]*)"
	);
	return std::regex_match(_email, _email_rgx);
}

auto zpt::test::phone(std::string _phone) -> bool {
	static const std::regex _phone_rgx(
		"\\(([0-9]){1,3}\\)([ ]*)"
		"([0-9]){3,12}"
	);
	return std::regex_match(_phone, _phone_rgx);
}

auto zpt::test::regex(std::string _target, std::string _regex) -> bool {
	std::regex _rgx(_regex);
	return std::regex_match(_target, _rgx);
}
