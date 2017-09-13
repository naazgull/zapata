/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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
	this->directory()->events(this->self());
	
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

auto zpt::RESTEmitter::shutdown() -> void {
	this->directory()->shutdown(this->uuid());
}

auto zpt::RESTEmitter::on(zpt::ev::performative _event, std::string _regex, zpt::ev::Handler _handler, zpt::json _opts) -> void {
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
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Search ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Notify ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Trace ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Connect ? nullptr : _handler));

	this->directory()->notify(_regex, std::make_tuple(this->options()["zmq"][0] + zpt::json{ "uuid", this->uuid(), "regex", _regex, "performatives", { zpt::ev::to_str(_event), true } }, _handlers, std::regex(_regex)));
	this->server()->subscribe(_regex, _opts);

	ztrace(std::string("registered handlers for ") + _regex);
}

auto zpt::RESTEmitter::on(std::string _regex, std::map< zpt::ev::performative, zpt::ev::Handler > _handler_set, zpt::json _opts) -> void {
	std::regex _url_pattern(_regex);

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
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Search)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Notify)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Trace)) == _handler_set.end() ? nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Connect)) == _handler_set.end() ? nullptr : _found->second);

	zpt::json _performatives = zpt::json::object();
	for (auto _h : _handler_set) {
		_performatives << zpt::ev::to_str(_h.first) << true;
	}

	this->directory()->notify(_regex, std::make_tuple(this->options()["zmq"][0] + zpt::json{ "uuid", this->uuid(), "regex", _regex, "performatives", _performatives }, _handlers, std::regex(_regex)));
	this->server()->subscribe(_regex, _opts);

	ztrace(std::string("registered handlers for ") + _regex);
}

auto zpt::RESTEmitter::on(zpt::ev::listener _listener, zpt::json _opts) -> void {
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
			case zpt::ev::Search :
			case zpt::ev::Notify :
			case zpt::ev::Trace :
			case zpt::ev::Connect : {
				 break;
			}
		}
	};
	
	zpt::json _performatives = zpt::json::object();
	zpt::ev::handlers _handlers;
	for (short _idx = zpt::ev::Get; _idx != zpt::ev::Reply + 1; _idx++) {
		_handlers.push_back(_handler);
		_performatives << zpt::ev::to_str((zpt::ev::performative) _idx) << true;
	}
	
	this->directory()->notify(_listener->regex(), std::make_tuple(this->options()["zmq"][0] + zpt::json{ "uuid", this->uuid(), "regex", _listener->regex(), "performatives", _performatives }, _handlers, std::regex(_listener->regex())));
	this->server()->subscribe(_listener->regex(), _opts);

	ztrace(std::string("registered handlers for ") + _listener->regex());
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
		if (_reply->ok()) {
			auto _callback = _exists->second;
			this->__pending.erase(_exists);
			_callback(zpt::ev::Reply, std::string(_request["resource"]), _reply, this->self());
		}
		else {
			this->__pending.erase(_exists);
		}
	}
}

auto zpt::RESTEmitter::trigger(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts, zpt::ev::handler _callback) -> void {
	this->resolve(_method, _url, _envelope, _opts + zpt::json{ "broker", (this->options()["broker"]->ok() && std::string(this->options()["broker"]) == "true") }, _callback);
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
				"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"] + zpt::json{ "X-Sender", this->uuid() }),
				"performative", _method,
				"resource", _url
			};
			this->__server->publish(_url, _in);
		}
		return;
	}
	if (bool(_opts["upnp"])) {
		this->__server->broadcast(_envelope + zpt::json{ 
				"headers", (_envelope["headers"] + zpt::json{ "X-Sender", this->uuid() }),
				"performative", _method,
				"resource", _url
			}
		);
		return;
	}
	
	this->resolve(_method, _url, _envelope, _opts + zpt::json{ "broker", true }, _callback);
}

auto zpt::RESTEmitter::resolve(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts, zpt::ev::handler _callback) -> void {
	zpt::ev::node _found = this->lookup(_url, _method);	
	zpt::json _container = std::get<0>(_found);
	zpt::ev::handlers _handlers = std::get<1>(_found);

	_envelope = _envelope + zpt::json{ 
		"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"]),
		"performative", _method,
		"resource", (_container["topic"]->ok() ? _container["topic"] : zpt::json::string(_url))
	};
	if (!zpt::test::uuid(std::string(_envelope["channel"]))) {
		_envelope << "channel" << zpt::generate::r_uuid();
	}
	bool _has_callback = false;
	if (_callback != nullptr) {
		if (_opts["bubble-error"]->is_object()) {
			this->pending(_envelope,
				[ _opts, _callback ] (zpt::ev::performative _performative, std::string _topic, zpt::json _reply, zpt::ev::emitter _emitter) mutable -> void {
					if (int(_reply["status"]) > 399) {
						_emitter->reply(_opts["bubble-error"], _reply);
					}
					else {
						_callback(_performative, _topic, _reply, _emitter);
					}
				}
			);
		}
		else {
			this->pending(_envelope, _callback);
		}
		_has_callback = true;
	}
	else if (_opts["bubble-response"]->is_object()) {
		this->pending(_envelope,
			[ _opts ] (zpt::ev::performative _performative, std::string _topic, zpt::json _reply, zpt::ev::emitter _emitter) mutable -> void {
				_emitter->reply(_opts["bubble-response"], _reply);
			}
		);
		_has_callback = true;
	}
	else if (_opts["bubble-error"]->is_object()) {
		this->pending(_envelope,
			[ _opts ] (zpt::ev::performative _performative, std::string _topic, zpt::json _reply, zpt::ev::emitter _emitter) mutable -> void {
				if (int(_reply["status"]) > 399) {
					_emitter->reply(_opts["bubble-error"], _reply);
				}
			}
		);
	}

	if (!_container->is_object() && _handlers.size() == 0) {
		zpt::json _out = zpt::ev::not_found(_url);
		this->reply(_envelope, _out);
		return;
	}
	
	if (_container["uuid"] == zpt::json::string(this->uuid()) && _handlers.size() > _method && _handlers[_method] != nullptr) {		
		try {
			_handlers[_method](_method, _url, _envelope, this->self());
			if (!_has_callback) {
				this->reply(_envelope, { "status", 204 });
			}
			return;
		}
		catch (zpt::assertion& _e) {
			zpt::json _out = zpt::ev::assertion_error(std::string(_envelope["resource"]), _e, zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"] + zpt::json{ "X-Sender", this->uuid() });
			zlog(std::string("error processing '") + _url + std::string("': ") + _e.what() + std::string(", ") + _e.description(), zpt::error);
			this->reply(_envelope, _out);
			return;
		}
		catch(std::exception& _e) {
		 	zpt::json _out = zpt::ev::internal_server_error(std::string(_envelope["resource"]), _e, zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"] + zpt::json{ "X-Sender", this->uuid() });
		 	zlog(std::string("error processing '") + _url + std::string("': ") + _e.what(), zpt::error);
		 	this->reply(_envelope, _out);
		 	return;
		}
	}
	else {
		short _type = zpt::str2type(_container["type"]->str());
		bool _no_answer = false;
		zpt::socket_ref _client;
		switch(_type) {
			case ZMQ_ROUTER_DEALER :
			case ZMQ_ROUTER :
			case ZMQ_DEALER :
			case ZMQ_REP :
			case ZMQ_REQ : {
				_client = this->__poll->add(ZMQ_DEALER, _container["connect"]->str());
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
				break;
			}
			default : {
				zpt::json _out = zpt::ev::unsupported_media_type(_url);
				this->reply(_envelope, _out);
				return;
			}
		}
		this->__poll->poll(_client);
		// this->pending(_envelope,
		//  	[ & ] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) mutable -> void {
		//  		this->__poll->clean_up(_client);
		//  		zdbg(_envelope);
		//  	}
		// );
		_client->send(_envelope);
		if (_no_answer && !_has_callback) {
			this->reply(_envelope, zpt::ev::accepted(_url));
		}
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
				"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"] + zpt::json{ "X-Sender", this->uuid() }),
				"performative", _method,
				"resource", _url
			};
			this->__server->publish(_url, _in);
		}
		return zpt::undefined;
	}
	if (bool(_opts["upnp"])) {
		this->__server->broadcast(_envelope + zpt::json{ 
				"headers", (_envelope["headers"] + zpt::json{ "X-Sender", this->uuid() }),
				"performative", _method,
				"resource", _url
			}
		);
		return zpt::undefined;
	}
	
	return this->sync_resolve(_method, _url, _envelope, _opts + zpt::json{ "broker", true });
}

auto zpt::RESTEmitter::sync_resolve(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	zpt::ev::node _found = this->lookup(_url, _method);	
	zpt::json _container = std::get<0>(_found);
	zpt::ev::handlers _handlers = std::get<1>(_found);

	if (!_container->is_object() && _handlers.size() == 0) {
		zpt::json _out = zpt::ev::not_found(_url);
		if (bool(_opts["bubble-error"]) && int(_out["status"]) > 399) {
			throw zpt::assertion(_out["payload"]["text"]->ok() ? std::string(_out["payload"]["text"]) : std::string(zpt::status_names[int(_out["status"])]), int(_out["status"]), int(_out["payload"]["code"]), _out["payload"]["assertion_failed"]->ok() ? std::string(_out["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_out["status"])]));
		}
		return _out;
	}	

	_envelope = _envelope + zpt::json{ 
		"headers", (zpt::ev::init_request() + this->options()["$defaults"]["headers"]["request"] + _envelope["headers"]),
		"performative", _method,
		"resource", (_container["topic"]->ok() ? _container["topic"] : zpt::json::string(_url))
	};
	if (!zpt::test::uuid(std::string(_envelope["channel"]))) {
		_envelope << "channel" << zpt::generate::r_uuid();
	}

	if (_container["uuid"] == zpt::json::string(this->uuid()) && _handlers.size() > _method && _handlers[_method] != nullptr) {		
		try {
			zpt::json _result;
			this->pending(_envelope,
				[ = ] (zpt::ev::performative _p_performtive, std::string _p_topic, zpt::json _p_result, zpt::ev::emitter _p_emitter) mutable -> void {
					_result << "result" << _p_result;
				}
			);
			_handlers[_method](_method, _url, _envelope, this->self());
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
				"headers" << (zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + this->options()["$defaults"]["headers"]["response"] + _result["headers"] + zpt::json{ "X-Sender", this->uuid() });
				return _result;
			}
			return zpt::ev::no_content(zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + this->options()["$defaults"]["headers"]["response"] + zpt::json{ "X-Sender", this->uuid() });
		}
		catch (zpt::assertion& _e) {
			zlog(std::string("error processing '") + _url + std::string("': ") + _e.what() + std::string(", ") + _e.description(), zpt::error);
			if (bool(_opts["bubble-error"])) {
				throw;
			}
			return zpt::ev::assertion_error(std::string(_envelope["resource"]), _e, zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"] + zpt::json{ "X-Sender", this->uuid() });
		}
		catch(std::exception& _e) {
			zlog(std::string("error processing '") + _url + std::string("': ") + _e.what(), zpt::error);
			if (bool(_opts["bubble-error"])) {
				throw;
			}
			return zpt::ev::internal_server_error(std::string(_envelope["resource"]), _e, zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + this->options()["$defaults"]["headers"]["response"] + zpt::json{ "X-Sender", this->uuid() });
		}
	}
	else {
		short _type = zpt::str2type(_container["type"]->str());
		switch(_type) {
			case ZMQ_ROUTER_DEALER :
			case ZMQ_ROUTER :
			case ZMQ_DEALER :
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
				return zpt::ev::accepted(_url);
			}
			case ZMQ_PUSH : {
				zpt::socket_ref _client = this->__poll->add(ZMQ_PUSH, _container["connect"]->str());
				_client->send(_envelope);
				return zpt::ev::accepted(_url);
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

	return zpt::undefined;
}

auto zpt::RESTEmitter::instance() -> zpt::ev::emitter {
	assertz(zpt::rest::__emitter != nullptr, "REST emitter has not been initialized", 500, 1106);
	return zpt::rest::__emitter->self();
}

auto zpt::rest::url_pattern(zpt::json _to_join) -> std::string {
	return std::string("^") + zpt::path::join(_to_join) + std::string("$");
}		
