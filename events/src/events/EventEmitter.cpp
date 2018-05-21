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

#include <zapata/events/EventEmitter.h>

#define ACCESS_CONTROL_HEADERS                                                                                         \
	"X-Cid,X-Status,X-No-Redirection,X-Redirect-To,Authorization,Accept,Accept-Language,Cache-Control,Connection," \
	"Content-Length,Content-Type,Cookie,Date,Expires,Location,Origin,Server,X-Requested-With,X-Replied-With,"      \
	"Pragma,Cache-Control,E-Tag"

namespace zpt {
namespace ev {
std::string* __default_authorization = nullptr;
}
}

zpt::PollPtr::PollPtr(zpt::Poll* _ptr) : std::shared_ptr<zpt::Poll>(_ptr) {}

zpt::PollPtr::~PollPtr() {}

zpt::Poll::Poll() {}

zpt::Poll::~Poll() {}

zpt::socket_ref::socket_ref() : std::string(), __poll(nullptr) {}

zpt::socket_ref::socket_ref(const zpt::socket_ref& _rhs) : std::string(_rhs.data()), __poll(_rhs.__poll) {}

zpt::socket_ref::socket_ref(std::string _rhs, zpt::poll _poll) : std::string(_rhs), __poll(_poll) {}

auto zpt::socket_ref::poll(zpt::poll _poll) -> void { this->__poll = _poll; }

auto zpt::socket_ref::poll() -> zpt::poll { return this->__poll; }

auto zpt::socket_ref::operator-> () -> zpt::Channel* { return this->__poll->relay(this->data()); }

auto zpt::socket_ref::operator*() -> zpt::Channel* { return this->__poll->relay(this->data()); }

zpt::ChannelFactory::ChannelFactory() {}

zpt::ChannelFactory::~ChannelFactory() {}

zpt::Channel::Channel(std::string _connection, zpt::json _options)
    : __options(_options), __connection(_connection.data()), __poll(nullptr) {
	this->__id.assign(zpt::generate::r_uuid());
}

zpt::Channel::~Channel() {}

auto zpt::Channel::id() -> std::string { return this->__id; }

auto zpt::Channel::options() -> zpt::json { return this->__options; }

auto zpt::Channel::connection() -> std::string { return this->__connection; }

auto zpt::Channel::connection(std::string _connection) -> void { this->__connection.assign(_connection); }

auto zpt::Channel::uri(size_t _idx) -> zpt::json { return this->__uri[_idx]; }

auto zpt::Channel::uri(std::string _uris) -> void {
	this->__uri = zpt::json::array();

	zpt::json _addresses = zpt::split(_uris, ",", true);
	for (auto _address : _addresses->arr()) {
		zpt::json _uri = zpt::uri::parse(std::string(_address));
		if (!_uri["type"]->is_string()) {
			_uri << "type"
			     << ">";
		}
		if (!_uri["port"]->is_string()) {
			_uri << "port"
			     << "*";
		}
		this->__uri << _uri;
	}
}

auto zpt::Channel::detach() -> void {
	if (this->uri()["type"] == zpt::json::string("@")) {
		this->in()->unbind(std::string(this->uri()["scheme"]) + std::string("://") +
				   std::string(this->uri()["domain"]) + std::string(":") +
				   std::string(this->uri()["port"]));
	} else {
		this->in()->disconnect(std::string(this->uri()["scheme"]) + std::string("://") +
				       std::string(this->uri()["domain"]) + std::string(":") +
				       std::string(this->uri()["port"]));
	}
}

auto zpt::Channel::close() -> void {
	this->in()->close();
	this->out()->close();
}

auto zpt::Channel::available() -> bool { return true; }

auto zpt::Channel::loop_iteration() -> void {}

zpt::BridgePtr::BridgePtr(zpt::Bridge* _target) : std::shared_ptr<zpt::Bridge>(_target) {}

zpt::BridgePtr::BridgePtr() : std::shared_ptr<zpt::Bridge>(nullptr) {}

zpt::Bridge::Bridge(zpt::json _options) : __options(_options) {}

zpt::Bridge::~Bridge() {}

auto zpt::Bridge::options() -> zpt::json { return this->__options; }

zpt::EventEmitter::EventEmitter() : __self(this), __keeper(nullptr), __directory(nullptr) {}

zpt::EventEmitter::EventEmitter(zpt::json _options)
    : __options(_options), __self(this), __keeper((new zpt::EventGatekeeper(_options))->self()),
      __directory((new zpt::EventDirectory(_options))->self()), __uuid(zpt::generate::r_uuid()) {}

zpt::EventEmitter::~EventEmitter() {}

auto zpt::EventEmitter::uuid() -> std::string { return this->__uuid; }

auto zpt::EventEmitter::options() -> zpt::json { return this->__options; }

auto zpt::EventEmitter::self() const -> zpt::ev::emitter { return this->__self; }

auto zpt::EventEmitter::unbind() -> void { this->__self.reset(); }

auto zpt::EventEmitter::gatekeeper() -> zpt::ev::gatekeeper { return this->__keeper; }

auto zpt::EventEmitter::gatekeeper(zpt::ev::gatekeeper _gatekeeper) -> void {
	this->__keeper->unbind();
	_gatekeeper->events(this->self());
	this->__keeper = _gatekeeper;
}

auto zpt::EventEmitter::directory() -> zpt::ev::directory { return this->__directory; }

auto zpt::EventEmitter::authorize(std::string _topic, zpt::json _envelope, zpt::json _roles_needed) -> zpt::json {
	return this->__keeper->authorize(_topic, _envelope, _roles_needed);
}

auto zpt::EventEmitter::lookup(std::string _topic, zpt::ev::performative _performative) -> zpt::ev::node {
	return this->__directory->lookup(_topic, _performative);
}

zpt::EventEmitterFactory::EventEmitterFactory() {}

zpt::EventEmitterFactory::~EventEmitterFactory() {}

auto zpt::EventEmitterFactory::enroll(zpt::ev::emitter _emitter) -> void { this->__emitters.push_back(_emitter); }

auto zpt::EventEmitterFactory::unroll(zpt::ev::emitter _emitter) -> void {
	auto _found = std::find_if(this->__emitters.begin(), this->__emitters.end(), [_emitter](zpt::ev::emitter _e) {
		return _e.get() == _emitter.get();
	});
	if (_found != this->__emitters.end()) {
		this->__emitters.erase(_found);
	}
}

auto zpt::EventEmitterFactory::connector(std::string _name, zpt::connector _connector) -> void {
	auto _found = this->__connector.find(_name);
	if (_found == this->__connector.end()) {
		ztrace(std::string("registering connector ") + _name + std::string("@") + _connector->name());
		try {
			_connector->connect();
		} catch (std::exception& _e) {
			zlog(std::string(_name) + std::string(": ") + _e.what(), zpt::error);
			return;
		}
		this->__connector.insert(std::make_pair(_name, _connector));
	}
}

auto zpt::EventEmitterFactory::connector(std::map<std::string, zpt::connector> _connectors) -> void {
	for (auto _connector : _connectors) {
		this->connector(_connector.first, _connector.second);
	}
}

auto zpt::EventEmitterFactory::connector(std::string _name) -> zpt::connector {
	auto _found = this->__connector.find(_name);
	assertz(_found != this->__connector.end(),
		std::string("theres isn't any connector by the name '") + _name + std::string("'"),
		500,
		0);
	return _found->second;
}

auto zpt::EventEmitterFactory::channel(std::string _name, zpt::socket_factory _channel_factory) -> void {
	auto _found = this->__channel.find(_name);
	if (_found == this->__channel.end()) {
		ztrace(std::string("registering channel factory") + _name);
		this->__channel.insert(std::make_pair(_name, _channel_factory));
	}
}

auto zpt::EventEmitterFactory::channel(std::map<std::string, zpt::socket_factory> _channel_factories) -> void {
	for (auto _channel_factory : _channel_factories) {
		this->channel(_channel_factory.first, _channel_factory.second);
	}
}

auto zpt::EventEmitterFactory::channel(std::string _name) -> zpt::socket_factory {
	auto _found = this->__channel.find(_name);
	assertz(_found != this->__channel.end(),
		std::string("theres isn't any channel factory by the name '") + _name + std::string("'"),
		500,
		0);
	return _found->second;
}

auto zpt::EventEmitterFactory::ontology(zpt::ev::ontology _ontology) -> void {
	std::for_each(this->__emitters.begin(), this->__emitters.end(), [_ontology](const zpt::ev::emitter& _e) {
		_e->ontology(_ontology);
	});
	this->__ontology = _ontology;
}

auto zpt::EventEmitterFactory::ontology() -> zpt::ev::ontology { return this->__ontology; }

auto zpt::EventEmitterFactory::credentials() -> zpt::json {
	zpt::json _return = zpt::json::object();
	std::for_each(this->__emitters.begin(), this->__emitters.end(), [&_return](const zpt::ev::emitter& _e) {
		_return = _return | _e->credentials();
	});
	return _return;
}

auto zpt::EventEmitterFactory::trigger(zpt::ev::performative _method,
				       std::string _resource,
				       zpt::json _payload,
				       zpt::json _opts,
				       zpt::ev::handler _callback) -> void {
	std::for_each(this->__emitters.begin(), this->__emitters.end(), [=](const zpt::ev::emitter& _e) {
		_e->trigger(_method, _resource, _payload, _opts, _callback);
	});
}

auto zpt::EventEmitterFactory::route(zpt::ev::performative _method,
				     std::string _resource,
				     zpt::json _payload,
				     zpt::json _opts,
				     zpt::ev::handler _callback) -> void {
	std::for_each(this->__emitters.begin(), this->__emitters.end(), [=](const zpt::ev::emitter& _e) {
		_e->route(_method, _resource, _payload, _opts, _callback);
	});
}

auto zpt::EventEmitterFactory::route(zpt::ev::performative _method,
				     std::string _resource,
				     zpt::json _payload,
				     zpt::ev::handler _callback) -> void {
	std::for_each(this->__emitters.begin(), this->__emitters.end(), [=](const zpt::ev::emitter& _e) {
		_e->route(_method, _resource, _payload, _callback);
	});
}

auto zpt::EventEmitterFactory::hook(zpt::ev::initializer _callback) -> void {
	std::for_each(
	    this->__emitters.begin(), this->__emitters.end(), [=](const zpt::ev::emitter& _e) { _e->hook(_callback); });
}

auto zpt::EventEmitterFactory::reply(zpt::json _request, zpt::json _reply) -> void {
	std::for_each(this->__emitters.begin(), this->__emitters.end(), [=](const zpt::ev::emitter& _e) {
		_e->reply(_request, _reply);
	});
}

auto zpt::EventEmitterFactory::has_pending(zpt::json _envelope) -> bool {
	for (auto _e : this->__emitters) {
		if (_e->has_pending(_envelope)) {
			return true;
		}
	};
	return false;
}

auto zpt::EventEmitterFactory::for_each(zpt::ev::initializer _callback) -> void {
	std::for_each(
	    this->__emitters.begin(), this->__emitters.end(), [=](const zpt::ev::emitter& _e) { _callback(_e); });
}

auto zpt::EventEmitterFactory::shutdown() -> void {
	std::for_each(
	    this->__emitters.begin(), this->__emitters.end(), [](const zpt::ev::emitter& _e) { _e->shutdown(); });
}

auto zpt::EventEmitterFactory::instance() -> zpt::ev::emitter_factory {
	static zpt::ev::emitter_factory _emitter_factory(new zpt::EventEmitterFactory());
	return _emitter_factory;
}

auto zpt::ev::set_default_authorization(std::string _default_authorization) -> void {
	if (zpt::ev::__default_authorization != nullptr) {
		delete zpt::ev::__default_authorization;
	}
	zpt::ev::__default_authorization = new std::string(_default_authorization.data());
}

auto zpt::ev::get_default_authorization() -> std::string {
	if (zpt::ev::__default_authorization != nullptr) {
		return std::string(zpt::ev::__default_authorization->data());
	}
	return "";
}

zpt::EventListener::EventListener(std::string _regex) : __regex(_regex) {}

zpt::EventListener::~EventListener() {}

auto zpt::EventListener::regex() -> std::string { return this->__regex; }

zpt::EventGatekeeper::EventGatekeeper(zpt::json _options) : __options(_options), __self(this) {}

zpt::EventGatekeeper::~EventGatekeeper() {}

auto zpt::EventGatekeeper::options() -> zpt::json { return this->__options; }

auto zpt::EventGatekeeper::unbind() -> void { this->__self.reset(); }

auto zpt::EventGatekeeper::self() const -> zpt::ev::gatekeeper { return this->__self; }

auto zpt::EventGatekeeper::events() -> zpt::ev::emitter { return this->__emitter; }

auto zpt::EventGatekeeper::events(zpt::ev::emitter _emitter) -> void { this->__emitter = _emitter; }

auto zpt::EventGatekeeper::get_credentials(zpt::json _client_id,
					   zpt::json _client_secret,
					   zpt::json _address,
					   zpt::json _grant_type,
					   zpt::json _scope) -> zpt::json {
	return {"access_token", "--blank--"};
}

auto zpt::EventGatekeeper::authorize(std::string _topic, zpt::json _envelope, zpt::json _roles_needed) -> zpt::json {
	return {"client_id", "anyone", "access_token", "--blank--", "roles", zpt::json::array()};
}

zpt::EventDirectory::EventDirectory(zpt::json _options)
    : __options(_options), __self(this),
      __services(
	  new zpt::EventDirectoryGraph("", std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*")))) {}

zpt::EventDirectory::~EventDirectory() {}

auto zpt::EventDirectory::options() -> zpt::json { return this->__options; }

auto zpt::EventDirectory::unbind() -> void { this->__self.reset(); }

auto zpt::EventDirectory::self() const -> zpt::ev::directory { return this->__self; }

auto zpt::EventDirectory::events() -> zpt::ev::emitter { return this->__emitter; }

auto zpt::EventDirectory::events(zpt::ev::emitter _emitter) -> void { this->__emitter = _emitter; }

auto zpt::EventDirectory::lookup(std::string _topic, zpt::ev::performative _performative) -> zpt::ev::node {
	std::lock_guard<std::mutex> _lock(this->__mtx);
	zpt::ev::node _found = this->__services->find(_topic, _performative);
	if (std::get<0>(_found)->is_object()) {
		size_t _next = size_t(std::get<0>(_found)["next"]);
		zpt::json _peers = std::get<0>(_found)["peers"];
		zpt::json _container = _peers[_next];
		if (_next == _peers->arr()->size() - 1) {
			_next = 0;
		} else {
			_next++;
		}
		std::get<0>(_found) << "next" << _next;
		return std::make_tuple(_container, std::get<1>(_found), std::get<2>(_found));
	}
	return _found;
}

auto zpt::EventDirectory::notify(std::string _topic, zpt::ev::node _connection) -> void {
	std::lock_guard<std::mutex> _lock(this->__mtx);
	std::get<0>(_connection) << "connect" << zpt::r_replace(std::string(std::get<0>(_connection)["connect"]),
								"tcp://*:",
								std::string("tcp://127.0.0.1:"));
	this->__services->insert(_topic, _connection);
}

auto zpt::EventDirectory::make_available(std::string _uuid) -> void {
	if (bool(this->__options["discoverable"])) {
		this->__emitter->route(zpt::ev::Notify,
				       "*",
				       {"headers",
					{"MAN",
					 "\"ssdp:discover\"",
					 "MX",
					 "3",
					 "ST",
					 "urn:schemas-upnp-org:container:available",
					 "X-UUID",
					 _uuid,
					 "Location",
					 this->__options["upnp"][0]["connect"]}},
				       {"upnp", true});
	}
}

auto zpt::EventDirectory::shutdown(std::string _uuid) -> void {
	if (bool(this->__options["discoverable"])) {
		this->__emitter->route(zpt::ev::Notify,
				       "*",
				       {"headers",
					{"MAN",
					 "\"ssdp:discover\"",
					 "MX",
					 "3",
					 "ST",
					 "urn:schemas-upnp-org:container:shutdown",
					 "X-UUID",
					 _uuid,
					 "Location",
					 this->__options["upnp"][0]["connect"]}},
				       {"upnp", true});
	}
}

auto zpt::EventDirectory::vanished(std::string _uuid) -> void { this->__services->remove(_uuid); }

auto zpt::EventDirectory::list(std::string _uuid) -> zpt::json { return this->__services->list(_uuid); }

auto zpt::EventDirectory::pretty() -> std::string { return this->__services->pretty(); }

zpt::EventDirectoryGraph::EventDirectoryGraph(std::string _resolver, zpt::ev::node _service)
    : __resolver(_resolver), __service(_service) {}

zpt::EventDirectoryGraph::~EventDirectoryGraph() {}

auto zpt::EventDirectoryGraph::merge(zpt::ev::node _service) -> void {
	auto _lhs = std::get<1>(this->__service);
	auto _rhs = std::get<1>(_service);
	for (short _idx = 0; _idx != short(zpt::ev::Connect) + 1; _idx++) {
		auto _lh_callback = _lhs[_idx];
		auto _rh_callback = _rhs[_idx];

		if (_lh_callback == nullptr && _rh_callback != nullptr) {
			std::get<1>(this->__service)[_idx] = _rh_callback;
			std::get<0>(this->__service)["peers"][0]["performatives"]
			    << zpt::ev::to_str(zpt::ev::performative(_idx)) << true;
		} else if (_lh_callback != nullptr && _rh_callback != nullptr) {
			std::get<1>(this->__service)[_idx] = ([=](zpt::ev::performative _performative,
								  std::string _topic,
								  zpt::json _envelope,
								  zpt::ev::emitter _emitter) mutable -> void {
				_lh_callback(_performative, _topic, _envelope, _emitter);
				_rh_callback(_performative, _topic, _envelope, _emitter);
			});
		}
	}
}

auto zpt::EventDirectoryGraph::insert(std::string _topic, zpt::ev::node _service) -> void {
	zpt::json _topics = zpt::ev::uri::get_simplified_topics(_topic);
	for (auto _topic : _topics->arr()) {
		this->insert(zpt::path::split(std::string(_topic)), _service);
	}
}

auto zpt::EventDirectoryGraph::insert(zpt::json _topic, zpt::ev::node _service) -> void {
	if (_topic->arr()->size() == 0) {
		if (std::get<1>(this->__service).size() != 0) {
			if (std::get<1>(_service).size() != 0) {
				this->merge(_service);
			}
			return;
		}

		zpt::json _containers = std::get<0>(this->__service)["peers"];
		if (_containers->is_array()) {
			zpt::json _new_container = std::get<0>(_service);
			if (_new_container->is_object()) {
				bool _found = false;
				for (auto _c : _containers->arr()) {
					if (_new_container["connect"] == _c["connect"]) {
						_c << "uuid" << _new_container["uuid"];
						_found = true;
						break;
					}
				}
				if (!_found) {
					_containers << _new_container;
				}
			}
		} else {
			zpt::json _s_data = std::get<0>(_service)->clone();
			std::get<0>(_service)->obj()->clear();
			std::get<0>(_service) << "peers" << zpt::json{zpt::array, _s_data} << "next" << 0;
			this->__service = _service;
		}
		return;
	}

	std::string _resolver = std::string(_topic[0]);
	auto _child = this->__children.find(_resolver);
	if (_child == this->__children.end()) {
		this->__children.insert(std::make_pair(
		    _resolver.data(),
		    zpt::ev::graph(new zpt::EventDirectoryGraph(
			_resolver, std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"))))));
		_child = this->__children.find(_resolver);
	}

	_topic->arr()->erase(_topic->arr()->begin());
	_child->second->insert(_topic, _service);
}

auto zpt::EventDirectoryGraph::find(std::string _topic, zpt::ev::performative _performative) -> zpt::ev::node {
	zpt::json _splited = zpt::path::split(_topic);
	if (std::string(_splited[0]).find(":") != std::string::npos) {
		if (!zpt::test::uri(_topic)) {
			return std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
		}
		zpt::json _uri = zpt::uri::parse(_topic);
		std::string _connect =
		    std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["authority"]);
		return std::make_tuple(
		    zpt::json{
			"peers",
			{zpt::array,
			 {"topic",
			  _uri["path"],
			  "type",
			  (_uri["scheme"] == zpt::json::string("tcp") ? zpt::json::string("dealer") : _uri["scheme"]),
			  "connect",
			  _connect,
			  "regex",
			  ".*"}},
			"next",
			0},
		    zpt::ev::handlers(),
		    std::regex(".*"));
	}
	return this->find(_topic, _splited, _performative);
}

auto zpt::EventDirectoryGraph::find(std::string _topic, zpt::json _splited, zpt::ev::performative _performative)
    -> zpt::ev::node {
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
		return std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
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
		} while (_less->arr()->size() != 0);
	}

	return std::make_tuple(zpt::undefined, zpt::ev::handlers(), std::regex(".*"));
}

auto zpt::EventDirectoryGraph::remove(std::string _uuid) -> void {
	zpt::json _containers = std::get<0>(this->__service);

	if (_containers->is_object()) {
		for (size_t _idx = 0; _idx != _containers["peers"]->arr()->size(); _idx++) {
			if (_containers["peers"][_idx]["uuid"] == zpt::json::string(_uuid)) {
				_containers["peers"] >> _idx;
				break;
			}
		}
		if (_containers["peers"]->arr()->size() == 0) {
			std::get<0>(this->__service) = zpt::undefined;
		}
	}

	for (auto _child : this->__children) {
		_child.second->remove(_uuid);
	}
}

auto zpt::EventDirectoryGraph::list(std::string _uuid) -> zpt::json {
	zpt::json _containers = std::get<0>(this->__service);
	zpt::json _return;

	if (_containers->is_object()) {
		_return = zpt::json::array();
		for (auto _container : _containers["peers"]->arr()) {
			if (!_container["performatives"]["REPLY"]->ok() &&
			    (_uuid.length() == 0 || _container["uuid"] == zpt::json::string(_uuid))) {
				_return << _container;
			}
		}
	} else {
		_return = zpt::json::array();
	}

	for (auto _child : this->__children) {
		_return = _return + _child.second->list(_uuid);
	}

	return _return;
}

auto zpt::EventDirectoryGraph::pretty(std::string _tabs, bool _last) -> std::string {
	std::string _return;
	if (this->__resolver != "") {
		_return =
		    (_tabs + (_tabs != "" ? std::string(!_last ? "├─ " : "└─ ") : std::string("─ ")) +
		     this->__resolver +
		     (std::get<0>(this->__service)->is_object()
			  ? std::string(" (") + std::to_string(std::get<0>(this->__service)["peers"]->arr()->size()) +
				std::string(")")
			  : std::string("")) +
		     std::string("\n"));
	}

	size_t _idx = 0;
	for (auto _child : this->__children) {
		_return += _child.second->pretty(
		    (this->__resolver != "" ? _tabs + std::string(!_last ? "│\t" : "\t") : std::string("")),
		    _idx == this->__children.size() - 1);
		_idx++;
	}
	return _return;
}

auto zpt::emitter() -> zpt::ev::emitter_factory { return zpt::EventEmitterFactory::instance(); }

auto zpt::ev::uri::get_simplified_topics(std::string _pattern) -> zpt::json {
	zpt::json _aliases = zpt::split(_pattern, "|");
	zpt::json _topics = zpt::json::array();
	char _op = '\0';
	for (auto _alias : _aliases->arr()) {
		std::string _return;
		short _state = 0;
		bool _regex = false;
		bool _escaped = false;
		for (auto _c : _alias->str()) {
			switch (_c) {
			case '/': {
				if (_state == 0) {
					if (_regex) {
						if (_return.back() != '/') {
							_return.push_back('/');
						}
						_return.push_back(_op);
						_regex = false;
					}
					_return.push_back(_c);
					_op = '\0';
				} else {
					_op = '+';
				}
				break;
			}
			case ')':
			case ']': {
				if (!_escaped) {
					_state--;
				} else {
					_escaped = false;
				}
				_regex = true;
				break;
			}
			case '(':
			case '[': {
				if (!_escaped) {
					_state++;
				} else {
					_escaped = false;
				}
				_regex = true;
				break;
			}
			case '{':
			case '}': {
				_op = '+';
				_regex = true;
				break;
			}
			case '+': {
				if (_op == '\0')
					_op = '*';
				_regex = true;
				break;
			}
			case '*': {
				_op = '*';
				_regex = true;
				break;
			}
			case '$':
			case '^': {
				break;
			}
			case '\\': {
				_escaped = !_escaped;
				break;
			}
			default: {
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

extern "C" auto zpt_events() -> int { return 1; }
