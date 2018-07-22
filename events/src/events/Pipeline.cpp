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

#include <dlfcn.h>
#include <sys/sem.h>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <systemd/sd-daemon.h>
#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/events/Pipeline.h>
#include <csignal>

namespace zpt {
pid_t root = 0;
bool interrupted = false;
std::atomic<bool> zpt::pipeline::__ready{false};
}

auto zpt::options(int argc, char* argv[]) -> zpt::json& {
	zpt::log_fd = &std::cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new string(argv[0]);

	zpt::json _args = zpt::conf::getopt(argc, argv);

	assertz(
		_args["c"]->ok(), "unable to start client: a valid configuration file must be provided", zpt::emergency, 500);

	short _log_level = (_args["l"]->ok() ? int(_args["l"][0]) : -1);
	std::string _conf_file = std::string(_args["c"][0]);
	zpt::log_format = (bool(_args["r"]) ? 0 : (bool(_args["j"]) ? 2 : 1));

	zpt::json _ptr;
	std::ifstream _in;
	_in.open(_conf_file.data());
	assertz(_in.is_open(), "unable to start: a valid configuration file must be provided", zpt::emergency, 500);
	_in >> _ptr;

	zpt::log_lvl = _log_level;
	return zpt::options(_ptr);
}

auto zpt::options(zpt::json _options) -> zpt::json& {
	static zpt::json _global_options;
	if (_global_options == zpt::undefined && _options != undefined) {
		_global_options = _options;
	}
	return _global_options;
}

// zpt::channel_ref::channel_ref() : std::string(), __poll(nullptr) {}

// zpt::channel_ref::channel_ref(const zpt::channel_ref& _rhs) : std::string(_rhs.data()), __poll(_rhs.__poll) {}

// zpt::channel_ref::channel_ref(std::string _rhs, zpt::poll _poll) : std::string(_rhs), __poll(_poll) {}

// auto zpt::channel_ref::poll(zpt::poll _poll) -> void { this->__poll = _poll; }

// auto zpt::channel_ref::poll() -> zpt::poll { return this->__poll; }

// auto zpt::channel_ref::operator-> () -> zpt::channel* { return this->__poll->relay(this->data()); }

// auto zpt::channel_ref::operator*() -> zpt::channel* { return this->__poll->relay(this->data()); }

zpt::channel_factory::channel_factory(std::string _protocol) : __protocol(_protocol) {}

zpt::channel_factory::channel_factory(const zpt::channel_factory& _rhs) { (*this) = _rhs; }

zpt::channel_factory::channel_factory(zpt::channel_factory&& _rhs) { (*this) = _rhs; }

zpt::channel_factory::~channel_factory() {}

auto zpt::channel_factory::protocol() -> std::string { return this->__protocol; }

auto zpt::channel_factory::operator=(const zpt::channel_factory& _rhs) -> zpt::channel_factory& {
	this->__protocol = _rhs.__protocol;
	return (*this);
}

auto zpt::channel_factory::operator=(zpt::channel_factory&& _rhs) -> zpt::channel_factory& {
	this->__protocol = _rhs.__protocol;
	_rhs.__protocol = "";
	return (*this);
}

zpt::abstract_channel::abstract_channel(std::string _connection, zpt::json _options)
    : __options(_options), __connection(_connection.data()) {
	this->__id.assign(zpt::generate::r_uuid());
}

zpt::abstract_channel::abstract_channel(const zpt::abstract_channel& _rhs) { (*this) = _rhs; }

zpt::abstract_channel::abstract_channel(zpt::abstract_channel&& _rhs) { (*this) = _rhs; }

zpt::abstract_channel::~abstract_channel() {}

auto zpt::abstract_channel::id() -> std::string { return this->__id; }

auto zpt::abstract_channel::options() -> zpt::json { return this->__options; }

auto zpt::abstract_channel::connection() -> std::string { return this->__connection; }

auto zpt::abstract_channel::connection(std::string _connection) -> void { this->__connection.assign(_connection); }

auto zpt::abstract_channel::uri(size_t _idx) -> zpt::json { return this->__uri[_idx]; }

auto zpt::abstract_channel::uri(std::string _uris) -> void {
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

auto zpt::abstract_channel::detach() -> void {
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

auto zpt::abstract_channel::close() -> void {
	this->in()->close();
	this->out()->close();
}

auto zpt::abstract_channel::available() -> bool { return true; }

auto zpt::abstract_channel::loop_iteration() -> void {}

auto zpt::abstract_channel::operator=(const zpt::abstract_channel& _rhs) -> zpt::abstract_channel& {
	this->__options = _rhs.__options;
	this->__connection = _rhs.__connection;
 	this->__id = _rhs.__id;
 	this->__uri = _rhs.__uri;
	return (*this);
}

auto zpt::abstract_channel::operator=(zpt::abstract_channel&& _rhs) -> zpt::abstract_channel& {
	this->__options = _rhs.__options;
	this->__connection = _rhs.__connection;
 	this->__id = _rhs.__id;
 	this->__uri = _rhs.__uri;

	_rhs.__options = zpt::undefined;
	_rhs.__connection = "";
	_rhs.__id = "";
	_rhs.__uri = zpt::undefined;
	return (*this);
}

zpt::stage::stage(zpt::json _initial, zpt::channel _channel) : __current(_initial), __channel(_channel) {}

zpt::stage::stage(const zpt::stage& _rhs) { (*this) = _rhs; }

zpt::stage::stage(zpt::stage&& _rhs) { (*this) = _rhs; }

zpt::stage::~stage() {}

auto zpt::stage::operator=(const zpt::stage& _rhs) -> zpt::stage& {
	this->__current = _rhs.__current;
	this->__channel = _rhs.__channel;
	return (*this);
}

auto zpt::stage::operator=(zpt::stage&& _rhs) -> zpt::stage& {
	this->__current = std::move(_rhs.__current);
	this->__channel = std::move(_rhs.__channel);
	return (*this);
}

auto zpt::stage::forward(zpt::json _message) -> void {}

auto zpt::stage::reply(zpt::json _reply) -> void {}

auto zpt::stage::route(zpt::json _request) -> void {}

zpt::pipeline::pipeline(std::string _name, zpt::json _options)
    : __name(_name), __uuid(zpt::generate::r_uuid()), __options(_options), __stage(0) {}

zpt::pipeline::pipeline(const zpt::pipeline& _rhs) { (*this) = _rhs; }

zpt::pipeline::pipeline(zpt::pipeline&& _rhs) { (*this) = _rhs; }

zpt::pipeline::~pipeline() {}

auto zpt::pipeline::init(std::string _name, zpt::json _options) -> void {
	this->__name = _name;
	this->__options = _options;
	if (not zpt::pipeline::__ready.test_and_set()) {
		try {
			if (this->__options["modules"]->ok()) {
				for (auto _i : this->__options["modules"]->arr()) {
					std::string _lib_file("lib");
					_lib_file.append((std::string)_i);
					_lib_file.append(".so");

					if (_lib_file.length() > 6) {
						zlog(std::string("loading module '") + _lib_file + std::string("'"),
						     zpt::notice);
						void* hndl = dlopen(_lib_file.data(), RTLD_NOW);
						if (hndl == nullptr) {
							zlog(std::string(dlerror()), zpt::error);
						} else {
							void (*_populate)();
							_populate = (void (*)())dlsym(hndl, "_zpt_load_");
							_populate();
						}
					}
				}
			}
			for (auto _callback : std::get<this->__stage>(this->__stages)) {
				_callback.second(_options, *this);
			}
			this->__stage = 1;
		} catch (zpt::assertion& _e) {
			zlog(_e.what() + std::string(" | ") + _e.description(), zpt::emergency);
			zlog(std::string("\n") + _e.backtrace(), zpt::trace);
		} catch (std::exception& _e) {
			zlog(_e.what(), zpt::emergency);
		}
	} else {
		this->__stage = 1;
	}
}

auto zpt::pipeline::name() -> std::string { return this->__name; }

auto zpt::pipeline::uuid() -> std::string { return this->__uuid; }

auto zpt::pipeline::options() -> zpt::json { return this->__options; }

auto zpt::pipeline::add(zpt::channel_factory& _factory) -> void {
	this->__factories.insert(std::make_pair(_factory.protocol(), _factory));
}

auto zpt::pipeline::add(zpt::initializer _callback) -> void { std::get<0>(this->__stages).push_back(_callback); }

auto zpt::pipeline::add(zpt::receiver _callback) -> void { std::get<1>(this->__stages).push_back(_callback); }

auto zpt::pipeline::add(zpt::request_transformer _callback) -> void {
	std::get<2>(this->__stages).push_back(_callback);
}

auto zpt::pipeline::add(zpt::replier _callback) -> void { std::get<3>(this->__stages).push_back(_callback); }

auto zpt::pipeline::add(zpt::reply_transformer _callback) -> void { std::get<4>(this->__stages).push_back(_callback); }

auto zpt::pipeline::operator=(const zpt::pipeline& _rhs) -> zpt::pipeline& {
	this->__name = _rhs.__name;
	this->__uuid = _rhs.__uuid;
	this->__options = _rhs.__options;
	this->__ready = _rhs.__ready;
	this->__stage = _rhs.__stage;
	this->__stages = _rhs.stages;
}

auto zpt::pipeline::operator=(zpt::pipeline&& _rhs) -> zpt::pipeline& {
	this->__name = _rhs.__name;
	this->__uuid = _rhs.__uuid;
	this->__options = _rhs.__options;
	this->__ready = _rhs.__ready;
	this->__stage = _rhs.__stage;
	this->__stages = std::move(_rhs.__stages);

	_rhs.__name = "";
	_rhs.__uuid = "";
	_rhs.__options = zpt::undefined;
	_rhs.__ready = false;
	_rhs.__stage = 0;
}

auto feed(zpt::json _request, zpt::channel _channel) - void {}

auto zpt::pipeline::forward(zpt::json _msg) -> void {}

auto zpt::pipeline::reply(zpt::json _reply) -> void {}

void zpt::pipeline::start() {
	try {
		if (bool(this->__options["discoverable"])) {
			this->notify_peers();
		}

		if (this->__options["rest"]["credentials"]["client_id"]->is_string() &&
		    this->__options["rest"]["credentials"]["client_secret"]->is_string() &&
		    this->__options["rest"]["credentials"]["server"]->is_string() &&
		    this->__options["rest"]["credentials"]["grant_type"]->is_string()) {
			zlog(std::string("going to retrieve credentials ") +
				 std::string(this->__options["rest"]["credentials"]["client_id"]) + std::string(" @ ") +
				 std::string(this->__options["rest"]["credentials"]["server"]),
			     zpt::info);
			zpt::json _credentials = this->__emitter->gatekeeper()->get_credentials(
			    this->__options["rest"]["credentials"]["client_id"],
			    this->__options["rest"]["credentials"]["client_secret"],
			    this->__options["rest"]["credentials"]["server"],
			    this->__options["rest"]["credentials"]["grant_type"],
			    this->__options["rest"]["credentials"]["scope"]);
			this->credentials(_credentials);
		}

		for (auto _callback : this->__initializers) {
			_callback(this->__emitter);
		}

		std::string _NAME(this->__name.data());
		std::transform(_NAME.begin(), _NAME.end(), _NAME.begin(), ::toupper);
		zlog(std::string("loaded *") + _NAME + std::string("*"), zpt::notice);

		sd_notify(0, "READY=1");

		this->__poll->loop();
	} catch (zpt::InterruptedException& e) {
		return;
	}
}

auto zpt::pipeline::instance() -> zpt::pipeline {
	zpt::pipeline _pipeline;
	zpt::json _options = zpt::options();
	_pipeline.init(_options["name"]->str(), _options);
	return _pipeline;
}

// auto zpt::pipeline::publish(std::string _topic, zpt::json _payload) -> void {
// 	ztrace(std::string("> PUBLISH ") + _topic);
// 	zverbose(std::string("\nPUBLISH \033[1;35m") + _topic + std::string("\033[0m MQTT/3.1\n\n") +
// 		 zpt::json::pretty(_payload));
// 	this->__mqtt->publish(_topic, _payload);
// }

// auto zpt::pipeline::subscribe(std::string _topic, zpt::json _opts) -> void {
// 	if (bool(_opts["mqtt"])) {
// 		zpt::json _topics = zpt::rest::uri::get_simplified_topics(_topic);
// 		for (auto _t : _topics->arr()) {
// 			this->__mqtt->subscribe(_t->str());
// 		}
// 	}
// }

// auto zpt::pipeline::broadcast(zpt::json _envelope) -> void {
// 	if (bool(this->options()["discoverable"])) {
// 		this->__upnp->send(_envelope);
// 	}
// }

// auto zpt::pipeline::notify_peers() -> void { // UPnP service discovery
// 	this->__emitter->on(
// 	    "([/]*)\\*",
// 	    {{zpt::ev::Notify,
// 	      [](zpt::performative _performative,
// 		 std::string _topic,
// 		 zpt::json _envelope,
// 		 zpt::ev::emitter _emitter) -> void {
// 		      if (_envelope["headers"]["X-Sender"] == zpt::json::string(_emitter->uuid())) {
// 			      return;
// 		      }
// 		      if (_envelope["headers"]["ST"]->is_string() && _envelope["headers"]["Location"]->is_string() &&
// 			  std::string(_envelope["headers"]["ST"]) == "urn:schemas-upnp-org:container:available") {
// 			      _emitter->route(
// 				  zpt::ev::Search,
// 				  std::string(_envelope["headers"]["Location"]) + std::string("/*"),
// 				  {"headers",
// 				   {"MAN", "\"ssdp:discover\"", "MX", "3", "ST", "urn:schemas-upnp-org:service:*"}},
// 				  [=](zpt::performative _pfv1,
// 				      std::string _tpc1,
// 				      zpt::json _r_services,
// 				      zpt::ev::emitter _emitter) mutable -> void {
// 					  if (_r_services["status"] != zpt::json::integer(200)) {
// 						  return;
// 					  }

// 					  for (auto _service : _r_services["payload"]["elements"]->arr()) {
// 						  _emitter->directory()->notify(
// 						      std::string(_service["regex"]),
// 						      std::make_tuple(_service,
// 								      zpt::ev::handlers(),
// 								      std::regex(std::string(_service["regex"]))));
// 					  }
// 					  // zdbg(_emitter->directory()->pretty());
// 				  });
// 			      zpt::json _services = _emitter->directory()->list(_emitter->uuid());
// 			      // zdbg(zpt::json::pretty(_services));
// 			      if (_services->is_array() && _services->arr()->size() != 0) {
// 				      _emitter->route(zpt::ev::Notify,
// 						      std::string(_envelope["headers"]["Location"]) + std::string("/*"),
// 						      {"headers",
// 						       {"MAN",
// 							"\"ssdp:discover\"",
// 							"MX",
// 							"3",
// 							"ST",
// 							"urn:schemas-upnp-org:service:*",
// 							"Location",
// 							_emitter->options()["zmq"][0]["connect"]},
// 						       "payload",
// 						       {"size", _services->arr()->size(), "elements", _services}});
// 			      }

// 			      // std::cout << _emitter->directory()->pretty() << endl << flush;
// 			      return;
// 		      }
// 		      if (_envelope["headers"]["ST"]->is_string() && _envelope["headers"]["Location"]->is_string() &&
// 			  std::string(_envelope["headers"]["ST"]) == "urn:schemas-upnp-org:container:shutdown") {
// 			      _emitter->directory()->vanished(std::string(_envelope["headers"]["X-UUID"]));
// 			      ((zpt::RESTEmitter*)_emitter.get())
// 				  ->server()
// 				  ->poll()
// 				  ->vanished(std::string(_envelope["headers"]["Location"]));
// 			      // std::cout << _emitter->directory()->pretty() << endl << flush;
// 			      return;
// 		      }
// 		      if (_envelope["headers"]["ST"]->is_string() && _envelope["headers"]["Location"]->is_string() &&
// 			  std::string(_envelope["headers"]["ST"]) == "urn:schemas-upnp-org:service:*") {
// 			      for (auto _service : _envelope["payload"]["elements"]->arr()) {
// 				      _emitter->directory()->notify(
// 					  std::string(_service["regex"]),
// 					  std::make_tuple(_service,
// 							  zpt::ev::handlers(),
// 							  std::regex(std::string(_service["regex"]))));
// 			      }
// 			      // zdbg(_emitter->directory()->pretty());
// 			      return;
// 		      }
// 		      if (_envelope["payload"]["connect"]->ok()) {
// 			      // zdbg(zpt::ev::pretty(_envelope));
// 			      _emitter->directory()->notify(
// 				  std::string(_envelope["payload"]["regex"]),
// 				  std::make_tuple(_envelope["payload"],
// 						  zpt::ev::handlers(),
// 						  std::regex(std::string(_envelope["payload"]["regex"]))));
// 			      return;
// 		      }
// 	      }},
// 	     {zpt::ev::Search,
// 	      [](zpt::performative _performative,
// 		 std::string _topic,
// 		 zpt::json _envelope,
// 		 zpt::ev::emitter _emitter) -> void {
// 		      if (_envelope["headers"]["X-Sender"] == zpt::json::string(_emitter->uuid())) {
// 			      return;
// 		      }

// 		      std::string _search_term =
// 			  zpt::r_replace(std::string(_envelope["headers"]["ST"]), "urn:schemas-upnp-org:service:", "");
// 		      if (_search_term == "*") {
// 			      zpt::json _services = _emitter->directory()->list(_emitter->uuid());
// 			      // zdbg(zpt::json::pretty(_services));
// 			      if (_services->is_array() && _services->arr()->size() != 0) {
// 				      _emitter->reply(_envelope,
// 						      {"status",
// 						       200,
// 						       "payload",
// 						       {"size", _services->arr()->size(), "elements", _services}});
// 			      } else {
// 				      _emitter->reply(_envelope, {"status", 204});
// 			      }
// 		      } else {
// 			      zpt::json _found =
// 				  std::get<0>(_emitter->directory()->lookup(_search_term, zpt::ev::Connect));
// 			      if (_found["uuid"] == zpt::json::string(_emitter->uuid())) {
// 				      _emitter->reply(_envelope, {"status", 200, "payload", _found});
// 			      } else {
// 				      _emitter->reply(_envelope, {"status", 204});
// 			      }
// 		      }
// 	      }}},
// 	    {"upnp", true, "0mq", true});

// 	zpt::rest::__emitter->make_available();
// }

// auto zpt::pipe::shutdown(int _signal) -> void {
// 	if (zpt::pipe::interrupted) {
// 		exit(0);
// 	}
// 	zpt::emitter()->shutdown();
// 	zpt::pipe::interrupted = true;
// }

// auto zpt::pipe::terminate(int _signal) -> void {
// 	if (zpt::pipe::interrupted) {
// 		exit(0);
// 	}
// 	zpt::pipe::interrupted = true;
// }

// std::string zpt::rest::scopes::serialize(zpt::json _info) {
// 	assertz(_info->type() == zpt::JSObject && _info->obj()->size() != 0,
// 		"scope serialization failed: required at least one scope",
// 		412,
// 		0);
// 	std::string _scopes;
// 	for (auto _field : _info->obj()) {
// 		if (_scopes.length() != 0) {
// 			_scopes += std::string(",");
// 		}
// 		_scopes += _field.first + std::string("{") + ((std::string)_field.second) + std::string("}");
// 	}
// 	return _scopes;
// }

// zpt::json zpt::rest::scopes::deserialize(std::string _scope) {
// 	zpt::json _return = zpt::json::object();

// 	zpt::json _splited = zpt::split(_scope, ",");
// 	for (auto _part : _splited->arr()) {
// 		zpt::json _pair = zpt::split(_part->str(), "{");
// 		_return << _pair[0]->str() << _pair[1]->str().substr(0, _pair[1]->str().length() - 1);
// 	}

// 	return _return;
// }

// bool zpt::rest::scopes::has_permission(std::string _scope, std::string _ns, std::string _permissions) {
// 	return zpt::rest::scopes::has_permission(zpt::rest::scopes::deserialize(_scope), _ns, _permissions);
// }

// bool zpt::rest::scopes::has_permission(zpt::json _scope, std::string _ns, std::string _needed) {
// 	if (_scope["all"]->ok()) {
// 		std::string _difference;
// 		std::string _granted(_scope["all"]->str());
// 		std::sort(_needed.begin(), _needed.end());
// 		std::sort(_granted.begin(), _granted.end());
// 		std::set_difference(
// 		    _needed.begin(), _needed.end(), _granted.begin(), _granted.end(), std::back_inserter(_difference));
// 		return _difference.length() == 0;
// 	} else if (_scope->ok() && _scope[_ns]->ok()) {
// 		std::string _difference;
// 		std::string _granted(_scope[_ns]->str());
// 		std::sort(_needed.begin(), _needed.end());
// 		std::sort(_granted.begin(), _granted.end());
// 		std::set_difference(
// 		    _needed.begin(), _needed.end(), _granted.begin(), _granted.end(), std::back_inserter(_difference));
// 		return _difference.length() == 0;
// 	}
// 	return false;
// }

// auto zpt::rest::authorization::serialize(zpt::json _info) -> std::string {
// 	assertz(_info["owner"]->type() == zpt::JSString && _info["application"]->type() == zpt::JSString &&
// 		    _info["grant_type"]->type() == zpt::JSString,
// 		"token serialization failed: required fields are 'owner', 'application' and 'grant_type'",
// 		412,
// 		0);
// 	return _info["owner"]->str() + std::string("@") + _info["application"]->str() + std::string("/") +
// 	       _info["grant_type"]->str() + std::string("/") +
// 	       (_info["key"]->type() == zpt::JSString ? _info["key"]->str() : zpt::generate::r_key(64));
// }

// auto zpt::rest::authorization::deserialize(std::string _token) -> zpt::json {
// 	zpt::json _return = zpt::json::object();

// 	zpt::json _splitted = zpt::split(_token, "@");
// 	_return << "owner" << _splitted[0];

// 	_splitted = zpt::split(_splitted[1], "/");
// 	_return << "application" << _splitted[0] << "grant_type" << _splitted[1] << "key" << _splitted[2];

// 	return _return;
// }

// auto zpt::rest::authorization::extract(zpt::json _envelope) -> std::string {
// 	if (_envelope["headers"]["Authorization"]->ok()) {
// 		return std::string(zpt::split(std::string(_envelope["headers"]["Authorization"]), " ")[1]);
// 	}
// 	if (_envelope["payload"]["access_token"]->ok()) {
// 		std::string _param(_envelope["payload"]["access_token"]);
// 		zpt::url::decode(_param);
// 		return _param;
// 	}
// 	if (_envelope["params"]["access_token"]->ok()) {
// 		std::string _param(_envelope["params"]["access_token"]);
// 		zpt::url::decode(_param);
// 		return _param;
// 	}
// 	if (_envelope["headers"]["Cookie"]->ok()) {
// 		return std::string(zpt::split(std::string(_envelope["headers"]["Cookie"]), ";")[0]);
// 	}
// 	return "";
// }

// auto zpt::rest::authorization::headers(std::string _token) -> zpt::json {
// 	return {"Authorization", (std::string("Bearer ") + _token)};
// }

// auto zpt::rest::authorization::validate(std::string _topic,
// 					zpt::json _envelope,
// 					zpt::ev::emitter _emitter,
// 					zpt::json _roles_needed) -> zpt::json {
// 	return _emitter->authorize(_topic, _envelope, _roles_needed);
// }

// auto zpt::rest::authorization::has_roles(zpt::json _identity, zpt::json _roles_needed) -> bool {
// 	std::vector<zpt::json> _result;
// 	std::set_intersection(std::begin(_identity["roles"]->arr()),
// 			      std::end(_identity["roles"]->arr()),
// 			      std::begin(_roles_needed->arr()),
// 			      std::end(_roles_needed->arr()),
// 			      std::back_inserter(_result),
// 			      [](zpt::json _lhs, zpt::json _rhs) -> bool { return _lhs == _rhs; });
// 	return _result.size() != 0;
// }

// auto zpt::rest::uri::get_simplified_topics(std::string _pattern) -> zpt::json {
// 	zpt::json _aliases = zpt::split(_pattern, "|");
// 	zpt::json _topics = zpt::json::array();
// 	for (auto _alias : _aliases->arr()) {
// 		std::string _return;
// 		short _state = 0;
// 		bool _regex = false;
// 		bool _escaped = false;
// 		for (auto _c : _alias->str()) {
// 			switch (_c) {
// 			case '/': {
// 				if (_state == 0) {
// 					if (_regex) {
// 						if (_return.back() != '/') {
// 							_return.push_back('/');
// 						}
// 						_return.push_back('+');
// 						_regex = false;
// 					}
// 					_return.push_back(_c);
// 				}
// 				break;
// 			}
// 			case ')':
// 			case ']': {
// 				if (!_escaped) {
// 					_state--;
// 				} else {
// 					_escaped = false;
// 				}
// 				_regex = true;
// 				break;
// 			}
// 			case '(':
// 			case '[': {
// 				if (!_escaped) {
// 					_state++;
// 				} else {
// 					_escaped = false;
// 				}
// 				_regex = true;
// 				break;
// 			}
// 			case '{':
// 			case '}':
// 			case '.':
// 			case '+':
// 			case '*': {
// 				_regex = true;
// 				break;
// 			}
// 			case '$':
// 			case '^': {
// 				break;
// 			}
// 			case '\\': {
// 				_escaped = !_escaped;
// 				break;
// 			}
// 			default: {
// 				if (_state == 0) {
// 					_return.push_back(_c);
// 				}
// 			}
// 			}
// 		}
// 		if (_regex) {
// 			if (_return.back() != '/') {
// 				_return.push_back('/');
// 			}
// 			_return.push_back('#');
// 		}
// 		_topics << _return;
// 	}
// 	return _topics;
// }
//
