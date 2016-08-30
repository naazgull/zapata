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
#include <dlfcn.h>

zpt::Addons::Addons(zpt::json _options) : zpt::EventEmitter( _options ) {
	if (_options["addons"]["modules"]->ok()) {
		for (auto _i : _options["addons"]["modules"]->obj()) {
			string _key = _i.first;
			zpt::JSONElement _value = _i.second;

			string _lib_file("lib");
			_lib_file.append((string) _value["lib"]);
			_lib_file.append(".so");

			if (_lib_file.length() > 6) {
				void *hndl = dlopen(_lib_file.data(), RTLD_NOW);
				if (hndl == nullptr) {
					zlog(string(dlerror()), zpt::error);
				}
				else {
					void (*_populate)(zpt::ev::emitter);
					_populate = (void (*)(zpt::ev::emitter)) dlsym(hndl, "plug");
					_populate(this->self());
				}
			}
		}
	}
}

zpt::Addons::~Addons() {
	for (auto _i : this->__resources) {
		regfree(_i.second.first);
		delete _i.second.first;
	}
}

std::string zpt::Addons::on(string _regex, zpt::ev::Handler _handler) {
	return this->on(zpt::ev::Post, _regex, _handler);
}

std::string zpt::Addons::on(zpt::ev::performative _event, string _regex,  zpt::ev::Handler _handler) {
	regex_t * _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	std::vector< zpt::ev::Handler> _handlers;
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);
	_handlers.push_back(_handler);
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);
	_handlers.push_back(nullptr);

	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	this->__resources.insert(make_pair(_uuid.string(), pair<regex_t*, vector< zpt::ev::Handler> >(_url_pattern, _handlers)));
	return _uuid.string();
}

std::string zpt::Addons::on(string _regex,  zpt::ev::Handler _handler_set[7]) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	vector< zpt::ev::Handler> _handlers;
	_handlers.push_back(_handler_set[zpt::ev::Get]);
	_handlers.push_back(_handler_set[zpt::ev::Put]);
	_handlers.push_back(_handler_set[zpt::ev::Post]);
	_handlers.push_back(_handler_set[zpt::ev::Delete]);
	_handlers.push_back(_handler_set[zpt::ev::Head]);
	_handlers.push_back(_handler_set[zpt::ev::Options]);
	_handlers.push_back(_handler_set[zpt::ev::Patch]);

	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	this->__resources.insert(make_pair(_uuid.string(), pair<regex_t*, vector< zpt::ev::Handler> >(_url_pattern, _handlers)));
	return _uuid.string();
}

std::string zpt::Addons::on(string _regex,  std::map< zpt::ev::performative, zpt::ev::Handler > _handler_set) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}
	
	vector< zpt::ev::Handler> _handlers;
	_handlers.push_back(_handler_set[zpt::ev::Get]);
	_handlers.push_back(_handler_set[zpt::ev::Put]);
	_handlers.push_back(_handler_set[zpt::ev::Post]);
	_handlers.push_back(_handler_set[zpt::ev::Delete]);
	_handlers.push_back(_handler_set[zpt::ev::Head]);
	_handlers.push_back(_handler_set[zpt::ev::Options]);
	_handlers.push_back(_handler_set[zpt::ev::Patch]);
	
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	this->__resources.insert(make_pair(_uuid.string(), pair<regex_t*, vector< zpt::ev::Handler> >(_url_pattern, _handlers)));
	return _uuid.string();
}

void zpt::Addons::off(zpt::ev::performative _event, std::string _callback_id) {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		_found->second.second[_event] = nullptr;
	}
}

void zpt::Addons::off(std::string _callback_id) {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		this->__resources.erase(_callback_id);
	}
}

zpt::json zpt::Addons::trigger(std::string _regex, zpt::json _payload) {
	return this->trigger(zpt::ev::Post, _regex, _payload);
}

zpt::json zpt::Addons::trigger(zpt::ev::performative _method, std::string _resource, zpt::json _payload) {
	zpt::json _return = zpt::mkarr();

	for (auto _i : this->__resources) {
		if (regexec(_i.second.first, _resource.c_str(), (size_t) (0), nullptr, 0) == 0) {
			try {
				zpt::json _result = _i.second.second[_method](_method, _resource, _payload, this->self());
				if (_result->ok()) {
					_return << _result;
				}
			}
			catch (zpt::AssertionException& _e) {
				return zpt::json(
					{
						"status", _e.status(),
						"error",  true,
						"assertion_failed", _e.description(),
						"message", _e.what(),
						"code", _e.code()
					}
				);
			}
		}
	}
	return _return;
}

zpt::json zpt::Addons::route(zpt::ev::performative _method, std::string _resource, zpt::json _payload) {
	return zpt::undefined;
}
