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

#include <zapata/zmq/ZMQMutationEmitter.h>
#include <sys/sem.h>
#include <map>

namespace zpt {
	namespace mutation {
		namespace zmq {
			emitter* __emitter = nullptr;
		}
	}
}

zpt::ZMQMutationEmitter::ZMQMutationEmitter(zpt::json _options) : zpt::MutationEmitter(_options), __socket(new ZMQPubSub(std::string(_options["$mutations"]["connect"]), _options)) {
	zsys_init();
	zsys_handler_set(nullptr);
	assertz(zsys_has_curve(), "no security layer for 0mq. Is libcurve (https://github.com/zeromq/libcurve) installed?", 500, 0);
	if (zpt::mutation::zmq::__emitter != nullptr) {
		delete zpt::mutation::zmq::__emitter;
		zlog("something is definitely wrong, ZMQMutationEmitter already initialized", zpt::emergency);
	}
	zpt::mutation::zmq::__emitter = this;
	
	
	zsock_set_subscribe(this->__socket->in(), "");
}

zpt::ZMQMutationEmitter::~ZMQMutationEmitter() {
}

auto zpt::ZMQMutationEmitter::version() -> std::string {
	return this->options()["rest"]["version"]->str();
}

auto zpt::ZMQMutationEmitter::loop() -> void {
	for (; true; ) {
		zpt::json _envelope = this->__socket->recv();
		this->trigger((zpt::mutation::operation) int(_envelope["performative"]), std::string(_envelope["resource"]), _envelope);
	}
}

auto zpt::ZMQMutationEmitter::on(zpt::mutation::operation _operation, std::string _data_class_ns,  zpt::mutation::Handler _handler, zpt::json _opts) -> std::string {
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
	zlog(string("registered mutation listener for ") + _uri, zpt::notice);
	return _uuid;
}

auto zpt::ZMQMutationEmitter::on(std::string _data_class_ns,  std::map< zpt::mutation::operation, zpt::mutation::Handler > _handler_set, zpt::json _opts) -> std::string {
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
	zlog(string("registered mutation listener for ") + _uri, zpt::notice);
	return _uuid;
}

auto zpt::ZMQMutationEmitter::on(zpt::mutation::listener _listener, zpt::json _opts) -> std::string {
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
	zlog(string("registered mutation listener for ") + _uri, zpt::notice);
	return _uuid;
}

auto zpt::ZMQMutationEmitter::off(zpt::mutation::operation _operation, std::string _callback_id) -> void {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		this->__resources.erase(_callback_id);
	}
}

auto zpt::ZMQMutationEmitter::off(std::string _callback_id) -> void {
	auto _found = this->__resources.find(_callback_id);
	if (_found != this->__resources.end()) {
		this->__resources.erase(_callback_id);
	}
}		

auto zpt::ZMQMutationEmitter::route(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts) -> zpt::json {
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

auto zpt::ZMQMutationEmitter::trigger(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record, zpt::json _opts) -> zpt::json {
	for (auto _i : this->__resources) {
		std::regex _regexp = _i.second.first;
		if (std::regex_match(_data_class_ns, _regexp)) {
			if (_i.second.second[_operation] != nullptr) {
				_i.second.second[_operation](_operation, _data_class_ns, _record, this->self());
			}
		}
	}
	return zpt::undefined;
}

auto zpt::ZMQMutationEmitter::instance() -> zpt::mutation::emitter {
	assertz(zpt::mutation::zmq::__emitter != nullptr, "ZMQ mutation emitter has not been initialized", 500, 0);
	return zpt::mutation::zmq::__emitter->self();
}

zpt::ZMQMutationServerPtr::ZMQMutationServerPtr(zpt::json _options) : std::shared_ptr<zpt::ZMQMutationServer>(new zpt::ZMQMutationServer(_options)) {
}

zpt::ZMQMutationServerPtr::ZMQMutationServerPtr(zpt::ZMQMutationServer * _ptr) : std::shared_ptr<zpt::ZMQMutationServer>(_ptr) {
}

zpt::ZMQMutationServerPtr::~ZMQMutationServerPtr() {
}

zpt::mutation::server zpt::ZMQMutationServerPtr::setup(zpt::json _options) {
	zpt::conf::setup(_options);
	zpt::mutation::server _server(_options);
	return _server;
}

int zpt::ZMQMutationServerPtr::launch(int argc, char* argv[], int _semaphore) {
	zpt::log_fd = &std::cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new std::string(argv[0]);

	zpt::json _args = zpt::conf::getopt(argc, argv);
	short _log_level = (_args["l"]->ok() ? int(_args["l"][0]) : -1);
	std::string _conf_file = (_args["c"]->ok() ? std::string(_args["c"][0]) : "");
	
	zpt::log_format = (bool(_args["r"]) ? 0 : (bool(_args["j"]) ? 2 : 1));
	zpt::log_lvl = _log_level;

	zpt::json _ptr;
	if (_conf_file.length() == 0) {
		zlog("must provide a configuration file", zpt::warning);
	}
	else {
		std::ifstream _in;
		_in.open(_conf_file.data());
		if (!_in.is_open()) {
			zlog("unable to start client: a valid configuration file must be provided", zpt::error);
			exit(-10);
		}
		try {
			_in >> _ptr;
		}
		catch(zpt::SyntaxErrorException& _e) {
			zlog("unable to start client: syntax error when parsing configuration file", zpt::error);
			exit(-10);
		}
		_ptr << "argv" << _args;
	}

	if (!bool(_ptr["$mutations"]["run"])) {
		exit(0);
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

	zlog("starting mutations container", zpt::warning);
	zpt::mutation::server _server = zpt::mutation::server::setup(_ptr);
	struct sembuf _unlock[1] = { { (short unsigned int) 0, -1 } };
	semop(_semaphore, _unlock, 1);	
	_server->start();
	return 0;
}

zpt::ZMQMutationServer::ZMQMutationServer(zpt::json _options) : __options(_options), __self(this), __server(new ZMQXPubXSub(std::string(_options["$mutations"]["bind"]), _options)), __client(new ZMQPubSub(std::string(_options["$mutations"]["connect"]), _options)) {
	zsys_init();
	zsys_handler_set(nullptr);
	assertz(zsys_has_curve(), "no security layer for 0mq. Is libcurve (https://github.com/zeromq/libcurve) installed?", 500, 0);

	zsock_set_subscribe(this->__client->in(), "");
}

zpt::ZMQMutationServer::~ZMQMutationServer(){
	this->__self.reset();
}

zpt::json zpt::ZMQMutationServer::options() {
	return this->__options;
}

void zpt::ZMQMutationServer::start() {
	for(; true; ) {
		this->__client->recv();
	}
	
}
