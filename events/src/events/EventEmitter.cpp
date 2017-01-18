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

#include <zapata/events/EventEmitter.h>

#define ACCESS_CONTROL_HEADERS "X-Cid,X-Status,X-No-Redirection,X-Redirect-To,Authorization,Accept,Accept-Language,Cache-Control,Connection,Content-Length,Content-Type,Cookie,Date,Expires,Location,Origin,Server,X-Requested-With,X-Replied-With,Pragma,Cache-Control,E-Tag"

namespace zpt {
	namespace ev {
		std::string* __default_authorization = nullptr;
	}
}

zpt::EventEmitter::EventEmitter() : __self( this ), __mutant() {
}

zpt::EventEmitter::EventEmitter(zpt::json _options) :  __options( _options), __self( this ) {
}

zpt::EventEmitter::~EventEmitter() {
}

auto zpt::EventEmitter::options() -> zpt::json {
	return this->__options;
}
					 
auto zpt::EventEmitter::self() -> zpt::ev::emitter {
	return this->__self;
}

auto zpt::EventEmitter::mutations() -> zpt::mutation::emitter {
	return this->__mutant;
}

auto zpt::EventEmitter::connector(std::string _name, zpt::connector _connector) -> void {
	_connector->events(this->__self);
	this->__mutant->connector(_name, _connector);
}

auto zpt::EventEmitter::connector(std::string _name) -> zpt::connector {
	return this->__mutant->connector(_name);
}

auto zpt::ev::split(std::string _url, zpt::json _orphans) -> zpt::json {
	zpt::JSONObj _ret;
	zpt::json _splited = zpt::split(_url, "/");

	size_t _idx = 0;
	for (auto _label : _orphans->arr()) {
		_ret << (string) _label << (string) _splited[_idx];
		_idx++;
	}
	for (; _idx < _splited->arr()->size(); _idx++) {
		_ret << (string) _splited[_idx] << (string) _splited[_idx + 1];
		_idx++;
	}
	return mkptr(_ret);
}

auto zpt::ev::join(zpt::json _info, size_t _orphans) -> std::string {
	std::string _ret;
	size_t _idx = 0;

	for (auto _field : _info->obj()) {
		if (_idx < _orphans) {
			_ret += string("/") + _field.second->str();
		}
		else {
			_ret += string("/") + _field.first + string("/") + _field.second->str();
		}
	}
	return _ret;
}

auto zpt::ev::set_default_authorization(std::string _default_authorization) -> void {
	if (zpt::ev::__default_authorization != nullptr) {
		delete zpt::ev::__default_authorization;
	}
	zpt::ev::__default_authorization = new std::string(_default_authorization.data());
}

auto zpt::ev::get_default_authorization() -> std::string{
	if (zpt::ev::__default_authorization != nullptr) {
		return std::string(zpt::ev::__default_authorization->data());
	}
	return "";
}

auto zpt::ev::init_request(std::string _cid) -> zpt::json {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);
	
	zpt::json _return(
		{
			"Accept", "application/json",
			"Accept-Charset", "utf-8",
			"Cache-Control", "no-cache",
			"Date", string(_buffer_date),
			"Expires", string(_buffer_expires),
			"User-Agent", "zapata RESTful server"
		}
	);
	if (_cid != "") {
		_return << "X-Cid" << _cid;
	}
	else {
		uuid _uuid;
		_uuid.make(UUID_MAKE_V1);
		_return << "X-Cid" << _uuid.string();
	}
	if (zpt::ev::__default_authorization != nullptr) {
		_return << "Authorization" << std::string(zpt::ev::__default_authorization->data());
	}
	return _return;
}

auto zpt::ev::init_reply(std::string _uuid) -> zpt::json {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	zpt::json _return = {
		"Server", "zapata RESTful server",
		"Cache-Control", "max-age=3600",
		"Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag",
		"Date", std::string(_buffer_date),
		"Expires", std::string(_buffer_expires)
	};
	if (_uuid != "") {
		_return << "X-Cid" << _uuid;
	}
	return _return;
}

zpt::EventListener::EventListener(std::string _regex) : __regex(_regex) {
}

zpt::EventListener::~EventListener() {
}

auto zpt::EventListener::regex() -> std::string {
	return this->__regex;
}

auto zpt::EventListener::get(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::put(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::post(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::del(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::head(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::options(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	if (_envelope["headers"]["Origin"]->ok()) {
		return {
			"status", 413,
			"headers", zpt::ev::init_reply(((string) _envelope["headers"]["X-Cid"]))
		};
	}
	string _origin = _envelope["headers"]["Origin"];
	return {
		"status", 200,
		"headers", (zpt::ev::init_reply(((string) _envelope["headers"]["X-Cid"])) + zpt::json(
				{
					"Access-Control-Allow-Origin", _envelope["headers"]["Origin"],
					"Access-Control-Allow-Methods", "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY",
					"Access-Control-Allow-Headers", ACCESS_CONTROL_HEADERS,
					"Access-Control-Expose-Headers", ACCESS_CONTROL_HEADERS,
					"Access-Control-Max-Age", "1728000"
				}
			)
		)
	};
}

auto zpt::EventListener::patch(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::reply(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> zpt::json {
	return zpt::undefined;
}
