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

#include <zapata/addons/Addons.h>

zapata::Addons::Addons(zapata::JSONObj& _options) : __options( _options ), __self(this) {
}

zapata::Addons::~Addons() {
	for (auto _i : this->__resources) {
		regfree(_i.first);
		delete _i.first;
	}
}

zapata::JSONObj& zapata::Addons::options() {
	return this->__options;
}

void zapata::Addons::on(string _regex, zapata::AddonHandler _handler) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	this->__resources.push_back(pair<regex_t*, zapata::AddonHandler >(_url_pattern, _handler));
}

zapata::JSONPtr zapata::Addons::trigger(std::string _regex, zapata::JSONPtr _payload) {
	return this->process(_regex, _payload);
}

void zapata::Addons::add_kb(std::string _name, zapata::KBPtr _kb) {
	this->__kb.insert(make_pair(_name, _kb));
}

zapata::KBPtr zapata::Addons::get_kb(std::string _name) {
	auto _found = this->__kb.find(_name);
	if (_found == this->__kb.end()) {
		return zapata::KBPtr(nullptr);
	}
	return _found->second;
}

zapata::JSONPtr zapata::Addons::process(std::string _regex, zapata::JSONPtr _payload) {	
	zapata::JSONPtr _ret = zapata::make_arr();
	for (auto _i : this->__resources) {
		if (regexec(_i.first, _regex.c_str(), (size_t) (0), nullptr, 0) == 0) {
			try {
				zapata::JSONPtr _result = _i.second(_payload, make_ptr(this->__options), this->__self);
				if (_result->ok()) {
					_ret << _result;
				}
			}
			catch (zapata::AssertionException& _e) {
			}
		}
	}
}

extern "C" int zapata_addons() {
	return 1;
}