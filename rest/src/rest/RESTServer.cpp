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

#include <dlfcn.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/exceptions/InterruptedException.h>
#include <zapata/json.h>
#include <zapata/log/log.h>
#include <sys/sem.h>
#include <zapata/text/convert.h>
#include <zapata/file/manip.h>
#include <zapata/rest/RESTServer.h>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <systemd/sd-daemon.h>
#include <zapata/rest/codes_rest.h>
#include <zapata/python.h>
#include <zapata/lisp.h>

namespace zpt {
	namespace rest {
		pid_t root = 0;
		bool interrupted = false;
	}
}

zpt::RESTServerPtr::RESTServerPtr(std::string _name, zpt::json _options) : std::shared_ptr<zpt::RESTServer>(new zpt::RESTServer(_name, _options)) {
}

zpt::RESTServerPtr::RESTServerPtr(zpt::RESTServer * _ptr) : std::shared_ptr<zpt::RESTServer>(_ptr) {
}

zpt::RESTServerPtr::~RESTServerPtr() {
}

zpt::rest::server zpt::RESTServerPtr::setup(zpt::json _options, std::string _name) {
	if (_name.length() == 0){
		_name = _options->obj()->begin()->first;
	}	
	try {
		zpt::rest::server _server(_name, _options);
		return _server;
	}
	catch (zpt::assertion& _e) {
		zlog(_e.what() + std::string(" | ") + _e.description() + std::string("\n") + _e.backtrace(), zpt::emergency);
		throw;
	}
	catch (std::exception& _e) {
		zlog(_e.what(), zpt::emergency);
		throw;
	}
}

auto zpt::rest::terminate(int _signal) -> void {
	if (zpt::rest::interrupted) {
		exit(0);
	}
	zpt::rest::interrupted = true;
}

int zpt::RESTServerPtr::launch(int argc, char* argv[]) {	
	zpt::json _ptr = zpt::conf::rest::init(argc, argv);
	zpt::conf::setup(_ptr);

	if (!_ptr["boot"]->is_array() || _ptr["boot"]->arr()->size() == 0) {
		std::cout << "nothing to boot..." << endl << flush;
		exit(0);
	}

	if (zpt::log_lvl == -1 && _ptr["$log"]["level"]->ok()) {
		zpt::log_lvl = (int) _ptr["$log"]["level"];
	}
	if (_ptr["$log"]["format"]->ok()) {
		zpt::log_format = (_ptr["$log"]["format"] == zpt::json::string("raw") ? 0 : (_ptr["$log"]["format"] == zpt::json::string("json") ? 2 : 1));
	}
	if (_ptr["$log"]["file"]->is_string() && _ptr["$log"]["file"]->str().length() != 0) {
		std::ofstream* _lf = new std::ofstream();
		_lf->open(((std::string) _ptr["$log"]["file"]).data(), std::ofstream::out | std::ofstream::app | std::ofstream::ate);
		if (_lf->is_open()) {
			zpt::log_fd = _lf;
		}
		else {
			delete _lf;
		}
	}
		
	zpt::json _options = _ptr["boot"][0];
	_options << "proc" << zpt::json({ "directory_register", "on", "mqtt_register", "on" });

	::signal(SIGINT, zpt::rest::terminate);
	::signal(SIGTERM, zpt::rest::terminate);
	::signal(SIGABRT, zpt::rest::terminate);
	::signal(SIGSEGV, zpt::rest::terminate);
	
	std::string _name = std::string(_options["name"]);
	if (_name.length() != 0) {
		delete zpt::log_pname;
		zpt::log_pname = new string(_name.data());
	}
	std::string _u_name(_name.data());
	std::transform(_u_name.begin(), _u_name.end(), _u_name.begin(), ::toupper);
	zlog(std::string("starting RESTful service container *") + _u_name + std::string("*"), zpt::notice);
	zpt::rest::server _server(nullptr);
	try {
		_server = zpt::rest::server::setup(_options, _name);
		if (_server->suicidal()) {
			zlog("server initialization gone wrong, server is now in 'suicidal' state", zpt::emergency);
			return -1;
		}
		_server->start();
	}
	catch (zpt::assertion& _e) {
		_server->unbind();
		zlog(_e.what() + std::string(" | ") + _e.description() + std::string("\n") + _e.backtrace(), zpt::emergency);
		return -1;
	}
	catch (std::exception& _e) {
		_server->unbind();
		zlog(_e.what(), zpt::emergency);
		return -1;
	}
	_server->unbind();

	return 0;
}

zpt::RESTServer::RESTServer(std::string _name, zpt::json _options) : __name(_name), __emitter((new zpt::RESTEmitter(_options))->self()), __poll((new zpt::ZMQPoll(_options, __emitter))->self()), __options(_options), __self(this), __mqtt(new zpt::MQTT()), __upnp(_options), __max_threads(0), __alloc_threads(0), __n_threads(0), __suicidal(false) {
	try {
		assertz(this->__options["zmq"]->ok() && this->__options["zmq"]->type() == zpt::JSArray && this->__options["zmq"]->arr()->size() != 0, "zmq settings (bind, type) must be provided in the configuration file", 500, 0);
		((zpt::RESTEmitter*) this->__emitter.get())->poll(this->__poll);
		((zpt::RESTEmitter*) this->__emitter.get())->server(this->__self);

		for (auto _definition : this->__options["zmq"]->arr()) {
			short int _type = zpt::str2type(_definition["type"]->str());
			zpt::socket_ref _socket;
			switch(_type) {
				case ZMQ_ROUTER_DEALER : {
					_socket = this->__poll->add(ZMQ_ROUTER_DEALER, _definition["bind"]->str());
					break;
				}
				case ZMQ_PUB_SUB : {
					_socket = this->__poll->add(ZMQ_XPUB_XSUB, _definition["bind"]->str());
					break;
				}
				default : {
					_socket = this->__poll->add(_type, _definition["bind"]->str());
					this->__poll->poll(_socket);
					break;
				}
			}
			_definition << "connect" << (_definition["public"]->is_string() ? _definition["public"]->str() : zpt::r_replace(_socket->connection(), "@tcp", ">tcp"));
			zlog(std::string("starting 0MQ listener for ") + _socket->connection(), zpt::info);
		}
		
		if (this->__options["rest"]["modules"]->ok()) {
			for (auto _i : this->__options["rest"]["modules"]->arr()) {
				if (_i->str().find(".py") != std::string::npos) {
					if (!zpt::bridge::is_booted< zpt::python::bridge >()) {
						zpt::bridge::boot< zpt::python::bridge >(this->__options, ((zpt::RESTEmitter*) this->__emitter.get())->self());
					}
					continue;
				}
				if (_i->str().find(".lisp") != std::string::npos || _i->str().find(".fasb") != std::string::npos) {
					if (!zpt::bridge::is_booted< zpt::lisp::bridge >()) {
						zpt::bridge::boot< zpt::lisp::bridge >(this->__options, ((zpt::RESTEmitter*) this->__emitter.get())->self());
					}
					continue;
				}
			
				std::string _lib_file("lib");
				_lib_file.append((std::string) _i);
				_lib_file.append(".so");

				if (_lib_file.length() > 6) {
					zlog(std::string("loading module '") + _lib_file + std::string("'"), zpt::notice);
					void *hndl = dlopen(_lib_file.data(), RTLD_NOW);
					if (hndl == nullptr) {
						zlog(std::string(dlerror()), zpt::error);
					}
					else {
						void (*_populate)();
						_populate = (void (*)()) dlsym(hndl, "_zpt_load_");
						_populate();
					}
				}
			}
		}
	}
	catch (zpt::assertion& _e) {
		zlog(_e.what() + std::string(" | ") + _e.description() + std::string("\n") + _e.backtrace(), zpt::emergency);
		this->__suicidal = true;
	}
	catch (std::exception& _e) {
		zlog(_e.what(), zpt::emergency);
		this->__suicidal = true;
	}
}

zpt::RESTServer::~RESTServer(){
	this->__mqtt->unbind();
}

auto zpt::RESTServer::suicidal() -> bool {
	return this->__suicidal;
}

auto zpt::RESTServer::unbind() -> void {
	this->__self.reset();
}

auto zpt::RESTServer::name() -> std::string {
	return this->__name;
}

auto zpt::RESTServer::poll() -> zpt::poll {
	return this->__poll;
}

auto zpt::RESTServer::events() -> zpt::ev::emitter {
	return this->__emitter;
}

auto zpt::RESTServer::options() -> zpt::json {
	return this->__options;
}

auto zpt::RESTServer::credentials() -> zpt::json {
	return this->__emitter->credentials();
}

auto zpt::RESTServer::credentials(zpt::json _credentials) -> void {
	this->__emitter->credentials(_credentials);
}

auto zpt::RESTServer::hook(zpt::ev::initializer _callback) -> void {
	this->__initializers.push_back(_callback);
}

void zpt::RESTServer::start() {
	try {
		if (this->__options["rest"]["credentials"]["client_id"]->is_string() && this->__options["rest"]["credentials"]["client_secret"]->is_string() && this->__options["rest"]["credentials"]["server"]->is_string() && this->__options["rest"]["credentials"]["grant_type"]->is_string()) {
			zlog(std::string("going to retrieve credentials ") + std::string(this->__options["rest"]["credentials"]["client_id"]) + std::string(" @ ") + std::string(this->__options["rest"]["credentials"]["server"]), zpt::info);
			zpt::json _credentials = this->__emitter->gatekeeper()->get_credentials(this->__options["rest"]["credentials"]["client_id"], this->__options["rest"]["credentials"]["client_secret"], this->__options["rest"]["credentials"]["server"], this->__options["rest"]["credentials"]["grant_type"], this->__options["rest"]["credentials"]["scope"]);
			this->credentials(_credentials);
		}

		if (this->credentials()["endpoints"]["mqtt"]->ok() || (this->__options["mqtt"]->ok() && this->__options["mqtt"]["bind"]->ok())) {
			zpt::json _uri = zpt::uri::parse(std::string(this->credentials()["endpoints"]["mqtt"]->ok() ? this->credentials()["endpoints"]["mqtt"] : this->__options["mqtt"]["bind"]));
			if (!_uri["port"]->ok()) {
				_uri << "port" << (_uri["scheme"] == zpt::json::string("mqtts") ? 8883 : 1883);
			}
			if (_uri["user"]->ok() && _uri["password"]->ok()) {
				this->__mqtt->credentials(std::string(_uri["user"]), std::string(_uri["password"]));
			}
			else if (this->credentials()["client_id"]->is_string() && this->credentials()["access_token"]->is_string()) {
				this->__mqtt->credentials(std::string(this->credentials()["client_id"]), std::string(this->credentials()["access_token"]));
			}
			
			this->__mqtt->on("connect",
				[ this ] (zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void {
					if (_data->__rc == 0) {
						zlog(std::string("MQTT server is up and connection authenticated"), zpt::notice);
					}
				}
			);
			this->__mqtt->on("disconnect",
				[] (zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void {
					_mqtt->reconnect();
				}
			);
			this->__mqtt->on("message",
				[ this ] (zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void {
					this->route_mqtt(_data);
				}
			);
			this->__mqtt->connect(std::string(_uri["domain"]), _uri["scheme"] == zpt::json::string("mqtts"), int(_uri["port"]));
			zlog(std::string("connecting MQTT listener to ") + std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) + std::string(":") + std::string(_uri["port"]), zpt::info);

			this->__poll->poll(this->__poll->add(this->__mqtt.get()));
			
			// if (!this->__options["mqtt"]["no-threads"]->ok() || std::string(this->__options["mqtt"]["no-threads"]) != "true") {
			// 	this->__mqtt->start();
			// }			
		}

		if (this->__options["http"]->ok() && this->__options["http"]["bind"]->ok()) {
			std::thread _http(
				[ this ] () -> void {
					zpt::json _uri = zpt::uri::parse(std::string(this->__options["http"]["bind"]));
					if (!_uri["port"]->ok()) {
						_uri << "port" << 80;
					}
						
					zpt::serversocketstream_ptr _ss(new zpt::serversocketstream());
					if (!_ss->bind((uint) _uri["port"])) {
						zlog(std::string("couldn't bind HTTP listener to ") + std::string(this->__options["http"]["bind"]), zpt::alert);
						return;
					}
					zlog(std::string("starting HTTP listener for ") + std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) + std::string(":") + std::string(_uri["port"]), zpt::info);

					bool _is_ssl = _uri["scheme"] == zpt::json::string("https");
					for (; true; ) {
						int _fd = -1;
						_ss->accept(& _fd);
						if (_fd >= 0) {
							zpt::socketstream_ptr _cs(new zpt::socketstream(_fd, _is_ssl));
							this->__poll->poll(this->__poll->add(new ZMQHttp(_cs, this->__options)));
						}
						else {
							zlog("please, check your soft and hard limits for allowed number of opened file descriptors,", zpt::warning);
							zlog("unable to accept HTTP sockets, going to disable HTTP server.", zpt::emergency);
							_ss->close();
							return;
						}
					}
				}
			);
			_http.detach();
		}

		this->__poll->poll(this->__poll->add(this->__upnp.get()));
		
		for (auto _callback : this->__initializers) {
			_callback(this->__emitter);
		}

		std::string _NAME(this->__name.data());
		std::transform(_NAME.begin(), _NAME.end(), _NAME.begin(), ::toupper);
		zlog(std::string("loaded *") + _NAME + std::string("*"), zpt::notice);

		sd_notify(0, "READY=1");
		// if (this->__options["mqtt"]["no-threads"]->ok() && std::string(this->__options["mqtt"]["no-threads"]) == "true") {
		// 	this->__mqtt->loop();
		// }
		
		this->__poll->loop();
	}
	catch (zpt::InterruptedException& e) {
		return;
	}
}

bool zpt::RESTServer::route_mqtt(zpt::mqtt::data _data) {
	zpt::json _envelope = zpt::json::object();
	_envelope << "performative" << int(zpt::ev::Reply);
	if (!_data->__message["channel"]->ok() || !zpt::test::uuid(std::string(_data->__message["channel"]))) {
		_envelope << "channel" << zpt::generate::r_uuid();
	}
	else {
		_envelope << "channel" << _data->__message["channel"];
	}
	if (!_data->__message["resource"]->ok()) {
		_envelope << "resource" << _data->__topic;
	}
	else {
		_envelope << "resource" << _data->__message["resource"];
	}
	if (!_data->__message["payload"]->ok()) {
		_envelope << "payload" << _data->__message;
	}
	else {
		_envelope << "payload" << _data->__message["payload"];
	}
	if (_data->__message["headers"]->ok()) {
		_envelope << "headers" << _data->__message["headers"];
	}
	if (_data->__message["params"]->ok()) {
		_envelope << "params" << _data->__message["params"];
	}
	_envelope << "protocol" << this->__mqtt->protocol();
	ztrace(std::string("MQTT ") + std::string(_data->__topic));
	zverbose(zpt::ev::pretty(_envelope));
	try {
		this->events()->trigger(zpt::ev::Reply, std::string(_data->__topic), _envelope);
	}
	catch(...) {}
	return true;
}

auto zpt::RESTServer::publish(std::string _topic, zpt::json _payload) -> void {
	ztrace(std::string("> PUBLISH ") + _topic);
	zverbose(std::string("PUBLISH \033[1;35m") + _topic + std::string("\033[0m MQTT/3.1\n\n") + zpt::json::pretty(_payload));
	this->__mqtt->publish(_topic, _payload);
}

auto zpt::RESTServer::subscribe(std::string _topic, zpt::json _opts) -> void {
	if (bool(_opts["mqtt"])) {
		zpt::json _topics = zpt::rest::uri::get_simplified_topics(_topic);
		for (auto _t : _topics->arr()) {
			this->__mqtt->subscribe(_t->str());
		}
	}
}

std::string zpt::rest::scopes::serialize(zpt::json _info) {
	assertz(
		_info->type() == zpt::JSObject &&
		_info->obj()->size() != 0,
		"scope serialization failed: required at least one scope", 412, 0);
	std::string _scopes;
	for (auto _field : _info->obj()){
		if (_scopes.length() != 0) {
			_scopes += std::string(",");
		}
		_scopes += _field.first + std::string("{") + ((std::string) _field.second) + std::string("}");
	}
	return _scopes;
}

zpt::json zpt::rest::scopes::deserialize(std::string _scope) {
	zpt::json _return = zpt::json::object();

	zpt::json _splited = zpt::split(_scope, ",");
	for (auto _part : _splited->arr()) {
		zpt::json _pair = zpt::split(_part->str(), "{");
		_return << _pair[0]->str() << _pair[1]->str().substr(0, _pair[1]->str().length() - 1);
	}

	return _return;
}

bool zpt::rest::scopes::has_permission(std::string _scope, std::string _ns, std::string _permissions) {
	return zpt::rest::scopes::has_permission(zpt::rest::scopes::deserialize(_scope), _ns, _permissions);
}

bool zpt::rest::scopes::has_permission(zpt::json _scope, std::string _ns, std::string _needed) {
	if (_scope["all"]->ok()) {
		std::string _difference;
		std::string _granted(_scope["all"]->str());
		std::sort(_needed.begin(), _needed.end());
		std::sort(_granted.begin(), _granted.end());
		std::set_difference(_needed.begin(), _needed.end(), _granted.begin(), _granted.end(), std::back_inserter(_difference));
		return _difference.length() == 0;
	}
	else if (_scope->ok() && _scope[_ns]->ok()) {
		std::string _difference;
		std::string _granted(_scope[_ns]->str());
		std::sort(_needed.begin(), _needed.end());
		std::sort(_granted.begin(), _granted.end());
		std::set_difference(_needed.begin(), _needed.end(), _granted.begin(), _granted.end(), std::back_inserter(_difference));
		return _difference.length() == 0;
	}
	return false;
}

auto zpt::rest::authorization::serialize(zpt::json _info) -> std::string {
	assertz(
		_info["owner"]->type() == zpt::JSString &&
		_info["application"]->type() == zpt::JSString &&
		_info["grant_type"]->type() == zpt::JSString,
		"token serialization failed: required fields are 'owner', 'application' and 'grant_type'", 412, 0);
	return _info["owner"]->str() + std::string("@") + _info["application"]->str() + std::string("/") + _info["grant_type"]->str() + std::string("/") + (_info["key"]->type() == zpt::JSString ? _info["key"]->str() : zpt::generate::r_key(64));
}

auto zpt::rest::authorization::deserialize(std::string _token) -> zpt::json {
	zpt::json _return = zpt::json::object();

	zpt::json _splitted = zpt::split(_token, "@");
	_return << "owner" << _splitted[0];

	_splitted = zpt::split(_splitted[1], "/");
	_return << "application" << _splitted[0] << "grant_type" << _splitted[1] << "key" << _splitted[2];

	return _return;
}

auto zpt::rest::authorization::extract(zpt::json _envelope) -> std::string {
	if (_envelope["headers"]["Authorization"]->ok()) {
		return std::string(zpt::split(std::string(_envelope["headers"]["Authorization"]), " ")[1]);
	}
	if (_envelope["payload"]["access_token"]->ok()) {
		std::string _param(_envelope["payload"]["access_token"]);
		zpt::url::decode(_param);
		return _param;
	}
	if (_envelope["params"]["access_token"]->ok()) {
		std::string _param(_envelope["params"]["access_token"]);
		zpt::url::decode(_param);
		return _param;
	}
	if (_envelope["headers"]["Cookie"]->ok()) {
		return std::string(zpt::split(std::string(_envelope["headers"]["Cookie"]), ";")[0]);
	}
	return "";
}

auto zpt::rest::authorization::headers(std::string _token) -> zpt::json {
	return { "Authorization", (std::string("Bearer ") + _token) };
}

auto zpt::rest::authorization::validate(std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter, zpt::json _roles_needed) -> zpt::json {
	return _emitter->authorize(_topic, _envelope, _roles_needed);
}

auto zpt::rest::authorization::has_roles(zpt::json _identity, zpt::json _roles_needed) -> bool {
	std::vector< zpt::json > _result;
	std::set_intersection(std::begin(_identity["roles"]->arr()), std::end(_identity["roles"]->arr()), std::begin(_roles_needed->arr()), std::end(_roles_needed->arr()), std::back_inserter(_result));
	return _result.size() != 0;
}

auto zpt::rest::uri::get_simplified_topics(std::string _pattern) -> zpt::json {
	zpt::json _aliases = zpt::split(_pattern, "|");
	zpt::json _topics = zpt::json::array();
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
							_return.push_back('+');
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
				case '+' :
				case '*' : {
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
			_return.push_back('#');
		}
		_topics << _return;
	}
	return _topics;
}

auto zpt::conf::rest::init(int argc, char* argv[]) -> zpt::json {
	zpt::log_fd = &std::cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new string(argv[0]);

	zpt::json _args = zpt::conf::getopt(argc, argv);

	if (!_args["c"]->ok()) {
		zlog("unable to start client: a valid configuration file must be provided", zpt::error);
		exit(-10);
	}
	
	short _log_level = (_args["l"]->ok() ? int(_args["l"][0]) : -1);
	std::string _conf_file = std::string(_args["c"][0]);
	zpt::log_format = (bool(_args["r"]) ? 0 : (bool(_args["j"]) ? 2 : 1));

	zpt::json _ptr;
	std::ifstream _in;
	_in.open(_conf_file.data());
	if (!_in.is_open()) {
		zlog("unable to start: a valid configuration file must be provided", zpt::error);
		exit(-10);
	}
	try {
		_in >> _ptr;
	}
	catch(zpt::SyntaxErrorException& _e) {
		zlog("unable to start: syntax error when parsing configuration file", zpt::error);
		exit(-10);
	}

	zpt::log_lvl = _log_level;
	return _ptr;
}
