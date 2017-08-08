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

#include <zapata/rest/RESTEmitter.h>
#include <zapata/lisp.h>
#include <zapata/python.h>
#include <map>

namespace zpt {
	namespace rest {
		emitter* __emitter = nullptr;
	}
}

zpt::RESTEmitter::RESTEmitter(zpt::json _options) : zpt::EventEmitter(_options), __poll(nullptr), __server(nullptr) {
	if (zpt::rest::__emitter != nullptr) {
		delete zpt::rest::__emitter;
		zlog("something is definitely wrong, RESTEmitter already initialized", zpt::emergency);
	}
	zpt::rest::__emitter = this;
	this->__default_options = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		if (_envelope["headers"]["Origin"]->ok()) {
			return {
				"status", 413,
				"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]))
			};
		}
		std::string _origin = _envelope["headers"]["Origin"];
		return {
			"status", 200,
			"headers", (zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + zpt::json(
					{
						"Access-Control-Allow-Origin", _envelope["headers"]["Origin"],
						"Access-Control-Allow-Methods", "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY",
						"Access-Control-Allow-Headers", REST_ACCESS_CONTROL_HEADERS,
						"Access-Control-Expose-Headers", REST_ACCESS_CONTROL_HEADERS,
						"Access-Control-Max-Age", "1728000"
					}
				)
			)
		};
	};
}

zpt::RESTEmitter::~RESTEmitter() {
}

auto zpt::RESTEmitter::version() -> std::string {
	return this->options()["rest"]["version"]->str();
}

auto zpt::RESTEmitter::credentials() -> zpt::json {
	return this->__credentials;
}

auto zpt::RESTEmitter::credentials(zpt::json _credentials) -> void {
	this->__credentials = _credentials;
}

auto zpt::RESTEmitter::poll(zpt::poll _poll) -> void {
	this->__poll = _poll;
}

auto zpt::RESTEmitter::poll() -> zpt::poll {
	return this->__poll;
}

auto zpt::RESTEmitter::server(zpt::rest::server _server) -> void {
	this->__server = _server;
}

auto zpt::RESTEmitter::server() -> zpt::rest::server {
	return this->__server;
}

auto zpt::RESTEmitter::hook(zpt::ev::initializer _callback) -> void {
	this->__server->hook(_callback);
}

auto zpt::RESTEmitter::init_thread() -> zpt::thread::context {
	zpt::RESTThreadContext* _context = new zpt::RESTThreadContext();
	if (zpt::bridge::is_booted< zpt::lisp::bridge >()) {
		ecl_import_current_thread(ECL_NIL, ECL_NIL);
	}
	if (zpt::bridge::is_booted< zpt::python::bridge >()) {
		//_context->python_state = PyGILState_Ensure();
		_context->python_thread_state = PyEval_SaveThread();
	}
	return zpt::thread::context(_context);
}

auto zpt::RESTEmitter::dispose_thread(zpt::thread::context _context) -> void {
	if (zpt::bridge::is_booted< zpt::lisp::bridge >()) {
		ecl_release_current_thread();
	}
	if (zpt::bridge::is_booted< zpt::python::bridge >()) {
		//PyGILState_Release(((zpt::RESTThreadContext*) _context.get())->python_state);
		PyEval_RestoreThread((PyThreadState*) ((zpt::RESTThreadContext*) _context.get())->python_thread_state);
	}
}

auto zpt::RESTEmitter::get_hash(std::string _pattern) -> std::string {
	zpt::json _splited = zpt::split(_pattern, "/");
	std::string _return;
	for (auto _p : _splited->arr()) {
		std::string _part = _p->str();
		if (_part.back() == '$') {
			_part.erase(_part.length() - 1, 1);
		}
		if (zpt::test::ascii(_part)) {
			_return += std::string("/") + _part;
		}
		else if (_part != "^") {
			break;
		}
	}
	return _return;
}

auto zpt::RESTEmitter::add_by_hash(std::string _topic, std::regex& _url_pattern, zpt::ev::handlers& _handlers) -> void {
	std::string _possible_hash = this->get_hash(_topic);
	zpt::json _splited = zpt::split(_possible_hash, "/");
	std::string _hash;
	for (auto _i : _splited->arr()) {
		_hash += std::string("/") + _i->str();
		if (this->__hashed.find(_hash) != this->__hashed.end()) {
			break;
		}
	}
	auto _h_found = this->__hashed.find(_hash);
	if (_h_found == this->__hashed.end()) {
		std::vector< std::pair< std::regex, zpt::ev::handlers > > _bag;
		_bag.push_back(std::make_pair(_url_pattern, _handlers));
		this->__hashed.insert(std::make_pair(_hash, _bag));
	}
	else {
		_h_found->second.push_back(std::make_pair(_url_pattern, _handlers));
	}
}

auto zpt::RESTEmitter::on(zpt::ev::performative _event, std::string _regex, zpt::ev::Handler _handler, zpt::json _opts) -> std::string {
	std::regex _url_pattern(_regex);

	zpt::ev::handlers _handlers;
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Get ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Put ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Post ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Delete ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Head ? nullptr : _handler));
	_handlers.push_back(this->__default_options);
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Patch ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Reply ? nullptr : _handler));

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	if (_event != zpt::ev::Reply) {
		this->add_by_hash(_regex, _url_pattern, _handlers);
	}
	else {
		this->add_by_hash("/___reply___", _url_pattern, _handlers);
	}

	if (_event != zpt::ev::Reply && std::string(this->options()["proc"]["directory_register"]) != "off") {
		this->directory()->notify(_regex, this->options()["zmq"]);
	}
	if (std::string(this->options()["proc"]["mqtt_register"]) != "off") {
		this->server()->subscribe(_regex, _opts);
	}

	ztrace(std::string("registered handlers for ") + _regex);
	return _uuid;
}

auto zpt::RESTEmitter::on(std::string _regex, std::map< zpt::ev::performative, zpt::ev::Handler > _handler_set, zpt::json _opts) -> std::string {
	std::regex _url_pattern(_regex);
	bool _to_register = (_handler_set.find(zpt::ev::Get) != _handler_set.end() || _handler_set.find(zpt::ev::Put) != _handler_set.end() || _handler_set.find(zpt::ev::Post) != _handler_set.end() || _handler_set.find(zpt::ev::Delete) != _handler_set.end() || _handler_set.find(zpt::ev::Head) != _handler_set.end() || _handler_set.find(zpt::ev::Options) != _handler_set.end() || _handler_set.find(zpt::ev::Patch) != _handler_set.end());

	std::map< zpt::ev::performative, zpt::ev::Handler >::iterator _found;
	zpt::ev::handlers _handlers;
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Get)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Put)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Post)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Delete)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Head)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back(this->__default_options);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Patch)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Reply)) == _handler_set.end() ? nullptr : _found->second);

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	if (_to_register) {
		this->add_by_hash(_regex, _url_pattern, _handlers);
	}
	else {
		this->add_by_hash("/___reply___", _url_pattern, _handlers);
	}

	if (_to_register && std::string(this->options()["proc"]["directory_register"]) != "off") {
		this->directory()->notify(_regex, this->options()["zmq"]);
	}
	if (std::string(this->options()["proc"]["mqtt_register"]) != "off") {
		this->server()->subscribe(_regex, _opts);
	}

	ztrace(std::string("registered handlers for ") + _regex);
	return _uuid;
}

auto zpt::RESTEmitter::on(zpt::ev::listener _listener, zpt::json _opts) -> std::string {
	std::regex _url_pattern(_listener->regex());

	zpt::ev::Handler _handler = [ & ] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> void {
		switch (_performative) {
			case zpt::ev::Get : {
				 _listener->get(_resource, _envelope, _emitter);
				 break;
			}
			case zpt::ev::Put : {
				 _listener->put(_resource, _envelope, _emitter);
				 break;
			}
			case zpt::ev::Post : {
				 _listener->post(_resource, _envelope, _emitter);
				 break;
			}
			case zpt::ev::Delete : {
				 _listener->del(_resource, _envelope, _emitter);
				 break;
			}
			case zpt::ev::Head : {
				 _listener->head(_resource, _envelope, _emitter);
				 break;
			}
			case zpt::ev::Options : {
				 _listener->options(_resource, _envelope, _emitter);
				 break;
			}
			case zpt::ev::Patch : {
				 _listener->patch(_resource, _envelope, _emitter);
				 break;
			}
			case zpt::ev::Reply : {
				 _listener->reply(_resource, _envelope, _emitter);
				 break;
			}
		}
	};
	
	zpt::ev::handlers _handlers;
	for (short _idx = zpt::ev::Get; _idx != zpt::ev::Reply + 1; _idx++) {
		_handlers.push_back(_handler);
	}
	
	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	this->add_by_hash(_listener->regex(), _url_pattern, _handlers);

	if (std::string(this->options()["proc"]["directory_register"]) != "off") {
		this->directory()->notify(_listener->regex(), this->options()["zmq"]);
	}
	if (std::string(this->options()["proc"]["mqtt_register"]) != "off") {
		this->server()->subscribe(_listener->regex(), _opts);
	}

	ztrace(std::string("registered handlers for ") + _listener->regex());
	return _uuid;
}

auto zpt::RESTEmitter::off(zpt::ev::performative _event, std::string _callback_id) -> void {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		_found->second.second[_event] = nullptr;
	}
}

auto zpt::RESTEmitter::off(std::string _callback_id) -> void {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		this->__resources.erase(_callback_id);
	}
}

auto zpt::RESTEmitter::pending(zpt::json _envelope, zpt::ev::handler _callback) -> void {
	auto _exists = this->__pending.find(std::string(_envelope["channel"]));
	if (_exists != this->__pending.end()) {
		auto _first_callback = _exists->second;
		this->__pending.erase(_exists);
		this->__pending.insert(std::make_pair(std::string(_envelope["channel"]),
				[ _first_callback, _callback ] (zpt::ev::performative _p_performative, std::string _p_topic, zpt::json _p_envelope, zpt::ev::emitter _p_emitter) mutable -> void {
					_first_callback(_p_performative, _p_topic, _p_envelope, _p_emitter);
					_callback(_p_performative, _p_topic, _p_envelope, _p_emitter);
				}
			)
		);
	}
	else {
		this->__pending.insert(std::make_pair(std::string(_envelope["channel"]), _callback));
	}
}

auto zpt::RESTEmitter::has_pending(zpt::json _envelope) -> bool {
	return this->__pending.find(std::string(_envelope["channel"])) != this->__pending.end();
}

auto zpt::RESTEmitter::reply(zpt::json _request, zpt::json _reply) -> void {
	auto _exists = this->__pending.find(std::string(_request["channel"]));
	if (_exists != this->__pending.end()) {
		auto _callback = _exists->second;
		_callback(zpt::ev::Reply, std::string(_request["resource"]), _reply, this->self());
		this->__pending.erase(_exists);
	}
}

auto zpt::RESTEmitter::trigger(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts, zpt::ev::handler _callback) -> void {
	return this->resolve(_method, _url, _envelope, _opts + zpt::json{ "broker", (this->options()["broker"]->ok() && std::string(this->options()["broker"]) == "true") }, _callback);
}

// auto zpt::RESTEmitter::route(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts) -> zpt::json {
// }

auto zpt::RESTEmitter::route(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::ev::handler _callback) -> void {
	this->route(_method, _url, _envelope, zpt::undefined, _callback);
}

auto zpt::RESTEmitter::route(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts, zpt::ev::handler _callback) -> void {
	assertz(_url.length() != 0, "resource URI must be valid", 400, 0);
	
	if (bool(_opts["mqtt"])) {
		if (bool(_opts["no-envelope"])) {
			this->__server->publish(_url, _envelope);
		}
		else {
			zpt::json _in = _envelope + zpt::json{ 
				"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"]),
				"performative", _method,
				"resource", _url
			};
			this->__server->publish(_url, _in);
		}
		return;
	}
	
	this->resolve(_method, _url, _envelope, _opts + zpt::json{ "broker", true }, _callback);
}

auto zpt::RESTEmitter::resolve(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts, zpt::ev::handler _callback) -> void {
	zpt::json _return;
	bool _endpoint_found = false;
	bool _method_found = false;

	_envelope = _envelope + zpt::json{ 
		"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"]),
		"performative", _method,
		"resource", _url
	};
	if (!zpt::test::uuid(std::string(_envelope["channel"]))) {
		_envelope << "channel" << zpt::generate::r_uuid();
	}

	std::vector< std::pair<std::regex, zpt::ev::handlers > > _resources;
	if (_method != zpt::ev::Reply) {
		zpt::json _splited = zpt::split(_url, "/");
		std::string _hash;
		for (auto _i : _splited->arr()) {
			_hash += std::string("/") + _i->str();
			auto _found = this->__hashed.find(_hash);
			if (_found != this->__hashed.end()) {
				_resources = _found->second;
				break;
			}
		}
	}
	else {
		auto _found = this->__hashed.find("/___reply___");
		if (_found != this->__hashed.end()) {
			_resources = _found->second;
		}
	}

	for (auto _endpoint : _resources) {
		std::regex _regexp = _endpoint.first;
		if (std::regex_match(_url, _regexp)) {
			_endpoint_found = true;
			if (_endpoint.second[_method] != nullptr) {
				_method_found = true;
				try {
					bool _has_callback = false;
					if (_callback != nullptr) {
						this->pending(_envelope, _callback);
						_has_callback = true;
					}
					else if (_opts["bubble-response"]->is_object()) {
						this->pending(_envelope,
							[ _opts ] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) mutable -> void {
								_emitter->reply(_opts["bubble-response"], _envelope);
							}
						);
						_has_callback = true;
					}

					_endpoint.second[_method](_method, _url, _envelope, this->self());
					if (!_has_callback) {
						this->reply(_envelope, { "status", 204 });
					}
					return;
				}
				catch (zpt::assertion& _e) {
					zlog(std::string("error processing '") + _url + std::string("': ") + _e.what() + std::string(", ") + _e.description(), zpt::error);
					if (bool(_opts["bubble-error"])) {
						throw;
					}
					_return = {
						"performative", zpt::ev::Reply,
						"status", _e.status(),
						"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"],
						"payload", {
							"text", _e.what(),
							"assertion_failed", _e.description(),
							"code", _e.code()
						}
					};
				}
				catch(std::exception& _e) {
					zlog(std::string("error processing '") + _url + std::string("': ") + _e.what(), zpt::error);
					if (bool(_opts["bubble-error"])) {
						throw;
					}
					_return = {
						"performative", zpt::ev::Reply,
						"status", 500,
						"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"],
						"payload", {
							"text", _e.what(),
							"code", 0
						}
					};
				}
			}
		}
	}

	if (_method != zpt::ev::Reply) {
		if (!_endpoint_found) {
			this->resolve_remotely(_method, _url, _envelope, _opts, _callback);
			return;
		}
		else if (!_method_found) {
			zlog(std::string("error processing '") + _url + std::string("': method'") + zpt::ev::to_str(_method) + std::string("' not registered"), zpt::error);
			if (bool(_opts["bubble-error"])) {
				assertz(_endpoint_found, "the requested performative is not allowed to be used with the requested resource", 405, 0);
			}
			_return = {
				"performative", zpt::ev::Reply,
				"status", 405,
				"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"],
				"payload", {
					"text", "the requested performative is not allowed to be used with the requested resource"
				}
			};
		}
	}	
	if (this->has_pending(_envelope)) {
		this->reply(_envelope, _return);
	}
}

auto zpt::RESTEmitter::resolve_remotely(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts, zpt::ev::handler _callback) -> void {
	assertz(_url.length() != 0, "resource URI must be valid", 400, 0);

	bool _has_callback = false;
	if (_callback != nullptr) {
		this->pending(_envelope, _callback);
		_has_callback = true;
	}
	else if (_opts["bubble-response"]->is_object()) {
		this->pending(_envelope,
			[ _opts ] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) mutable -> void {
				_emitter->reply(_opts["bubble-response"], _envelope);
			}
		);
		_has_callback = true;
	}
	zpt::json _uri = zpt::uri::parse(_url);
	
	zpt::json _container;
	if (_uri["scheme"] != zpt::json::string("zpt")) {
		std::string _connect = std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["authority"]);
		_container = { "type", (_uri["scheme"] == zpt::json::string("tcp") ? "req" : _uri["scheme"]), "connect",  _connect };
		_envelope << "resource" << zpt::r_replace(_url, _connect, "");
	}
	else {
		_container = this->lookup(_url);
	}
	if (_container->ok()) {
		short _type = zpt::str2type(_container["type"]->str());
		bool _no_answer = false;
		zpt::socket_ref _client;
		switch(_type) {
			case ZMQ_ROUTER_DEALER :
			case ZMQ_ROUTER :
			case ZMQ_REP :
			case ZMQ_REQ : {
				_client = this->__poll->add(ZMQ_REQ, _container["connect"]->str(), true);				
				this->__poll->poll(_client);
				break;
			}
			case ZMQ_PUB_SUB : {
				std::string _connect = _container["connect"]->str();
				_client = this->__poll->add(ZMQ_PUB, _connect.substr(0, _connect.find(",")));
				_no_answer = true;
				break;
			}
			case ZMQ_PUSH : {
				_client = this->__poll->add(ZMQ_PUSH, _container["connect"]->str());
				_no_answer = true;
				break;
			}
			case ZMQ_HTTP_RAW : {
				_client = this->__poll->add(ZMQ_HTTP_RAW, _container["connect"]->str(), true);
				this->__poll->poll(_client);
				break;
			}
		}
		this->pending(_envelope,
			[ = ] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) mutable -> void {
				this->__poll->clean_up(_client);
			}
		);
		_client->send(_envelope);
		if (_no_answer && !_has_callback) {
			this->reply(_envelope, zpt::rest::accepted(_url));
		}
		return;
	}
	zpt::json _out = zpt::rest::not_found(_url);
	if (bool(_opts["bubble-error"]) && int(_out["status"]) > 399) {
		throw zpt::assertion(_out["payload"]["text"]->ok() ? std::string(_out["payload"]["text"]) : std::string(zpt::status_names[int(_out["status"])]), int(_out["status"]), int(_out["payload"]["code"]), _out["payload"]["assertion_failed"]->ok() ? std::string(_out["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_out["status"])]));
	}
	if (this->has_pending(_envelope)) {
		this->reply(_envelope, _out);
	}
}

auto zpt::RESTEmitter::sync_route(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	assertz(_url.length() != 0, "resource URI must be valid", 400, 0);
	
	if (bool(_opts["mqtt"])) {
		if (bool(_opts["no-envelope"])) {
			this->__server->publish(_url, _envelope);
		}
		else {
			zpt::json _in = _envelope + zpt::json{ 
				"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"]),
				"performative", _method,
				"resource", _url
			};
			this->__server->publish(_url, _in);
		}
		return zpt::undefined;
	}
	
	return this->sync_resolve(_method, _url, _envelope, _opts + zpt::json{ "broker", true });
}

auto zpt::RESTEmitter::sync_resolve(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	zpt::json _return;
	bool _endpoint_found = false;
	bool _method_found = false;

	_envelope = _envelope + zpt::json{ 
		"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"]),
		"performative", _method,
		"resource", _url
	};
	if (!zpt::test::uuid(std::string(_envelope["channel"]))) {
		_envelope << "channel" << zpt::generate::r_uuid();
	}

	std::vector< std::pair<std::regex, zpt::ev::handlers > > _resources;
	if (_method != zpt::ev::Reply) {
		zpt::json _splited = zpt::split(_url, "/");
		std::string _hash;
		for (auto _i : _splited->arr()) {
			_hash += std::string("/") + _i->str();
			auto _found = this->__hashed.find(_hash);
			if (_found != this->__hashed.end()) {
				_resources = _found->second;
				break;
			}
		}
	}
	else {
		auto _found = this->__hashed.find("/___reply___");
		if (_found != this->__hashed.end()) {
			_resources = _found->second;
		}
	}

	for (auto _endpoint : _resources) {
		std::regex _regexp = _endpoint.first;
		if (std::regex_match(_url, _regexp)) {
			_endpoint_found = true;
			if (_endpoint.second[_method] != nullptr) {
				_method_found = true;
				try {
					zpt::json _result;
					this->pending(_envelope,
						[ = ] (zpt::ev::performative _p_performtive, std::string _p_topic, zpt::json _p_result, zpt::ev::emitter _p_emitter) mutable -> void {
							_result << "result" << _p_result;
						}
					);
					_endpoint.second[_method](_method, _url, _envelope, this->self()); // > HASH <
					if (!this->has_pending(_envelope)) {
						this->reply(_envelope, { "status", 204 });
					}
					_result = _result["result"];
					if (_result->ok()) {
						if (bool(_opts["bubble-error"]) && int(_result["status"]) > 399) {
							zlog(std::string("error processing '") + _url + std::string("': ") + std::string(_result["payload"]), zpt::error);
							if (bool(_opts["bubble-error"])) {
								throw zpt::assertion(_result["payload"]["text"]->ok() ? std::string(_result["payload"]["text"]) : std::string(zpt::status_names[int(_result["status"])]), int(_result["status"]), int(_result["payload"]["code"]), _result["payload"]["assertion_failed"]->ok() ? std::string(_result["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_result["status"])]));
							}
						}

						_result << 
						"performative" << zpt::ev::Reply <<
						"headers" << (zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + this->options()["$defaults"]["headers"]["response"] + _result["headers"]);

						_return = _result;
					}
				}
				catch (zpt::assertion& _e) {
					zlog(std::string("error processing '") + _url + std::string("': ") + _e.what() + std::string(", ") + _e.description(), zpt::error);
					if (bool(_opts["bubble-error"])) {
						throw;
					}
					_return = {
						"performative", zpt::ev::Reply,
						"status", _e.status(),
						"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"],
						"payload", {
							"text", _e.what(),
							"assertion_failed", _e.description(),
							"code", _e.code()
						}
					};
				}
				catch(std::exception& _e) {
					zlog(std::string("error processing '") + _url + std::string("': ") + _e.what(), zpt::error);
					if (bool(_opts["bubble-error"])) {
						throw;
					}
					_return = {
						"performative", zpt::ev::Reply,
						"status", 500,
						"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"],
						"payload", {
							"text", _e.what(),
							"code", 1102
						}
					};
				}
			}
		}
	}

	if (_method != zpt::ev::Reply) {
		if (!_endpoint_found) {
			_return = this->sync_resolve_remotely(_method, _url, _envelope, _opts);
		}
		else if (!_method_found) {
			zlog(std::string("error processing '") + _url + std::string("': method'") + zpt::ev::to_str(_method) + std::string("' not registered"), zpt::error);
			if (bool(_opts["bubble-error"])) {
				assertz(_endpoint_found, "the requested performative is not allowed to be used with the requested resource", 405, 1103);
			}
			_return = {
				"performative", zpt::ev::Reply,
				"status", 405,
				"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"],
				"payload", {
					"text", "the requested performative is not allowed to be used with the requested resource",
					"code", 1104
				}
			};
		}
	}	
	return _return;
}

auto zpt::RESTEmitter::sync_resolve_remotely(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	assertz(_url.length() != 0, "resource URI must be valid", 400, 0);
	zpt::json _uri = zpt::uri::parse(_url);

	zpt::json _container;
	if (_uri["scheme"] != zpt::json::string("zpt")) {
		std::string _connect = std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["authority"]);
		_container = { "type", (_uri["scheme"] == zpt::json::string("tcp") ? "req" : _uri["scheme"]), "connect",  _connect };
		_envelope << "resource" << zpt::r_replace(_url, _connect, "");
	}
	else {
		_container = this->lookup(_url);
	}
	if (_container->ok()) {
		short _type = zpt::str2type(_container["type"]->str());
		switch(_type) {
			case ZMQ_ROUTER_DEALER :
			case ZMQ_ROUTER :
			case ZMQ_REP :
			case ZMQ_REQ : {
				zpt::socket_ref _client = this->__poll->add(ZMQ_REQ, _container["connect"]->str(), true);
				_client->send(_envelope);
				zpt::json _out = _client->recv();
				if (!_out["status"]->ok() || ((int) _out["status"]) < 100) {
					_out << "status" << 501 << zpt::json({ "payload", { "text", "required protocol is not implemented", "code", 1105, "assertion_failed", "_out[\"status\"]->ok()" } });
				}
				if (bool(_opts["bubble-error"]) && int(_out["status"]) > 399) {
					throw zpt::assertion(_out["payload"]["text"]->ok() ? std::string(_out["payload"]["text"]) : std::string(zpt::status_names[int(_out["status"])]), int(_out["status"]), int(_out["payload"]["code"]), _out["payload"]["assertion_failed"]->ok() ? std::string(_out["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_out["status"])]));
				}
				this->__poll->remove(_client);
				return _out;
			}
			case ZMQ_PUB_SUB : {
				std::string _connect = _container["connect"]->str();
				zpt::socket_ref _client = this->__poll->add(ZMQ_PUB, _connect.substr(0, _connect.find(",")));
				_client->send(_envelope);
				return zpt::rest::accepted(_url);
			}
			case ZMQ_PUSH : {
				zpt::socket_ref _client = this->__poll->add(ZMQ_PUSH, _container["connect"]->str());
				_client->send(_envelope);
				return zpt::rest::accepted(_url);
			}
			case ZMQ_HTTP_RAW : {
				zpt::socket_ref _client = this->__poll->add(ZMQ_HTTP_RAW, _container["connect"]->str(), true);
				_client->send(_envelope);
				zpt::json _out = _client->recv();
				if (!_out["status"]->ok() || ((int) _out["status"]) < 100) {
					_out << "status" << 501 << zpt::json({ "payload", { "text", "required protocol is not implemented", "code", 1105, "assertion_failed", "_out[\"status\"]->ok()" } });
				}
				if (bool(_opts["bubble-error"]) && int(_out["status"]) > 399) {
					throw zpt::assertion(_out["payload"]["text"]->ok() ? std::string(_out["payload"]["text"]) : std::string(zpt::status_names[int(_out["status"])]), int(_out["status"]), int(_out["payload"]["code"]), _out["payload"]["assertion_failed"]->ok() ? std::string(_out["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_out["status"])]));
				}
				this->__poll->remove(_client);
				return _out;
			}
		}
	}
	zpt::json _out = zpt::rest::not_found(_url);
	if (bool(_opts["bubble-error"]) && int(_out["status"]) > 399) {
		throw zpt::assertion(_out["payload"]["text"]->ok() ? std::string(_out["payload"]["text"]) : std::string(zpt::status_names[int(_out["status"])]), int(_out["status"]), int(_out["payload"]["code"]), _out["payload"]["assertion_failed"]->ok() ? std::string(_out["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_out["status"])]));
	}
	return _out;
}

auto zpt::RESTEmitter::instance() -> zpt::ev::emitter {
	assertz(zpt::rest::__emitter != nullptr, "REST emitter has not been initialized", 500, 1106);
	return zpt::rest::__emitter->self();
}

auto zpt::rest::not_found(std::string _resource) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 404,
		"payload", {
			"text", "resource not found",
			"code", 1101,
			"assertion_failed", "_container->ok()"
		}
	};
}

auto zpt::rest::bad_request() -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", zpt::generate::r_uuid(),
		"status", 400,
		"payload", {
			"text", "bad request",
			"code", 1100,
			"assertion_failed", "_socket >> _req"
		}
	};
}

auto zpt::rest::accepted(std::string _resource) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 202,
		"payload", {
			"text", "request was accepted"
		}
	};
}

auto zpt::rest::no_content(std::string _resource) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 204
	};
}

auto zpt::rest::temporary_redirect(std::string _resource, std::string _target_resource) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 307,
		"headers", {
			"Location", _target_resource
		},
		"payload", {
			"text", "temporarily redirecting you to another location"
		}
	};
}

auto zpt::rest::see_other(std::string _resource, std::string _target_resource) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 303,
		"headers", {
			"Location", _target_resource
		},
		"payload", {
			"text", "temporarily redirecting you to another location"
		}
	};
}

auto zpt::rest::options(std::string _resource, std::string _origin) -> zpt::json {
	return {
		"channel", zpt::generate::r_uuid(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 200,
		"headers", {
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

auto zpt::rest::url_pattern(zpt::json _to_join) -> std::string {
	return std::string("^") + zpt::path::join(_to_join) + std::string("$");
}		
