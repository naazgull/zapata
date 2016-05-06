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

zapata::Addons::Addons(zapata::JSONObj& _options) : zapata::EventEmitter( _options ) {
}

zapata::Addons::~Addons() {
	for (auto _i : this->__resources) {
		regfree(_i.first);
		delete _i.first;
	}
}

void zapata::Addons::on(string _regex, zapata::ev::Handler _handler) {
	this->on(zapata::ev::Post, _regex, _handler);
}

zapata::JSONPtr zapata::Addons::trigger(std::string _regex, zapata::JSONPtr _payload) {
	return this->trigger(zapata::ev::Post, _regex, _payload);
}

void zapata::Addons::on(zapata::ev::Performative _event, string _regex,  zapata::ev::Handler _handler) {
	regex_t * _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	std::vector< zapata::ev::Handler> _handlers;
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);
	_handlers.push_back(_handler);
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);

	this->__resources.push_back(pair<regex_t*, vector< zapata::ev::Handler> >(_url_pattern, _handlers));
}

void zapata::Addons::on(string _regex,  zapata::ev::Handler _handler_set[7]) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	vector< zapata::ev::Handler> _handlers;
	_handlers.push_back(_handler_set[zapata::ev::Get]);
	_handlers.push_back(_handler_set[zapata::ev::Put]);
	_handlers.push_back(_handler_set[zapata::ev::Post]);
	_handlers.push_back(_handler_set[zapata::ev::Delete]);
	_handlers.push_back(_handler_set[zapata::ev::Head]);
	_handlers.push_back(_handler_set[zapata::ev::Options]);
	_handlers.push_back(_handler_set[zapata::ev::Patch]);

	this->__resources.push_back(pair<regex_t*, vector< zapata::ev::Handler> >(_url_pattern, _handlers));
}

zapata::JSONPtr zapata::Addons::trigger(zapata::ev::Performative _method, std::string _resource, zapata::JSONPtr _payload) {
	zapata::JSONPtr _return = zapata::make_arr();

	for (auto _i : this->__resources) {
		if (regexec(_i.first, _resource.c_str(), (size_t) (0), nullptr, 0) == 0) {
			try {
				zapata::JSONPtr _result = _i.second[_method](_method, _resource, _payload, this->self());
				if (_result->ok()) {
					_return << _result;
				}
			}
			catch (zapata::AssertionException& _e) {
				return zapata::make_ptr(JSON(
					"status" << _e.status()
					<< "error" <<  true
					<< "assertion_failed" << _e.description()
					<< "message" << _e.what()
					<< "code" << _e.code()
				));
			}
		}
	}
	return _return;
}

extern "C" int zapata_addons() {
	return 1;
}