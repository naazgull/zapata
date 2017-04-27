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

#include <zapata/rest/MutationEmitter.h>
#include <zapata/rest/RESTServer.h>
#include <sys/sem.h>
#include <map>
#include <zapata/mqtt.h>

namespace zpt {
	namespace mutation {
		namespace zmq {
			emitter* __emitter = nullptr;
		}
	}
}

zpt::RESTMutationEmitter::RESTMutationEmitter(zpt::json _options) : zpt::MutationEmitter(_options), __socket(new ZMQPubSub(std::string(_options["$mutations"]["connect"]), _options)) {
	if (zpt::mutation::zmq::__emitter != nullptr) {
		delete zpt::mutation::zmq::__emitter;
		zlog("something is definitely wrong, RESTMutationEmitter already initialized", zpt::emergency);
	}
	zpt::mutation::zmq::__emitter = this;
	this->__socket->in()->setsockopt(ZMQ_SUBSCRIBE, "", 0);
}

zpt::RESTMutationEmitter::~RESTMutationEmitter() {
}

auto zpt::RESTMutationEmitter::version() -> std::string {
	return this->options()["rest"]["version"]->str();
}

auto zpt::RESTMutationEmitter::loop() -> void {
	for (; true; ) {
		zpt::json _envelope = this->__socket->recv();
		this->trigger((zpt::mutation::operation) int(_envelope["performative"]), std::string(_envelope["resource"]), _envelope);
	}
}

auto zpt::RESTMutationEmitter::on(zpt::mutation::operation _operation, std::string _data_class_ns,  zpt::mutation::Handler _handler, zpt::json _opts) -> std::string {
	std::string _uri(std::string("^/") + this->version() + std::string("/mutations/([^/]+)") + zpt::r_replace(zpt::r_replace(zpt::r_replace(_data_class_ns, std::string("/") + this->version(), ""), "^", ""), "$", "") + std::string("$"));
	std::regex _url_pattern(_uri);

	std::vector< zpt::mutation::Handler> _handlers;
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Insert ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Remove ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Update ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Replace ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Connect ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Reconnect ? nullptr : _handler));

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	ztrace(string("registered mutation handler for ") + _uri);
	return _uuid;
}

auto zpt::RESTMutationEmitter::on(std::string _data_class_ns,  std::map< zpt::mutation::operation, zpt::mutation::Handler > _handler_set, zpt::json _opts) -> std::string {
	std::string _uri(std::string("^/") + this->version() + std::string("/mutations/([^/]+)") + zpt::r_replace(zpt::r_replace(zpt::r_replace(_data_class_ns, std::string("/") + this->version(), ""), "^", ""), "$", "") + std::string("$"));
	std::regex _url_pattern(_uri);

	std::map< zpt::mutation::operation, zpt::mutation::Handler >::iterator _found;
	vector< zpt::mutation::Handler> _handlers;
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Insert)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Remove)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Update)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Replace)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Connect)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Reconnect)) == _handler_set.end() ?  nullptr : _found->second);

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	ztrace(string("registered mutation handler for ") + _uri);
	return _uuid;
}

auto zpt::RESTMutationEmitter::on(zpt::mutation::listener _listener, zpt::json _opts) -> std::string {
	std::string _uri(std::string("^/") + this->version() + std::string("/mutations/([^/]+)") + zpt::r_replace(zpt::r_replace(zpt::r_replace(_listener->ns(), std::string("/") + this->version(), ""), "^", ""), "$", "") + std::string("$"));
	std::regex _url_pattern(_uri);

	zpt::mutation::Handler _handler = [ _listener ] (zpt::mutation::operation _performative, std::string _resource, zpt::json _envelope, zpt::mutation::emitter _emitter) -> void {
		switch (_performative) {
			case zpt::mutation::Insert : {
				_listener->inserted(_resource, _envelope, _emitter);
			}
			case zpt::mutation::Remove : {
				_listener->removed(_resource, _envelope, _emitter);
			}
			case zpt::mutation::Update : {
				_listener->updated(_resource, _envelope, _emitter);
			}
			case zpt::mutation::Replace : {
				_listener->replaced(_resource, _envelope, _emitter);
			}
			case zpt::mutation::Connect : {
				_listener->connected(_resource, _envelope, _emitter);
			}
			case zpt::mutation::Reconnect : {
				_listener->reconnected(_resource, _envelope, _emitter);
			}
			default : {
			}
		}
	};
	
	vector< zpt::mutation::Handler > _handlers;
	for (short _idx = zpt::mutation::Insert; _idx != zpt::mutation::Reconnect + 1; _idx++) {
		_handlers.push_back(_handler);
	}

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	ztrace(string("registered mutation handler for ") + _uri);
	return _uuid;
}

auto zpt::RESTMutationEmitter::off(zpt::mutation::operation _operation, std::string _callback_id) -> void {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		this->__resources.erase(_callback_id);
	}
}

auto zpt::RESTMutationEmitter::off(std::string _callback_id) -> void {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		this->__resources.erase(_callback_id);
	}
}		

auto zpt::RESTMutationEmitter::route(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts) -> zpt::json {
	if (bool(_opts["mutated-event"])) return zpt::undefined;
	
	std::string _op = zpt::mutation::to_str(_operation);
	std::transform(std::begin(_op), std::end(_op), std::begin(_op), ::tolower);
	std::string _uri(std::string("/") + this->version() + std::string("/mutations/") + _op + zpt::r_replace(_data_class_ns, std::string("/") + this->version(), ""));
	
	zpt::json _envelope = {
		"channel", _uri,
		"performative", (int) _operation,
		"resource", _uri, 
		"payload", _record
	};

	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		this->__socket->send(_envelope); }
	return zpt::undefined;
}

auto zpt::RESTMutationEmitter::trigger(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts) -> zpt::json {
	if (bool(_opts["mutated-event"])) return zpt::undefined;

	for (auto _i : this->__resources) {
		std::regex _regexp = _i.second.first;
		if (std::regex_match(_data_class_ns, _regexp)) {
			if (_i.second.second[_operation] != nullptr) {
				try {
					_i.second.second[_operation](_operation, _data_class_ns, _record, this->self());
				}
				catch(zpt::assertion& _e) {
					zlog(_e.what() + std::string(", ") + _e.description(), zpt::error);
					return zpt::undefined;
				}
				catch(std::exception& _e) {
					zlog(_e.what(), zpt::error);
					return zpt::undefined;
				}
			}
		}
	}
	return zpt::undefined;
}

auto zpt::RESTMutationEmitter::instance() -> zpt::mutation::emitter {
	assertz(zpt::mutation::zmq::__emitter != nullptr, "REST mutation emitter has not been initialized", 500, 0);
	return zpt::mutation::zmq::__emitter->self();
}

zpt::RESTMutationServerPtr::RESTMutationServerPtr(zpt::json _options) : std::shared_ptr<zpt::RESTMutationServer>(new zpt::RESTMutationServer(_options)) {
}

zpt::RESTMutationServerPtr::RESTMutationServerPtr(zpt::RESTMutationServer * _ptr) : std::shared_ptr<zpt::RESTMutationServer>(_ptr) {
}

zpt::RESTMutationServerPtr::~RESTMutationServerPtr() {
}

zpt::mutation::server zpt::RESTMutationServerPtr::setup(zpt::json _options) {
	zpt::conf::setup(_options);
	zpt::mutation::server _server(_options);
	return _server;
}

int zpt::RESTMutationServerPtr::launch(int argc, char* argv[], int _semaphore) {
	zpt::log_fd = &std::cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new std::string(argv[0]);

	zpt::json _ptr = zpt::conf::rest::init(argc, argv);
	zpt::conf::setup(_ptr);

	if (std::string(_ptr["$mutations"]["run"]) != "true") {
		std::cout << "THE MISTERY IS HERE: " << __FILE__ << ":" << __LINE__ << endl << flush;//exit(0);
	}

	if (zpt::log_lvl == -1 && _ptr["$log"]["level"]->ok()) {
		zpt::log_lvl = (int) _ptr["$log"]["level"];
	}
	if (zpt::log_lvl == -1) {
		zpt::log_lvl = 8;
	}
	if (_ptr["$log"]["format"]->ok()) {
		zpt::log_format = (_ptr["$log"]["format"] == zpt::json::string("raw") ? 0 : (_ptr["$log"]["format"] == zpt::json::string("json") ? 2 : 1));
	}
	if (_ptr["$log"]["file"]->ok()) {
		zpt::log_fd = new std::ofstream();
		((std::ofstream *) zpt::log_fd)->open(((std::string) _ptr["$log"]["file"]).data(), std::ofstream::out | std::ofstream::app | std::ofstream::ate);
	}

	zlog("starting mutations container", zpt::notice);
	zpt::mutation::server _server = zpt::mutation::server::setup(_ptr);
	struct sembuf _unlock[1] = { { (short unsigned int) 0, -1 } };
	semop(_semaphore, _unlock, 1);	
	_server->start();
	return 0;
}

zpt::RESTMutationServer::RESTMutationServer(zpt::json _options) : __options(_options), __self(this), __server(new ZMQXPubXSub(std::string(_options["$mutations"]["bind"]), _options)), __client(new ZMQPubSub(std::string(_options["$mutations"]["connect"]), _options)) {
	this->__client->in()->setsockopt(ZMQ_SUBSCRIBE, "", 0);
}

zpt::RESTMutationServer::~RESTMutationServer(){
	this->__self.reset();
}

auto zpt::RESTMutationServer::options() -> zpt::json {
	return this->__options;
}

auto zpt::RESTMutationServer::start() -> void {
	bool _has_mqtt = this->__options["$mutations"]["forward"]["mqtt"]->is_string();
	zpt::mqtt::broker _mqtt(new zpt::MQTT());
	if (_has_mqtt) {
		zpt::json _uri = zpt::uri::parse(this->__options["$mutations"]["forward"]["mqtt"]->str());
		if (!_uri["port"]->ok()) {
			_uri << "port" << (_uri["scheme"] == zpt::json::string("mqtts") ? 8883 : 1883);
		}
		if (_uri["user"]->ok() && _uri["password"]->ok()) {
			_mqtt->credentials(std::string(_uri["user"]), std::string(_uri["password"]));
		}
		_mqtt->on("connect",
			[ this ] (zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void {
				if (_data->__rc == 0) {
					zlog(std::string("MQTT server is up and connection authenticated"), zpt::notice);
				}
			}
		);
		_mqtt->connect(std::string(_uri["domain"]), _uri["scheme"] == zpt::json::string("mqtts"), int(_uri["port"]));
		_mqtt->start();
		zlog(std::string("starting MQTT forwarding to ") + std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) + std::string(":") + std::string(_uri["port"]), zpt::info);
	}
	for(; true; ) {
		zpt::json _mutation = this->__client->recv();
		if (_has_mqtt) {
			_mqtt->publish(std::string(_mutation["resource"]), _mutation);
		}
	}
	
}
