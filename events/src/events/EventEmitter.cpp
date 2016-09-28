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

zpt::EventEmitter::EventEmitter() : __self( this ) {
}

zpt::EventEmitter::EventEmitter(zpt::json _options) :  __options( _options), __self( this ) {
}

zpt::EventEmitter::~EventEmitter() {
}

zpt::json zpt::EventEmitter::options() {
	return this->__options;
}

zpt::EventEmitterPtr zpt::EventEmitter::self() {
	return this->__self;
}

std::string zpt::EventEmitter::version() {
	return this->__options["rest"]["version"]->str();
}

void zpt::EventEmitter::add_kb(std::string _name, zpt::kb _kb) {
	auto _found = this->__kb.find(_name);
	if (_found == this->__kb.end()) {
		this->__kb.insert(make_pair(_name, _kb));
	}
}

zpt::kb zpt::EventEmitter::get_kb(std::string _name) {
	auto _found = this->__kb.find(_name);
	if (_found == this->__kb.end()) {
		return zpt::kb(nullptr);
	}
	return _found->second;
}

zpt::json zpt::ev::split(std::string _url, zpt::json _orphans) {
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

std::string zpt::ev::join(zpt::json _info, size_t _orphans) {
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

std::string zpt::ev::to_str(zpt::ev::performative _performative) {
	switch(_performative) {
		case zpt::ev::Get : {
			return "GET";
		}
		case zpt::ev::Put : {
			return "PUT";
		}
		case zpt::ev::Post : {
			return "POST";
		}
		case zpt::ev::Delete : {
			return "DELETE";
		}
		case zpt::ev::Head : {
			return "HEAD";
		}
		case zpt::ev::Options : {
			return "OPTIONS";
		}
		case zpt::ev::Patch: {
			return "PATCH";
		}
		case zpt::ev::Reply: {
			return "REPLY";
		}
	}
	return "HEAD";
}

zpt::ev::performative zpt::ev::from_str(std::string _performative) {
	if (_performative == "GET") {
		return zpt::ev::Get;
	}
	if (_performative == "PUT") {
		return zpt::ev::Put;
	}
	if (_performative == "POST") {
		return zpt::ev::Post;
	}
	if (_performative == "DELETE") {
		return zpt::ev::Delete;
	}
	if (_performative == "HEAD") {
		return zpt::ev::Head;
	}
	if (_performative == "OPTIONS") {
		return zpt::ev::Options;
	}
	if (_performative == "PATCH") {
		return zpt::ev::Patch;
	}
	if (_performative == "REPLY") {
		return zpt::ev::Reply;
	}
	return zpt::ev::Head;
}

zpt::json zpt::ev::init_request(std::string _cid) {
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
	return _return;
}

zpt::json zpt::ev::init_reply(std::string _uuid) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	zpt::json _return(
		{
			"Server", "zapata RESTful server",
			"Cache-Control", "max-age=3600",
			"Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag",
			"Date", std::string(_buffer_date),
			"Expires", std::string(_buffer_expires)
		}
	);
	if (_uuid != "") {
		_return << "X-Cid" << _uuid;
	}
	return _return;
}

zpt::EventListener::EventListener(std::string _regex) : __regex(_regex) {
}

zpt::EventListener::~EventListener() {
}

std::string zpt::EventListener::regex() {
	return this->__regex;
}

zpt::json zpt::EventListener::get(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

zpt::json zpt::EventListener::put(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

zpt::json zpt::EventListener::post(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

zpt::json zpt::EventListener::del(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

zpt::json zpt::EventListener::head(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

zpt::json zpt::EventListener::options(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
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

zpt::json zpt::EventListener::patch(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

zpt::json zpt::EventListener::reply(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) {
	return zpt::undefined;
}
