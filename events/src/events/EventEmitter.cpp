/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <n@zgul.me>

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

zpt::EventEmitter::EventEmitter() : __self(this), __keeper(nullptr), __directory(nullptr) {
}

zpt::EventEmitter::EventEmitter(zpt::json _options) :  __options(_options), __self(this), __keeper((new zpt::EventGatekeeper(_options))->self()), __directory((new zpt::EventDirectory(_options))->self()), __uuid(zpt::generate::r_uuid()) {
}

zpt::EventEmitter::~EventEmitter() {
}

auto zpt::EventEmitter::uuid() -> std::string {
	return this->__uuid;
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

auto zpt::EventEmitter::authorize(std::string _topic, zpt::json _envelope, zpt::json _roles_needed) -> zpt::json {
	return this->__keeper->authorize(_topic, _envelope, _roles_needed);
}

auto zpt::EventEmitter::lookup(std::string _topic, zpt::ev::performative _performative) -> zpt::ev::node {
	return this->__directory->lookup(_topic, _performative);
}

auto zpt::EventEmitter::connector(std::string _name, zpt::connector _connector) -> void {
	auto _found = this->__connector.find(_name);
	if (_found == this->__connector.end()) {
		ztrace(std::string("registering connector ") + _name + std::string("@") + _connector->name());
		_connector->events(this->__self);
		try {
			_connector->connect();
		}
		catch(std::exception& _e) {
			zlog(_e.what(), zpt::error);
			return;
		}
		this->__connector.insert(std::make_pair(_name, _connector));
	}
}

auto zpt::EventEmitter::connector(std::map<std::string, zpt::connector> _connectors) -> void {
	for (auto _connector : _connectors) {
		this->connector(_connector.first, _connector.second);
	}
}

auto zpt::EventEmitter::connector(std::string _name) -> zpt::connector {
	auto _found = this->__connector.find(_name);
	assertz(_found != this->__connector.end(), std::string("theres isn't any connector by the name '") + _name + std::string("'"), 500, 0);
	return _found->second;
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
		zpt::json{
			"Accept", "application/json",
			"Accept-Charset", "utf-8",
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

auto zpt::ev::init_reply(std::string _uuid, zpt::json _request) -> zpt::json {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	zpt::json _return(
		zpt::json{
			"Server", "zapata RESTful server",
			"Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag",
			"Date", std::string(_buffer_date)
		}
	);
	if (_uuid != "") {
		_return << "X-Cid" << _uuid;
	}
	if (_request["headers"]["Connection"]->ok()) {
		_return << "Connection" << _request["headers"]["Connection"];
	}

	if (_request->ok()) {
		if (_request["headers"]["X-No-Redirection"]->ok() && std::string(_request["headers"]["X-No-Redirection"]) == "true") {
			if (((int) _return["status"]) > 299 && ((int) _return["status"]) < 400) {
				_return << "status" << 200;
				_return["headers"] << "X-Redirect-To" << _return["headers"]["Location"];
			}
		}
		if (_request["headers"]["Origin"]->ok()) {
			_return <<
			"Access-Control-Allow-Origin" << _request["headers"]["Origin"] <<
			"Access-Control-Allow-Credentials" << "true" <<
			"Access-Control-Allow-Methods" << "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY" <<
			"Access-Control-Allow-Headers" << REST_ACCESS_CONTROL_HEADERS <<
			"Access-Control-Expose-Headers" << REST_ACCESS_CONTROL_HEADERS <<
			"Access-Control-Max-Age" << "1728000";
		}
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

auto zpt::EventListener::get(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::put(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::post(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::del(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::head(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::options(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	if (_envelope["headers"]["Origin"]->ok()) {
		_emitter->reply(_envelope, {
			"status", 413,
			"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]))
		});
	}
	string _origin = _envelope["headers"]["Origin"];
	_emitter->reply(_envelope, {
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
	});
}

auto zpt::EventListener::patch(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
	assertz(false, "Performative is not accepted for the given resource", 405, 0);
}

auto zpt::EventListener::reply(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter) -> void {
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

zpt::EventDirectory::EventDirectory(zpt::json _options) : __options(_options), __self(this), __services(new zpt::EventDirectoryGraph("",  std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*")))) {
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

auto zpt::EventDirectory::lookup(std::string _topic, zpt::ev::performative _performative) -> zpt::ev::node {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	zpt::ev::node _found = this->__services->find(_topic, _performative);
	if (std::get<0>(_found)->is_object()) {
		size_t _next = size_t(std::get<0>(_found)["next"]);
		zpt::json _peers = std::get<0>(_found)["peers"];
		zpt::json _container = _peers[_next];
		if (_next == _peers->arr()->size() - 1) {
			_next = 0;
		}
		else {
			_next++;
			std::get<0>(_found) << "next" << _next;
		}
		return std::make_tuple(_container, std::get<1>(_found), std::get<2>(_found));
	}
	return _found;
}

auto zpt::EventDirectory::notify(std::string _topic, zpt::ev::node _connection) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	std::get<0>(_connection) << "connect" << zpt::r_replace(std::string(std::get<0>(_connection)["connect"]), "tcp://*:", std::string("tcp://127.0.0.1:"));
	if (std::get<1>(_connection).size() != 0 && !std::get<0>(_connection)["performatives"]["NOTIFY"]->ok() && !std::get<0>(_connection)["performatives"]["SEARCH"]->ok()) {
		this->__emitter->route(
			zpt::ev::Notify,
			"*",
			{ "headers", { "MAN", "\"ssdp:discover\"", "MX", "3", "ST", (std::string("urn:schemas-upnp-org:service:") + _topic), "Location", std::get<0>(_connection)["connect"] },
				"payload", std::get<0>(_connection) },
			{ "upnp", true }
		);
	}
	this->__services->insert(_topic, _connection);
}

auto zpt::EventDirectory::list(std::string _uuid) -> zpt::json {
	return this->__services->list(_uuid);
}

auto zpt::EventDirectory::pretty() -> std::string {
	return this->__services->pretty();
}

zpt::EventDirectoryGraph::EventDirectoryGraph(std::string _resolver, zpt::ev::node _service) : __resolver(_resolver), __service(_service) {
}

zpt::EventDirectoryGraph::~EventDirectoryGraph() {
}
		
auto zpt::EventDirectoryGraph::insert(std::string _topic, zpt::ev::node _service) -> void {
	zpt::json _topics = zpt::ev::uri::get_simplified_topics(_topic);
	for (auto _topic : _topics->arr()) {
		this->insert(zpt::path::split(std::string(_topic)), _service);
	}
}

auto zpt::EventDirectoryGraph::insert(zpt::json _topic, zpt::ev::node _service) -> void {	
	if (_topic->arr()->size() == 0) {
		zpt::json _containers = std::get<0>(this->__service)["peers"];
		if (_containers->is_array()) {
			zpt::json _new_containers = std::get<0>(_service);
			if (_new_containers->is_array()) {
				for (auto _nc : _new_containers->arr()) {
					bool _found = false;
					for (auto _c : _containers->arr()) {
						if (_nc["connect"] == _c["connect"]) {
							_c << "uuid" << _nc["uuid"] << "type" << _nc["type"];
							_found = true;
							break;
						}
					}
					if (!_found) {
						_containers << _nc;
					}
				}
			}
		}
		else {
			zpt::json _s_data = std::get<0>(_service)->clone();
			std::get<0>(_service)->obj()->clear();
			std::get<0>(_service) << "peers" << zpt::json{ zpt::array, _s_data } << "next" << 0;
			// std::get<0>(_service) = { "peers", { zpt::array, std::get<0>(_service) }, "next", 0 };
			this->__service = _service;
		}
		return;
	}
	
	std::string _resolver = std::string(_topic[0]);
	auto _child = this->__children.find(_resolver);
	if (_child == this->__children.end()) {
		this->__children.insert(std::make_pair(_resolver.data(), zpt::ev::graph(new zpt::EventDirectoryGraph(_resolver, std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"))))));
		_child = this->__children.find(_resolver);
	}

	_topic->arr()->erase(_topic->arr()->begin());
	_child->second->insert(_topic, _service);
}

auto zpt::EventDirectoryGraph::find(std::string _topic, zpt::ev::performative _performative) -> zpt::ev::node {
	zpt::json _splited = zpt::path::split(_topic);
	if (std::string(_splited[0]).find(":") != std::string::npos) {
		zpt::json _uri = zpt::uri::parse(_topic);
		std::string _connect = std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["authority"]);
		return std::make_tuple(zpt::json{ "peers", { zpt::array, { "topic", zpt::r_replace(_topic, _connect, ""), "type", (_uri["scheme"] == zpt::json::string("tcp") ? "req" : _uri["scheme"]), "connect",  _connect, "regex", ".*" } }, "next", 0 }, zpt::ev::handlers(), std::regex(".*"));
	}
	return this->find(_topic, _splited, _performative);
}

auto zpt::EventDirectoryGraph::find(std::string _topic, zpt::json _splited, zpt::ev::performative _performative) -> zpt::ev::node {
	zpt::json _containers = std::get<0>(this->__service);
	if (_containers->is_object()) {
		if (std::regex_match(_topic, std::get<2>(this->__service))) {
			if (_performative == zpt::ev::Connect) {
				return this->__service;
			}

			for (auto _c : _containers["peers"]->arr()) {
				if (_c["performatives"][zpt::ev::to_str(_performative)]->ok()) {
					return this->__service;
				}
			}
		}
	}

	if (_splited->arr()->size() == 0) {
		return  std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
	}
	
	auto _child = this->__children.find(std::string(_splited[0]));
	if (_child != this->__children.end()) {
		zpt::json _less = _splited->clone();
		_less->arr()->erase(_less->arr()->begin());
		zpt::ev::node _result = _child->second->find(_topic, _less, _performative);
		if (std::get<0>(_result)->ok()) {
			return _result;
		}
	}
	
	_child = this->__children.find("+");
	if (_child != this->__children.end()) {
		zpt::json _less = _splited->clone();
		_less->arr()->erase(_less->arr()->begin());
		zpt::ev::node _result = _child->second->find(_topic, _less, _performative);
		if (std::get<0>(_result)->ok()) {
			return _result;
		}
	}
	
	_child = this->__children.find("*");
	if (_child != this->__children.end()) {
		zpt::json _less = _splited->clone();
		do {
			_less->arr()->erase(_less->arr()->begin());
			zpt::ev::node _result = _child->second->find(_topic, _less, _performative);
			if (std::get<0>(_result)->ok()) {
				return _result;
			}
		}
		while(_less->arr()->size() != 0);
	}
	
	return  std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
}

auto zpt::EventDirectoryGraph::list(std::string _uuid) -> zpt::json {
	zpt::json _containers = std::get<0>(this->__service);
	zpt::json _return;
	
	if (_containers->is_object()) {
		if (_uuid.length() != 0) {
			_return = zpt::json::array();
			for (auto _container : _containers["peers"]->arr()) {
				if (_container["uuid"] == zpt::json::string(_uuid)) {
					_return << _container;
				}
			}
		}
		else {
			_return = _containers["peers"]->clone();
		}
	}
	else {
		_return = zpt::json::array();
	}

	for (auto _child : this->__children) {
		_return = _return + _child.second->list(_uuid);
	}
	
	return  _return;
}

auto zpt::EventDirectoryGraph::pretty(std::string _tabs, bool _last) -> std::string {
	std::string _return;
	if (this->__resolver != "") {
		_return = (_tabs + (_tabs != "" ? std::string(!_last ? "├─ " : "└─ ") : std::string("─ ")) + this->__resolver + (std::get<0>(this->__service)->is_object() ? std::string(" (") + std::to_string(std::get<0>(this->__service)["peers"]->arr()->size()) + std::string(")") : std::string("")) + std::string("\n"));
	}
	
	size_t _idx = 0;
	for (auto _child : this->__children) {
		_return += _child.second->pretty((this->__resolver != "" ? _tabs + std::string(!_last ? "│\t" : "\t") : std::string("")), _idx == this->__children.size() - 1);
		_idx++;
	}		
	return _return;
}

extern "C" auto zpt_events() -> int {
	return 1;
}

auto zpt::ev::pretty(zpt::json _envelope) -> std::string {
	std::string _protocol = (_envelope["protocol"]->ok() ? std::string(_envelope["protocol"]) : "ZPT/1.0");
	std::string _ret("\n");
	if (_envelope["status"]->ok()) {
		std::string _color;
		if (int(_envelope["status"]) < 300) {
			_color.assign("32");
		}
		else if (int(_envelope["status"]) < 400) {
			_color.assign("33");
		}
		else {
			_color.assign("31");
		}
		_ret += _protocol + std::string(" \033[1;") + _color + std::string("m") + std::string(zpt::status_names[int(_envelope["status"])]) + std::string("\033[0m\n"); 
	}
	else {
		_ret += zpt::ev::to_str(zpt::ev::performative(int(_envelope["performative"]))) + std::string(" \033[1;35m") + _envelope["resource"]->str() + std::string("\033[0m");
		if (_envelope["params"]->is_object()) {
			_ret += std::string("?");
			bool _first = true;
			for (auto _param : _envelope["params"]->obj()) {
				if (!_first) {
					_ret += std::string("&");
				}
				_first = false;
				_ret += _param.first + std::string("=") + std::string(_param.second);
			}
		}
		_ret += std::string(" ") + _protocol + std::string("\n"); 
	}
	if (_envelope["headers"]->ok()) {
		for (auto _header : _envelope["headers"]->obj()) {
			_ret += _header.first + std::string(": ") + std::string(_header.second) + std::string("\n");
		}
	}
	_ret += std::string("\n");
	if (_envelope["payload"]->ok() && !_envelope["payload"]->empty()) {
		_ret += zpt::json::pretty(_envelope["payload"]);
	}
	return _ret;
}

auto zpt::ev::uri::get_simplified_topics(std::string _pattern) -> zpt::json {
	zpt::json _aliases = zpt::split(_pattern, "|");
	zpt::json _topics = zpt::json::array();
	char _op;
	for (auto _alias : _aliases->arr()) {
		std::string _return;
		short _state = 0;
		bool _regex = false;
		bool _escaped = false;
		for (auto _c : _alias->str()) {
			switch (_c) {
				case '/' : {
					if (_state == 0) {
						if (_regex) {
							if (_return.back() != '/') {
								_return.push_back('/');
							}
							_return.push_back(_op);
							_regex = false;
						}
						_return.push_back(_c);
					}
					break;
				}
				case ')' : 
				case ']' : {
					if (!_escaped) {
						_state--;
					}
					else {
						_escaped = false;
					}
					_regex = true;
					break;
				}
				case '(' :
				case '[' : {
					if (!_escaped) {
						_state++;
					}
					else {
						_escaped = false;
					}
					_regex = true;
					break;
				}
				case '{' :
				case '}' :
				case '.' :
				case '+' : {
					_op = '+';
					_regex = true;
					break;
				}
				case '*' : {
					_op = '*';
					_regex = true;
					break;
				}
				case '$' :
				case '^' : {
					break;
				}
				case '\\' : {
					_escaped = !_escaped;
					break;
				}
				default : {
					if (_state == 0) {
						_return.push_back(_c);
					}
				}
			}		
		}
		if (_regex) {
			if (_return.back() != '/') {
				_return.push_back('/');
			}
			_return.push_back(_op);
		}
		_topics << _return;
	}
	return _topics;
}

auto zpt::ev::not_found(std::string _resource, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"headers", _headers,
		"status", 404,
		"payload", {
			"text", "resource not found",
			"code", 1101,
			"assertion_failed", "_container->ok()"
		}
	};
}

auto zpt::ev::bad_request(zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", zpt::generate::r_uuid(),
		"headers", _headers,
		"status", 400,
		"payload", {
			"text", "bad request",
			"code", 1100,
			"assertion_failed", "_socket >> _req"
		}
	};
}

auto zpt::ev::accepted(std::string _resource, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"headers", _headers,
		"status", 202,
		"payload", {
			"text", "request was accepted"
		}
	};
}

auto zpt::ev::no_content(std::string _resource, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"headers", _headers,
		"status", 204
	};
}

auto zpt::ev::temporary_redirect(std::string _resource, std::string _target_resource, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 307,
		"headers", _headers + zpt::json{
			"Location", _target_resource
		},
		"payload", {
			"text", "temporarily redirecting you to another location"
		}
	};
}

auto zpt::ev::see_other(std::string _resource, std::string _target_resource, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 303,
		"headers", _headers + zpt::json{
			"Location", _target_resource
		},
		"payload", {
			"text", "temporarily redirecting you to another location"
		}
	};
}

auto zpt::ev::options(std::string _resource, std::string _origin, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 200,
		"headers", _headers + zpt::json{
			"Access-Control-Allow-Origin", _origin,
			"Access-Control-Allow-Methods", "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY",
			"Access-Control-Allow-Headers", REST_ACCESS_CONTROL_HEADERS,
			"Access-Control-Expose-Headers", REST_ACCESS_CONTROL_HEADERS,
			"Access-Control-Max-Age", "1728000"
		},
		"payload", {
			"text", "request was accepted"
		}
	};
}

auto zpt::ev::internal_server_error(std::exception& _e, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"status", 500,
		"headers", _headers,
		"payload", {
			"text", _e.what(),
			"code", 0
		}
	};
}

auto zpt::ev::assertion_error(zpt::assertion& _e, zpt::json _headers) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"status", _e.status(),
		"headers", _headers,
		"payload", {
			"text", _e.what(),
			"assertion_failed", _e.description(),
			"code", _e.code()
		}
	};
}

namespace zpt {
	const char* status_names[] = { nullptr,
				       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				       "100 Continue ",
				       "101 Switching Protocols ",
				       "102 Processing ",
				       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				       "200 OK ",
				       "201 Created ",
				       "202 Accepted ",
				       "203 Non-Authoritative Information ",
				       "204 No Content ",
				       "205 Reset Content ",
				       "206 Partial Content ",
				       "207 Multi-Status ",
				       "208 Already Reported ",
				       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				       "226 IM Used ",
				       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				       "300 Multiple Choices ",
				       "301 Moved Permanently ",
				       "302 Found ",
				       "303 See Other ",
				       "304 Not Modified ",
				       "305 Use Proxy ",
				       "306 (Unused) ",
				       "307 Temporary Redirect ",
				       "308 Permanent Redirect ",
				       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				       "400 Bad Request ",
				       "401 Unauthorized ",
				       "402 Payment Required ",
				       "403 Forbidden ",
				       "404 Not Found ",
				       "405 Method Not Allowed ",
				       "406 Not Acceptable ",
				       "407 Proxy Authentication Required ",
				       "408 Request Timeout ",
				       "409 Conflict ",
				       "410 Gone ",
				       "411 Length Required ",
				       "412 Precondition Failed ",
				       "413 Payload Too Large ",
				       "414 URI Too Long ",
				       "415 Unsupported Media Type ",
				       "416 Requested Range Not Satisfiable ",
				       "417 Expectation Failed ",
				       nullptr, nullptr, nullptr, nullptr,
				       "422 Unprocessable Entity ",
				       "423 Locked ",
				       "424 Failed Dependency ",
				       "425 Unassigned ",
				       "426 Upgrade Required ",
				       "427 Unassigned ",
				       "428 Precondition Required ",
				       "429 Too Many Requests ",
				       "430 Unassigned ",
				       "431 Request Header Fields Too Large ",
				       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				       "451 Unavailable For Legal Reasons",
				       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
				       "500 Internal Server Error ",
				       "501 Not Implemented ",
				       "502 Bad Gateway ",
				       "503 Service Unavailable ",
				       "504 Gateway Timeout ",
				       "505 HTTP Version Not Supported ",
				       "506 Variant Also Negotiates (Experimental) ",
				       "507 Insufficient Storage ",
				       "508 Loop Detected ",
				       "509 Unassigned ",
				       "510 Not Extended ",
				       "511 Network Authentication Required ",
	};
}
