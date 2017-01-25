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
#include <map>

zpt::RESTEmitter::RESTEmitter(zpt::json _options) : zpt::EventEmitter( _options ), __poll(nullptr), __server(nullptr) {
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

auto zpt::RESTEmitter::on(zpt::ev::performative _event, std::string _regex, zpt::ev::Handler _handler, zpt::json _opts) -> std::string {
	std::regex _url_pattern(_regex);

	std::vector< zpt::ev::Handler> _handlers;
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Get? this->__default_get : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Put ? this->__default_put : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Post ? this->__default_post : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Delete ? this->__default_delete : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Head ? this->__default_head : _handler));
	_handlers.push_back(this->__default_options);
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Patch ? this->__default_patch : _handler));
	_handlers.push_back((_handler == nullptr || _event != zpt::ev::Reply ? this->__default_assync_reply : _handler));

	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	this->__resources.insert(std::make_pair(_uuid.string(), std::make_pair(_url_pattern, _handlers)));
	zlog(string("registered handlers for ") + _regex, zpt::info);
	this->server()->assync_on(_regex, _opts);
	return _uuid.string();
}

auto zpt::RESTEmitter::on(string _regex, std::map< zpt::ev::performative, zpt::ev::Handler > _handler_set, zpt::json _opts) -> std::string {
	std::regex _url_pattern(_regex);

	std::map< zpt::ev::performative, zpt::ev::Handler >::iterator _found;
	vector< zpt::ev::Handler> _handlers;
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Get)) == _handler_set.end() ?  this->__default_get : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Put)) == _handler_set.end() ?  this->__default_put : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Post)) == _handler_set.end() ?  this->__default_post : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Delete)) == _handler_set.end() ?  this->__default_delete : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Head)) == _handler_set.end() ?  this->__default_head : _found->second);
	_handlers.push_back(this->__default_options);
	_handlers.push_back((_found = _handler_set.find(zpt::ev::Patch)) == _handler_set.end() ?  this->__default_patch : _found->second);
	_handlers.push_back(this->__default_assync_reply);

	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	this->__resources.insert(std::make_pair(_uuid.string(), std::make_pair(_url_pattern, _handlers)));
	zlog(string("registered handlers for ") + _regex, zpt::info);
	this->server()->assync_on(_regex, _opts);
	return _uuid.string();
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
	
	std::map< zpt::ev::performative, zpt::ev::Handler >::iterator _found;
	vector< zpt::ev::Handler > _handlers;
	for (short _idx = zpt::ev::Get; _idx != zpt::ev::Reply + 1; _idx++) {
		_handlers.push_back(_handler);
	}
	
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	this->__resources.insert(std::make_pair(_uuid.string(), std::make_pair(_url_pattern, _handlers)));
	zlog(string("registered handlers for ") + _listener->regex(), zpt::info);
	this->server()->assync_on(_listener->regex(), _opts);
	return _uuid.string();
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

auto zpt::RESTEmitter::trigger(zpt::ev::performative _method, std::string _url, zpt::json _envelope) -> zpt::json {
	zpt::json _return;
	bool _endpoint_found = false;
	bool _method_found = false;
	for (auto _i : this->__resources) {
		std::regex _regexp = _i.second.first;
		if (std::regex_match(_url, _regexp)) {
			_endpoint_found = true;
			try {
				if (_i.second.second[_method] != nullptr) {
					_method_found = true;
					zpt::json _result = _i.second.second[_method](_method, _url, _envelope, this->self());
					if (_result->ok()) {
						_result << 
						"performative" << zpt::ev::Reply <<
						"headers" << (zpt::ev::init_reply(_envelope["headers"]["X-Cid"]->str()) + _result["headers"]);
						
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
			}
			catch (zpt::AssertionException& _e) {
				_return = {
					"performative", zpt::ev::Reply,
					"status", _e.status(),
					"headers", zpt::ev::init_reply(_envelope["headers"]["X-Cid"]->str()),
					"payload", {
						"text", _e.what(),
						"assertion_failed", _e.description(),
						"code", _e.code()
					}
				};
				break;
			}
			catch(std::exception& _e) {
				zlog(string("unhandled exception: ") + _e.what(), zpt::emergency);
				throw;
			}
		}
	}
	if (!_endpoint_found) {
		_return = {
			"performative", zpt::ev::Reply,
			"status", 404,
			"headers", zpt::ev::init_reply(_envelope["headers"]["X-Cid"]->str()),
			"payload", {
				"text", "the requrested resource was not found"
			}
		};
	}
	else if (!_method_found) {
		_return = {
			"performative", zpt::ev::Reply,
			"status", 405,
			"headers", zpt::ev::init_reply(_envelope["headers"]["X-Cid"]->str()),
			"payload", {
				"text", "the requrested performative is not allowed to be used with the requested resource"
			}
		};
	}
	
	return _return;
}

auto zpt::RESTEmitter::route(zpt::ev::performative _method, std::string _url, zpt::json _envelope) -> zpt::json {
	zpt::json _in = zpt::json::object() + _envelope;
	_in <<
	"headers" << (zpt::ev::init_request() + _envelope["headers"]) <<
	"channel" << _url <<
	"performative" << _method <<
	"resource" << _url;
	
	for (auto _i : this->__resources) {
		std::regex _regexp = _i.second.first;
		if (std::regex_match(_url, _regexp)) {
			try {
				if (_i.second.second[_method] != nullptr) {
					zpt::json _out = _i.second.second[_method](_method, _url, _in, this->self());
					if (_out->ok()) {
						_out << 
						"performative" << zpt::ev::Reply <<
						"headers" << (zpt::ev::init_reply(((std::string) _in["headers"]["X-Cid"])) + _out["headers"]);
						return _out;
					}
				}
			}
			catch (zpt::AssertionException& _e) {
			       return {
				       "performative", zpt::ev::Reply,
				       "status", _e.status(),
				       "headers", zpt::ev::init_reply((std::string) _in["headers"]["X-Cid"]),
				       "payload", {
					       "text", _e.what(),
					       "assertion_failed", _e.description(),
					       "code", _e.code()
					       }
			       };
			       break;
			}
			catch(std::exception& _e) {
				return {
					"performative", zpt::ev::Reply,
					"status", 500,
					"headers", zpt::ev::init_reply((std::string) _in["headers"]["X-Cid"]),
					"payload", {
						"text", _e.what(),
						"code", 0
					}
				};
			}
		}
	}

	if (this->options()["directory"]->ok()) {
		for (auto _api : this->options()["directory"]->obj()) {
			for (auto _endpoint : _api.second["endpoints"]->arr()) {
				if (_url.find(_endpoint->str()) == 0) {
					short _type = zpt::str2type(_api.second["type"]->str());
					switch(_type) {
						case ZMQ_ROUTER_DEALER :
						case ZMQ_REP :
						case ZMQ_REQ : {
							zpt::socket _client = this->__poll->bind(ZMQ_REQ, _api.second["connect"]->str());
							zpt::json _out = _client->send(_in);
							if (!_out["status"]->ok() || ((int) _out["status"]) < 100) {
								_out << "status" << 501;
							}
							_client->unbind();
							return _out;
						}
						case ZMQ_PUB_SUB : {
							std::string _connect = _api.second["connect"]->str();
							zpt::socket _client = this->__poll->bind(ZMQ_PUB, _connect.substr(0, _connect.find(",")));
							_client->send(_in);
							_client->unbind();
							return zpt::rest::accepted(_url);
						}
						case ZMQ_PUSH : {
							zpt::socket _client = this->__poll->bind(ZMQ_PUSH, _api.second["connect"]->str());
							_client->send(_in);
							_client->unbind();
							return zpt::rest::accepted(_url);
						}
					}
				}
			}
		}
	}
				
	return zpt::rest::not_found(_url);
}

auto zpt::rest::not_found(std::string _resource) -> zpt::json {
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	return {
		"channel", _uuid.string(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 404,
		"payload", {
			"text", "resource not found"
		}
	};
}

auto zpt::rest::accepted(std::string _resource) -> zpt::json {
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	return {
		"channel", _uuid.string(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 202,
		"payload", {
			"text", "request was accepted"
		}
	};
}

auto zpt::rest::no_content(std::string _resource) -> zpt::json {
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	return {
		"channel", _uuid.string(),
		"performative", zpt::ev::Reply,
		"resource", _resource,
		"status", 204,
		"payload", {
			"text", "the required resource produced no content"
		}
	};
}

auto zpt::rest::temporary_redirect(std::string _resource, std::string _target_resource) -> zpt::json {
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	return {
		"channel", _uuid.string(),
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
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	return {
		"channel", _uuid.string(),
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
	uuid _uuid;
	_uuid.make(UUID_MAKE_V1);
	return {
		"channel", _uuid.string(),
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
		
auto zpt::rest::cookies::deserialize(std::string _cookie_header) -> zpt::json {
	zpt::json _splitted = zpt::split(_cookie_header, ";");
	zpt::json _return = zpt::json::object();
	bool _first = true;
	for (auto _part : _splitted->arr()){
		std::string _value = std::string(_part);
		zpt::trim(_value);
		if (_first) {
			_return << "value" << zpt::json::string(_value); 
			_first = false;
		}
		else {
			zpt::json _pair = zpt::split(_value, "=");
			if (_pair->arr()->size() == 2) {
				_return << _pair[0]->str() << _pair[1];
			}
		}
	}
	return _return;
}

auto zpt::rest::cookies::serialize(zpt::json _info) -> std::string {
	std::string _return((std::string) _info["value"]);
	for (auto _field : _info->obj()) {
		if (_field.first == "value") {
			continue;
		}
		_return += std::string("; ") + _field.first + std::string("=") + std::string(_field.second);
	}
	return _return;
}
