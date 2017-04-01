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

#include <zapata/rest/RESTEmitter.h>
#include <zapata/rest/MutationEmitter.h>
#include <map>

namespace zpt {
	namespace rest {
		emitter* __emitter = nullptr;
	}
}

zpt::RESTEmitter::RESTEmitter(zpt::json _options) : zpt::EventEmitter(_options), __poll(nullptr), __server(nullptr) {
	zsys_init();
	zsys_handler_set(nullptr);
	assertz(zsys_has_curve(), "no security layer for 0mq. Is libcurve (https://github.com/zeromq/libcurve) installed?", 500, 0);
	if (zpt::rest::__emitter != nullptr) {
		delete zpt::rest::__emitter;
		zlog("something is definitely wrong, RESTEmitter already initialized", zpt::emergency);
	}
	zpt::rest::__emitter = this;
	
	if (std::string(this->options()["$mutations"]["enabled"]) == "true") {
		this->mutations((new zpt::RESTMutationEmitter(this->options()))->self());
	}
	else {
		this->mutations((new zpt::DefaultMutationEmitter(this->options()))->self());
	}
	
	this->__default_get = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		assertz(false, "Performative is not accepted for the given resource", 405, 0);
	};
	this->__default_put = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		assertz(false, "Performative is not accepted for the given resource", 405, 0);
	};
	this->__default_post = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		assertz(false, "Performative is not accepted for the given resource", 405, 0);
	};
	this->__default_delete = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		assertz(false, "Performative is not accepted for the given resource", 405, 0);
	};
	this->__default_head = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		assertz(false, "Performative is not accepted for the given resource", 405, 0);
	};
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
	this->__default_patch = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		assertz(false, "Performative is not accepted for the given resource", 405, 0);
	};
	this->__default_assync_reply = [] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _events) -> zpt::json {
		return zpt::undefined;
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
	this->add_by_hash(_regex, _url_pattern, _handlers);

	this->directory()->notify(_regex, this->options()["zmq"]);
	this->server()->subscribe(_regex, _opts);

	zlog(std::string("registered handlers for ") + _regex, zpt::notice);
	return _uuid;
}

auto zpt::RESTEmitter::on(std::string _regex, std::map< zpt::ev::performative, zpt::ev::Handler > _handler_set, zpt::json _opts) -> std::string {
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

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	this->add_by_hash(_regex, _url_pattern, _handlers);

	this->directory()->notify(_regex, this->options()["zmq"]);
	this->server()->subscribe(_regex, _opts);

	zlog(std::string("registered handlers for ") + _regex, zpt::notice);
	return _uuid;
}

auto zpt::RESTEmitter::on(zpt::ev::listener _listener, zpt::json _opts) -> std::string {
	std::regex _url_pattern(_listener->regex());

	zpt::ev::Handler _handler = [ _listener ] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
		switch (_performative) {
			case zpt::ev::Get : {
				return _listener->get(_resource, _envelope, _emitter);
			}
			case zpt::ev::Put : {
				return _listener->put(_resource, _envelope, _emitter);
			}
			case zpt::ev::Post : {
				return _listener->post(_resource, _envelope, _emitter);
			}
			case zpt::ev::Delete : {
				return _listener->del(_resource, _envelope, _emitter);
			}
			case zpt::ev::Head : {
				return _listener->head(_resource, _envelope, _emitter);
			}
			case zpt::ev::Options : {
				return _listener->options(_resource, _envelope, _emitter);
			}
			case zpt::ev::Patch : {
				return _listener->patch(_resource, _envelope, _emitter);
			}
			case zpt::ev::Reply : {
				return _listener->reply(_resource, _envelope, _emitter);
			}
		}
		return zpt::undefined;
	};
	
	zpt::ev::handlers _handlers;
	for (short _idx = zpt::ev::Get; _idx != zpt::ev::Reply + 1; _idx++) {
		_handlers.push_back(_handler);
	}
	
	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	this->add_by_hash(_listener->regex(), _url_pattern, _handlers);

	this->directory()->notify(_listener->regex(), this->options()["zmq"]);
	this->server()->subscribe(_listener->regex(), _opts);

	zlog(std::string("registered handlers for ") + _listener->regex(), zpt::notice);
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

auto zpt::RESTEmitter::trigger(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	zpt::json _return;
	bool _endpoint_found = false;
	bool _method_found = false;

	zpt::json _splited = zpt::split(_url, "/");
	std::vector< std::pair<std::regex, zpt::ev::handlers > > _resources;
	std::string _hash;
	for (auto _i : _splited->arr()) {
		_hash += std::string("/") + _i->str();
		auto _found = this->__hashed.find(_hash);
		if (_found != this->__hashed.end()) {
			_resources = _found->second;
			break;
		}
	}

	// for (auto _i : this->__resources) {
	// 	auto _endpoint = _i.second;
	for (auto _endpoint : _resources) {
		std::regex _regexp = _endpoint.first;
		if (std::regex_match(_url, _regexp)) {
			_endpoint_found = true;
			if (_endpoint.second[_method] != nullptr) {
				_method_found = true;
				try {
					zpt::json _result = _endpoint.second[_method](_method, _url, _envelope, this->self()); // > HASH <
					if (_result->ok()) {
						if (int(_result["status"]) > 399) {
							zlog(std::string("error processing '") + _url + std::string("': ") + std::string(_result["payload"]), zpt::error);
						}
						_result << 
						"performative" << zpt::ev::Reply <<
						"headers" << (zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) + _result["headers"]);
						
						if (_envelope["headers"]["X-No-Redirection"]->ok() && std::string(_envelope["headers"]["X-No-Redirection"]) == "true") {
							if (((int) _result["status"]) > 299 && ((int) _result["status"]) < 400) {
								_result << "status" << 200;
								_result["headers"] << "X-Redirect-To" << _result["headers"]["Location"];
							}
						}

						if (_envelope["headers"]["Origin"]->ok()) {
							_result["headers"] <<
							"Access-Control-Allow-Origin" << _envelope["headers"]["Origin"] <<
							"Access-Control-Allow-Credentials" << "true" <<
							"Access-Control-Allow-Methods" << "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY" <<
							"Access-Control-Allow-Headers" << REST_ACCESS_CONTROL_HEADERS <<
							"Access-Control-Expose-Headers" << REST_ACCESS_CONTROL_HEADERS <<
							"Access-Control-Max-Age" << "1728000";
						}
						
						_return = _result;
					}
				}
				catch (zpt::assertion& _e) {
					zlog(std::string("error processing '") + _url + std::string("': ") + _e.what() + std::string(", ") + _e.description(), zpt::error);
					_return = {
						"performative", zpt::ev::Reply,
						"status", _e.status(),
						"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])),
						"payload", {
							"text", _e.what(),
							"assertion_failed", _e.description(),
							"code", _e.code()
						}
					};
				}
				catch(std::exception& _e) {
					zlog(std::string("error processing '") + _url + std::string("': ") + _e.what(), zpt::error);
					_return = {
						"performative", zpt::ev::Reply,
						"status", 500,
						"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])),
						"payload", {
							"text", _e.what(),
							"code", 0
						}
					};
				}
			}
		}
	}
	if (!_endpoint_found) {
		zlog(std::string("error processing '") + _url + std::string("': listener not found"), zpt::error);
		_return = {
			"performative", zpt::ev::Reply,
			"status", 404,
			"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])),
			"payload", {
				"text", "the requested resource was not found"
			}
		};
	}
	else if (!_method_found) {
		zlog(std::string("error processing '") + _url + std::string("': method'") + zpt::ev::to_str(_method) + std::string("' not registered"), zpt::error);
		_return = {
			"performative", zpt::ev::Reply,
			"status", 405,
			"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])),
			"payload", {
				"text", "the requested performative is not allowed to be used with the requested resource"
			}
		};
	}
	
	return _return;
}

auto zpt::RESTEmitter::route(zpt::ev::performative _method, std::string _url, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	assertz(_url.length() != 0, "resource URI must be valid", 400, 0);
	
	zpt::json _in = zpt::json::object() + _envelope;
	_in <<
	"headers" << (zpt::ev::init_request() + _envelope["headers"]) <<
	"channel" << _url <<
	"performative" << _method <<
	"resource" << _url;

	if (bool(_opts["mqtt"])) {
		if (bool(_opts["no-envelope"])) {
			this->__server->publish(_url, _envelope);
		}
		else {
			this->__server->publish(_url, _in);
		}
		return zpt::undefined;
	}
	
	 zpt::json _splited = zpt::split(_url, "/");
	 std::vector< std::pair<std::regex, zpt::ev::handlers > > _resources;
	 std::string _hash;
	 for (auto _i : _splited->arr()) {
	 	_hash += std::string("/") + _i->str();
	 	auto _found = this->__hashed.find(_hash);
	 	if (_found != this->__hashed.end()) {
	 		_resources = _found->second;
	 		break;
	 	}
	 }

	 // for (auto _i : this->__resources) {
	 // 	auto _endpoint = _i.second;
	 for (auto _endpoint : _resources) {
		std::regex _regexp = _endpoint.first;
		if (std::regex_match(_url, _regexp)) {
			try {
				if (_endpoint.second[_method] != nullptr) {
					ztrace(std::string("") + zpt::ev::to_str(_method) + std::string(" ") + _url + std::string(" | @self"));
					zverbose(zpt::json::pretty(_in));
					zpt::json _out = _endpoint.second[_method](_method, _url, _in, this->self());
					if (_out->ok()) {
						if (bool(_opts["bubble-error"]) && int(_out["status"]) > 399) {
							throw zpt::assertion(_out["payload"]["text"]->ok() ? std::string(_out["payload"]["text"]) : std::string(zpt::status_names[int(_out["status"])]), int(_out["status"]), int(_out["payload"]["code"]), _out["payload"]["assertion_failed"]->ok() ? std::string(_out["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_out["status"])]));
						}						
						_out << 
						"performative" << zpt::ev::Reply <<
						"headers" << (zpt::ev::init_reply(std::string(_in["headers"]["X-Cid"])) + _out["headers"]);
						ztrace(std::string("REPLY ") + _url + std::string(" ")  + std::string(_out["stauts"]) + std::string(" | @self"));
						zverbose(zpt::json::pretty(_out));
						return _out;
					}
				}
			}
			catch (zpt::assertion& _e) {
				if (bool(_opts["bubble-error"])) {
					throw;
				}
				zpt::json _out = {
				       "performative", zpt::ev::Reply,
				       "status", _e.status(),
				       "headers", zpt::ev::init_reply(std::string(_in["headers"]["X-Cid"])),
				       "payload", {
					       "text", _e.what(),
					       "assertion_failed", _e.description(),
					       "code", _e.code()
				       }
			       };
				ztrace(std::string("REPLY ") + _url + std::string(" ")  + std::to_string(_e.status()) + std::string(" | @self"));
				zverbose(zpt::json::pretty(_out));
				return _out;
			}
			catch(std::exception& _e) {
				if (bool(_opts["bubble-error"])) {
					throw zpt::assertion(_e.what(), 500, 0, _e.what());
				}						
				zpt::json _out = {
					"performative", zpt::ev::Reply,
					"status", 500,
					"headers", zpt::ev::init_reply(std::string(_in["headers"]["X-Cid"])),
					"payload", {
						"text", _e.what(),
						"code", 0
					}
				};
				ztrace(std::string("REPLY ") + _url + std::string(" 500 | @self"));
				zverbose(zpt::json::pretty(_out));
				return _out;
			}
		}
	}

	zpt::json _container = this->lookup(_url);
	if (_container->ok()) {
		short _type = zpt::str2type(_container["type"]->str());
		switch(_type) {
			case ZMQ_ROUTER_DEALER :
			case ZMQ_REP :
			case ZMQ_REQ : {
				zpt::socket _client = this->__poll->bind(ZMQ_REQ, _container["connect"]->str());
				zpt::json _out = _client->send(_in);
				if (!_out["status"]->ok() || ((int) _out["status"]) < 100) {
					_out << "status" << 501 << zpt::json({ "payload", { "text", "required protocol is not implemented", "code", 0, "assertion_failed", "_out[\"status\"]->ok()" } });
				}
				_client->unbind();
				if (bool(_opts["bubble-error"]) && int(_out["status"]) > 399) {
					throw zpt::assertion(_out["payload"]["text"]->ok() ? std::string(_out["payload"]["text"]) : std::string(zpt::status_names[int(_out["status"])]), int(_out["status"]), int(_out["payload"]["code"]), _out["payload"]["assertion_failed"]->ok() ? std::string(_out["payload"]["assertion_failed"]) : std::string(zpt::status_names[int(_out["status"])]));
				}						
				return _out;
			}
			case ZMQ_PUB_SUB : {
				std::string _connect = _container["connect"]->str();
				zpt::socket _client = this->__poll->bind(ZMQ_PUB, _connect.substr(0, _connect.find(",")));
				_client->send(_in);
				_client->unbind();
				return zpt::rest::accepted(_url);
			}
			case ZMQ_PUSH : {
				zpt::socket _client = this->__poll->bind(ZMQ_PUSH, _container["connect"]->str());
				_client->send(_in);
				_client->unbind();
				return zpt::rest::accepted(_url);
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
	assertz(zpt::rest::__emitter != nullptr, "REST emitter has not been initialized", 500, 0);
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
			"code", 0,
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
			"code", 0,
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
		"status", 204,
		"payload", {
			"text", "the required resource produced no content"
		}
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
