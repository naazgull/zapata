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
#include <map>

zpt::ZMQMutationEmitter::ZMQMutationEmitter(zpt::json _options) : zpt::MutationEmitter(_options), __socket(new zpt::ZMQPubSub(std::string(_options["mutations"]["connect"]), _options)) {
	((zpt::ZMQPubSub*) this->__socket.get())->subscribe("/v2/mutations");
}

zpt::ZMQMutationEmitter::~ZMQMutationEmitter() {
}

auto zpt::ZMQMutationEmitter::version() -> std::string {
	return this->options()["rest"]["version"]->str();
}

auto zpt::ZMQMutationEmitter::socket() -> zpt::socket {
	return this->__socket;
}

auto zpt::ZMQMutationEmitter::on(zpt::mutation::operation _operation, std::string _data_class_ns,  zpt::mutation::Handler _handler, zpt::json _opts) -> std::string {
	zpt::replace(_data_class_ns, std::string("/") + this->version(), "");
	std::regex _url_pattern(std::string("/") + this->version() + std::string("/mutations/([^/]+)") + _data_class_ns);

	std::vector< zpt::mutation::Handler> _handlers;
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Insert ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Remove ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Update ? nullptr : _handler));
	_handlers.push_back((_handler == nullptr || _operation != zpt::mutation::Replace ? nullptr : _handler));

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	zlog(string("registered mutation listener for ") + _data_class_ns, zpt::info);
	return _uuid;
}

auto zpt::ZMQMutationEmitter::on(std::string _data_class_ns,  std::map< zpt::mutation::operation, zpt::mutation::Handler > _handler_set, zpt::json _opts) -> std::string {
	zpt::replace(_data_class_ns, std::string("/") + this->version(), "");
	std::regex _url_pattern(std::string("/") + this->version() + std::string("/mutations/([^/]+)") + _data_class_ns);

	std::map< zpt::mutation::operation, zpt::mutation::Handler >::iterator _found;
	vector< zpt::mutation::Handler> _handlers;
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Insert)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Remove)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Update)) == _handler_set.end() ?  nullptr : _found->second);
	_handlers.push_back((_found = _handler_set.find(zpt::mutation::Replace)) == _handler_set.end() ?  nullptr : _found->second);

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	zlog(string("registered mutation listener for ") + _data_class_ns, zpt::info);
	return _uuid;
}

auto zpt::ZMQMutationEmitter::on(zpt::mutation::listener _listener, zpt::json _opts) -> std::string {
	std::string _data_class_ns = _listener->ns();
	zpt::replace(_data_class_ns, std::string("/") + this->version(), "");
	std::regex _url_pattern(std::string("/") + this->version() + std::string("/mutations/([^/]+)") + _data_class_ns);

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
			default : {
			}
		}
	};

	
	vector< zpt::mutation::Handler > _handlers;
	for (short _idx = zpt::mutation::Insert; _idx != zpt::mutation::Replace + 1; _idx++) {
		_handlers.push_back(_handler);
	}

	std::string _uuid = zpt::generate::r_uuid();
	this->__resources.insert(std::make_pair(_uuid, std::make_pair(_url_pattern, _handlers)));
	zlog(string("registered mutation listener for ") + _listener->ns(), zpt::info);
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

auto zpt::ZMQMutationEmitter::route(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record) -> zpt::json {
	std::string _op = zpt::mutation::to_str(_operation);
	std::transform(std::begin(_op), std::end(_op), std::begin(_op), ::tolower);
	this->__socket->send((zpt::ev::performative) _operation, _data_class_ns, _record);
	return zpt::undefined;
}

auto zpt::ZMQMutationEmitter::trigger(zpt::mutation::operation _operation, std::string _data_class_ns, zpt::json _record) -> zpt::json {
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

int zpt::ZMQMutationServerPtr::launch(int argc, char* argv[]) {
	zpt::log_fd = &std::cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new std::string(argv[0]);

	zpt::json _args = zpt::conf::getopt(argc, argv);
	short _log_level = (_args["l"]->ok() ? int(_args["l"][0]) : -1);
	std::string _conf_file = (_args["c"]->ok() ? std::string(_args["c"][0]) : "");
	
	zpt::log_format = !bool(_args["r"]);
	zpt::log_lvl = _log_level;

	zlog(zpt::json::pretty(_args), zpt::alert);

	zpt::json _ptr;
	if (_conf_file.length() == 0) {
		zlog("must provide a configuration file", zpt::alert);
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

	if (zpt::log_lvl == -1 && _ptr["log"]["level"]->ok()) {
		zpt::log_lvl = (int) _ptr["log"]["level"];
	}
	if (zpt::log_lvl == -1) {
		zpt::log_lvl = 8;
	}
	if (!!_ptr["log"]["file"]) {
		zpt::log_fd = new std::ofstream();
		((std::ofstream *) zpt::log_fd)->open(((std::string) _ptr["log"]["file"]).data());
	}

	zlog("starting Mutation container", zpt::alert);
	zpt::mutation::server _server = zpt::mutation::server::setup(_ptr);
	_server->start();
	return 0;
}

zpt::ZMQMutationServer::ZMQMutationServer(zpt::json _options) : __options(_options), __self(this), __socket(nullptr) {
	zsys_init();
	zsys_handler_set(nullptr);
	assertz(zsys_has_curve(), "no security layer for 0mq. Is libcurve (https://github.com/zeromq/libcurve) installed?", 500, 0);

	std::string _connection = this->__options["mutations"]["bind"]->str();
	std::string _connection1(_connection.substr(0, _connection.find(",")));
	std::string _connection2(_connection.substr(_connection.find(",") + 1));
	this->__socket = zactor_new(zproxy, nullptr);
	zstr_sendx(this->__socket, "FRONTEND", "XPUB", _connection2.data(), nullptr);
	zsock_wait(this->__socket);
	zstr_sendx(this->__socket, "BACKEND", "XSUB", _connection1.data(), nullptr);
	zsock_wait(this->__socket);
	zlog(std::string("attaching XSUB/XPUB socket to ") + _connection, zpt::notice);
}

zpt::ZMQMutationServer::~ZMQMutationServer(){
	zlog(std::string("dettaching XPUB/XSUB from ") + std::string(this->__options["mutations"]["bind"]), zpt::notice);
	zactor_destroy(&this->__socket);
	this->__self.reset();
}

zpt::json zpt::ZMQMutationServer::options() {
	return this->__options;
}

void zpt::ZMQMutationServer::start() {
	try {
		//zpt::socket _sub(new zpt::ZMQSub(_connection2, this->__options));
		std::string _sub_connection = this->__options["mutations"]["connect"]->str();
		std::string _sub_connection1(_sub_connection.substr(0, _sub_connection.find(",")));
		std::string _sub_connection2(_sub_connection.substr(_sub_connection.find(",") + 1));
		zsock_t* _sub = zsock_new(ZMQ_SUB);
		assertz(zsock_attach(_sub, _sub_connection2.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(_sub)) + std::string(" socket to ") + _sub_connection2, 500, 0);
		zsock_set_sndhwm(_sub, 1000);	
		zsock_set_sndtimeo(_sub, 20000);
		zpoller_t* _poll = zpoller_new(_sub, nullptr);
		for(; true; ) {
			zsock_t* _awaken = (zsock_t*) zpoller_wait(_poll, -1);

			if (_awaken == nullptr) {
				continue;
			}
					
			zframe_t* _frame1;
			zframe_t* _frame2;
			if (zsock_recv(_sub, "ff", &_frame1, &_frame2) == 0) {
				char* _bytes = nullptr;

				_bytes = zframe_strdup(_frame1);
				std::string _directive(std::string(_bytes, zframe_size(_frame1)));
				delete _bytes;
				zframe_destroy(&_frame1);

				_bytes = zframe_strdup(_frame2);
				zpt::json _envelope(std::string(_bytes, zframe_size(_frame2)));
				delete _bytes;
				zframe_destroy(&_frame2);
				zlog(std::string(_envelope), zpt::info);
			}
		}
	}
	catch (zpt::InterruptedException& e) {
		return;
	}
}
