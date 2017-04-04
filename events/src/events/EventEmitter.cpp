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

zpt::BridgePtr::BridgePtr(zpt::Bridge* _target) : std::shared_ptr< zpt::Bridge >(_target) {
}

zpt::BridgePtr::BridgePtr() : std::shared_ptr< zpt::Bridge >(nullptr) {
}

zpt::Bridge::Bridge(zpt::json _options) : __options(_options) {
}

zpt::Bridge::~Bridge() {
}
		
auto zpt::Bridge::options() -> zpt::json {
	return this->__options;
}

zpt::EventEmitter::EventEmitter() : __self(this), __mutant(nullptr), __keeper(nullptr), __directory(nullptr) {
}

zpt::EventEmitter::EventEmitter(zpt::json _options) :  __options(_options), __self(this), __mutant(nullptr), __keeper((new zpt::EventGatekeeper(_options))->self()), __directory((new zpt::EventDirectory(_options))->self()) {
}

zpt::EventEmitter::~EventEmitter() {
}

auto zpt::EventEmitter::options() -> zpt::json {
	return this->__options;
}
					 
auto zpt::EventEmitter::self() const -> zpt::ev::emitter {
	return this->__self;
}

auto zpt::EventEmitter::unbind() -> void {
	this->__self.reset();
}

auto zpt::EventEmitter::mutations() -> zpt::mutation::emitter {
	return this->__mutant;
}

auto zpt::EventEmitter::mutations(zpt::mutation::emitter _emitter) -> void {
	if (this->__mutant.get() != nullptr) {
		this->__mutant->unbind();
	}
	_emitter->events(this->self());
	this->__mutant = _emitter;
}

auto zpt::EventEmitter::gatekeeper() -> zpt::ev::gatekeeper {
	return this->__keeper;
}

auto zpt::EventEmitter::gatekeeper(zpt::ev::gatekeeper _gatekeeper) -> void {
	this->__keeper->unbind();
	_gatekeeper->events(this->self());
	this->__keeper = _gatekeeper;
}

auto zpt::EventEmitter::directory() -> zpt::ev::directory {
	return this->__directory;
}

auto zpt::EventEmitter::directory(zpt::ev::directory _directory) -> void {
	this->__directory->unbind();
	_directory->events(this->self());
	this->__directory = _directory;
}

auto zpt::EventEmitter::authorize(std::string _topic, zpt::json _envelope, zpt::json _roles_needed) -> zpt::json {
	return this->__keeper->authorize(_topic, _envelope, _roles_needed);
}

auto zpt::EventEmitter::lookup(std::string _topic) -> zpt::json {
	return this->__directory->lookup(_topic);
}

auto zpt::EventEmitter::connector(std::string _name, zpt::connector _connector) -> void {
	_connector->events(this->__self);
	this->__mutant->connector(_name, _connector);
}

auto zpt::EventEmitter::connector(std::map<std::string, zpt::connector> _connectors) -> void {
	for (auto _connector : _connectors) {
		this->connector(_connector.first, _connector.second);
	}
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
			"User-Agent", "zapata RESTful server"
		}
	);
	if (_cid != "") {
		_return << "X-Cid" << _cid;
	}
	else {
		_return << "X-Cid" << zpt::generate::r_uuid();
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
		"Connection", "close",
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
			"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]))
		};
	}
	string _origin = _envelope["headers"]["Origin"];
	return {
		"status", 200,
		"headers", (zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + zpt::json(
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

zpt::EventGatekeeper::EventGatekeeper(zpt::json _options) : __options(_options), __self(this) {
}

zpt::EventGatekeeper::~EventGatekeeper() {
}
		
auto zpt::EventGatekeeper::options() -> zpt::json {
	return this->__options;
}

auto zpt::EventGatekeeper::unbind() -> void {
	this->__self.reset();
}

auto zpt::EventGatekeeper::self() const -> zpt::ev::gatekeeper {
	return this->__self;
}

auto zpt::EventGatekeeper::events() -> zpt::ev::emitter {
	return this->__emitter;
}

auto zpt::EventGatekeeper::events(zpt::ev::emitter _emitter) -> void {
	this->__emitter = _emitter;
}

auto zpt::EventGatekeeper::get_credentials(zpt::json _client_id, zpt::json _client_secret, zpt::json _address, zpt::json _grant_type, zpt::json _scope) -> zpt::json {
	return { "access_token", "--blank--" };
}

auto zpt::EventGatekeeper::authorize(std::string _topic, zpt::json _envelope, zpt::json _roles_needed) -> zpt::json {
	return { "client_id", "anyone", "access_token", "--blank--", "roles", zpt::json::array() };
}

zpt::EventDirectory::EventDirectory(zpt::json _options) : __options(_options), __self(this) {
	if (_options["directory"]->type() == zpt::JSObject) {
		for (auto _api : _options["directory"]->obj()) {
			if (_api.second["endpoints"]->type() != zpt::JSArray) {
				continue;
			}
			for (auto _endpoint : _api.second["endpoints"]->arr()) {
				if (_endpoint->type() != zpt::JSString) {
					continue;
				}
				try {
					std::regex _regex(std::string("^") + _endpoint->str() + std::string("(.*)$"));
					this->__index.insert(std::make_pair(std::string("^") + _endpoint->str() + std::string("(.*)$"), std::make_pair(_regex, zpt::json({ "connect", _api.second["connect"], "type", _api.second["type"] }))));
				}
				catch(std::exception& _e) {}
			}
		}
	}
}

zpt::EventDirectory::~EventDirectory() {
}
		
auto zpt::EventDirectory::options() -> zpt::json {
	return this->__options;
}

auto zpt::EventDirectory::unbind() -> void {
	this->__self.reset();
}

auto zpt::EventDirectory::self() const -> zpt::ev::directory {
	return this->__self;
}

auto zpt::EventDirectory::events() -> zpt::ev::emitter {
	return this->__emitter;
}

auto zpt::EventDirectory::events(zpt::ev::emitter _emitter) -> void {
	this->__emitter = _emitter;
}

auto zpt::EventDirectory::lookup(std::string _topic) -> zpt::json {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	for (auto _service : this->__index) {
		if (std::regex_match(_topic, _service.second.first)) {
			return _service.second.second;
		}
	}
	return zpt::undefined;
}

auto zpt::EventDirectory::notify(std::string _topic, zpt::json _connection) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	std::regex _regex(_topic);
	zpt::json _record = (_connection->type() == zpt::JSArray ? _connection[0] : _connection);
	_record << "connect" << zpt::r_replace(_record["connect"]->str(), "tcp://*:", std::string("tcp://127.0.0.1:"));
	this->__index.insert(std::make_pair(_topic, std::make_pair(_regex, _record)));
}

extern "C" auto zpt_events() -> int {
	return 1;
}
