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
#include <zapata/rest/codes_rest.h>
#include <zapata/python.h>
#include <zapata/lisp.h>
#include <zapata/rest/MutationEmitter.h>

namespace zpt {
	namespace rest {
		pid_t* pids = nullptr;
		size_t n_pid = 0;
		int m_sem = -1;
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
	if (zpt::rest::pids == nullptr) {
		return;
	}
	if (zpt::rest::m_sem != -1) {
		semctl(zpt::rest::m_sem, 0, IPC_RMID);
		zpt::rest::m_sem = -1;
	}
	for (size_t _i = zpt::rest::n_pid; _i != 0; _i--) {
		zlog(std::string("terminating process ") + std::to_string(zpt::rest::pids[_i - 1]), zpt::warning);
		::kill(zpt::rest::pids[_i - 1], (_signal == SIGSEGV ? SIGTERM : _signal));
	}
	delete [] zpt::rest::pids;
	zpt::rest::pids = nullptr;
}

int zpt::RESTServerPtr::launch(int argc, char* argv[]) {	
	zpt::json _ptr = zpt::conf::rest::init(argc, argv);
	zpt::conf::setup(_ptr);

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
	
	zpt::json _to_spawn = zpt::json::array();
	size_t _n_spawn = 0;
	if (_ptr["boot"]->is_array()) {
		for (auto _spawn : _ptr["boot"]->arr()) {
			if (!((bool) _spawn["enabled"])) {
				continue;
			}
			_n_spawn += (_spawn["spawn"]->ok() ? (size_t) _spawn["spawn"] : 1);
			_to_spawn << _spawn;
		}
	}

	std::string _f_key = std::string("/proc/") + std::to_string(::getpid()) + std::string("/");
	
	zpt::rest::n_pid = 0;
	zpt::rest::pids = new pid_t[_n_spawn];
	key_t _s_key = ftok((_f_key + std::string("stat")).data(), 1);
	int _sync_sem = semget(_s_key, 2, IPC_CREAT | 0777);
	assertz(_sync_sem != -1, "unabled to attach sem to /proc/{pid}/stat", 500, 0);

	if (std::string(_ptr["$mutations"]["run"]) == "true") {
		key_t _key = ftok("/usr/bin/zpt", 1);
		zpt::rest::m_sem = semget(_key, 1, IPC_CREAT | IPC_EXCL | 0777);
		if (zpt::rest::m_sem != -1) {
			struct sembuf _inc[1] = { { (short unsigned int) 0, 1 } };
			semop(zpt::rest::m_sem, _inc, 1);	
		
			pid_t _pid = fork();
			if (_pid == 0) {
				try {
					zpt::mutation::server::launch(argc, argv, zpt::rest::m_sem);
				}
				catch (zpt::assertion& _e) {
					zlog(_e.what() + string(" | ") + _e.description(), zpt::emergency);
					return -1;
				}
				catch (std::exception& _e) {
					zlog(_e.what(), zpt::emergency);
					return -1;
				}
				return 0;
			}
			else {
				struct sembuf _block[1] = { { (short unsigned int) 0, 0 } };
				semop(zpt::rest::m_sem, _block, 1);	
				zpt::rest::pids[zpt::rest::n_pid++] = _pid;
			}
		}
		else {
			zlog("mutation server already runing", zpt::notice);
		}
	}
	
	size_t _spawned = 0;
	std::string _name;
	zpt::json _options;
	for (auto _spawn : _to_spawn->arr()) {
		if (_spawned == _to_spawn->arr()->size() - 1) {
			_name.assign((_spawn["name"]->is_string() ? std::string(_spawn["name"]) : std::string("container-") + std::to_string(_spawned)));
			_options = _spawn;
			::signal(SIGINT, zpt::rest::terminate);
			::signal(SIGTERM, zpt::rest::terminate);
			::signal(SIGABRT, zpt::rest::terminate);
			::signal(SIGSEGV, zpt::rest::terminate);
		}
		else {
			struct sembuf _inc[1] = { { (short unsigned int) 0, 1 } };
			semop(_sync_sem, _inc, 1);
			pid_t _pid = fork();
			if (_pid == 0) {
				_name.assign((_spawn["name"]->is_string() ? std::string(_spawn["name"]) : std::string("container-") + std::to_string(_spawned)));
				_options = _spawn;
				break;
			}
			else {
				struct sembuf _block[1] = { { (short unsigned int) 0, 0 } };
				semop(_sync_sem, _block, 1);
				zpt::rest::pids[zpt::rest::n_pid++] = _pid;
				_spawned++;
			}
		}
	}
	
	if (_ptr["$mutations"]->ok()) {
		_options << "$mutations" << _ptr["$mutations"];
	}
	
	size_t _n_workers = _options["spawn"]->ok() ? (size_t) _options["spawn"] : 1;
	size_t _i = 0;
	for (; _i != _n_workers - 1; _i++) {
		struct sembuf _inc[2] = { { (short unsigned int) 0, 1 }, { (short unsigned int) 1, 1 } };
		semop(_sync_sem, _inc, 2);
		pid_t _pid = fork();
		if (_pid == 0) {
			zpt::rest::pids[zpt::rest::n_pid++] = _pid;
			break;
		}
		struct sembuf _block[1] = { { (short unsigned int) 1, 0 } };
		semop(_sync_sem, _block, 1);	
	}
	if (_i == _n_workers - 1) {
		struct sembuf _inc[1] = { { (short unsigned int) 1, 1 } };
		semop(_sync_sem, _inc, 1);
	}
	_name += std::string("-") + std::to_string(_i + 1);
	
	if (zpt::log_lvl == -1 && _options["log"]["level"]->ok()) {
		zpt::log_lvl = (int) _options["log"]["level"];
	}
	if (zpt::log_lvl == -1) {
		zpt::log_lvl = 8;
	}
	if (_options["log"]["format"]->ok()) {
		zpt::log_format = (_options["log"]["format"] == zpt::json::string("raw") ? 0 : (_options["log"]["format"] == zpt::json::string("json") ? 2 : 1));
	}
	if (_options["log"]["file"]->is_string() && _options["log"]["file"]->str().length() != 0) {
		std::ofstream* _lf = new std::ofstream();
		_lf->open(((std::string) _options["log"]["file"]).data(), std::ofstream::out | std::ofstream::app | std::ofstream::ate);
		if (_lf->is_open()) {
			zpt::log_fd = _lf;
		}
		else {
			delete _lf;
		}
	}
	
	zlog(std::string("starting RESTful service container: ") + _name, zpt::notice);
	zpt::rest::server _server(nullptr);
	try {
		_server = zpt::rest::server::setup(_options, _name);
		if (_server->suicidal()) {
			zlog("server initialization gone wrong, server is now in 'suicidal' state", zpt::emergency);
			semctl(_sync_sem, 0, IPC_RMID);
			return -1;
		}
		if (_spawned == _to_spawn->arr()->size() - 1) {
			semctl(_sync_sem, 0, IPC_RMID);
			_server->start();
		}
		else {
			_server->start({ zpt::array, _sync_sem });
		}
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

zpt::RESTServer::RESTServer(std::string _name, zpt::json _options) : __name(_name), __emitter((new zpt::RESTEmitter(_options))->self()), __poll((new zpt::ZMQPoll(_options, __emitter))->self()), __options(_options), __self(this), __mqtt(new zpt::MQTT()), __max_threads(0), __alloc_threads(0), __n_threads(0), __suicidal(false) {
	try {
		assertz(this->__options["zmq"]->ok() && this->__options["zmq"]->type() == zpt::JSArray && this->__options["zmq"]->arr()->size() != 0, "zmq settings (bind, type) must be provided in the configuration file", 500, 0);
		((zpt::RESTEmitter*) this->__emitter.get())->poll(this->__poll);
		((zpt::RESTEmitter*) this->__emitter.get())->server(this->__self);

		for (auto _definition : this->__options["zmq"]->arr()) {
			short int _type = zpt::str2type(_definition["type"]->str());

			switch(_type) {
				case ZMQ_ROUTER_DEALER : {
					zpt::socket _socket = this->__poll->bind(ZMQ_ROUTER_DEALER, _definition["bind"]->str());
					_definition << "connect" << (_definition["public"]->is_string() ? _definition["public"]->str() : zpt::r_replace(_socket->connection(), "@tcp", ">tcp"));
					this->__router_dealer.push_back(_socket);
					zlog(std::string("starting 0mq listener for ") + _socket->connection(), zpt::info);
					break;
				}
				case ZMQ_PUB_SUB : {
					zpt::socket _socket = this->__poll->bind(ZMQ_XPUB_XSUB, _definition["bind"]->str());
					_definition << "connect" << (_definition["public"]->is_string() ? _definition["public"]->str() : zpt::r_replace(_socket->connection(), "@tcp", ">tcp"));
					this->__pub_sub.push_back(_socket);
					zlog(std::string("starting 0mq listener for ") + _socket->connection(), zpt::info);
					break;
				}
				case ZMQ_REP : {
					if (((size_t) this->__options["threads"]) != 0) {
						this->__max_threads = ((size_t) this->__options["threads"]);
						zpt::json _uri = zpt::uri::parse(_definition["bind"]->str());
						if (_uri["type"] == zpt::json::string("@")) {
							uint _available = 32769;
							if (_uri["port"] != zpt::json::string("*")) {
								_available = (uint) _uri["port"];
							}
							else {
								zpt::serversocketstream _ss;
								do {
									if (_ss.bind(_available)) {
										break;
									}
									_available++;
								}
								while(_available < 60999);
								_ss.close();
							}

							std::string _socket_id = zpt::generate::r_uuid();
							_definition << "uuid" << _socket_id;
							std::string _connection = std::string("@tcp://*:") + std::to_string(_available) + std::string(",@inproc://") + _socket_id;
							zpt::socket _socket = this->__poll->bind(ZMQ_ROUTER_DEALER, _connection);
							_definition << "connect" << (_definition["public"]->is_string() ? _definition["public"]->str() : std::string(">tcp://*:") + std::to_string(_available));
							this->__router_dealer.push_back(_socket);
							zlog(std::string("starting 0mq listener for @tcp://*:") + std::to_string(_available), zpt::info);

							zlog(std::string("allocating ") + std::to_string(this->__max_threads) + std::string(" thread(s)"), zpt::info);
							std::string _in_connection = std::string(">inproc://") + std::string(_definition["uuid"]);
							for (size_t _k = 0; _k != this->__max_threads; _k++) {
								this->alloc_thread(_in_connection, false);
							}
							break;
						}
					}
				}
				default : {
					zpt::socket _socket = this->__poll->bind(_type, _definition["bind"]->str());
					_definition << "connect" << (_definition["public"]->is_string() ? _definition["public"]->str() : zpt::r_replace(_socket->connection(), "@tcp", ">tcp"));
					zlog(std::string("starting 0mq listener for ") + _socket->connection(), zpt::info);
					break;
				}
			}
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
					ztrace(std::string("loading module '") + _lib_file + std::string("'"));
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

		if (this->__options["rest"]["credentials"]["client_id"]->is_string() && this->__options["rest"]["credentials"]["client_secret"]->is_string() && this->__options["rest"]["credentials"]["server"]->is_string() && this->__options["rest"]["credentials"]["grant_type"]->is_string()) {
			zlog(std::string("going to retrieve credentials ") + std::string(this->__options["rest"]["credentials"]["client_id"]) + std::string(" @ ") + std::string(this->__options["rest"]["credentials"]["server"]), zpt::info);
			this->credentials(this->__emitter->gatekeeper()->get_credentials(this->__options["rest"]["credentials"]["client_id"], this->__options["rest"]["credentials"]["client_secret"], this->__options["rest"]["credentials"]["server"], this->__options["rest"]["credentials"]["grant_type"], this->__options["rest"]["credentials"]["scope"]));
			// zdbg(zpt::json::pretty(this->credentials()));
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
	this->__poll->unbind();
	this->__mqtt->unbind();
}

auto zpt::RESTServer::alloc_thread(std::string _in_connection, bool _temp) -> void {
	std::thread _worker(
		[ this ] (std::string _in_connection, bool _temp) -> void {
			zpt::ZMQRep* _socket = new zpt::ZMQRep(_in_connection, this->__options);
			for (;true;) {
				zpt::json _envelope = _socket->recv();
				if (bool(_envelope["error"])) {
					_envelope << 
					"channel" << "/bad-request" <<
					"performative" << zpt::ev::Reply <<
					"resource" << "/bad-request";
					_socket->send(_envelope);
				}
				if (_envelope->ok()) {
					zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);

					bool _spawn = false;
					{ std::lock_guard< std::mutex > _lock(this->__thread_mtx);
						this->__alloc_threads++;
						_spawn = (this->__alloc_threads >= this->__max_threads - 1) && (this->__n_threads < 2 * this->__max_threads); }
					if (_spawn) {
						this->alloc_thread(_in_connection, true);
					}
						
					zpt::json _result = this->__emitter->trigger(_performative, _envelope["resource"]->str(), _envelope);
					if (_result->ok()) {
						try {
							_result << 
							"channel" << _envelope["headers"]["X-Cid"] <<
							"performative" << zpt::ev::Reply <<
							"resource" << _envelope["resource"];
							_socket->send(_result);
						}
						catch(zpt::assertion& _e) {}
					}

					{ std::lock_guard< std::mutex > _lock(this->__thread_mtx);
						this->__alloc_threads--; }
				}
			}
		}
		, _in_connection, _temp
	);
	{ std::lock_guard< std::mutex > _lock(this->__thread_mtx);
		this->__n_threads++; }
	_worker.detach();
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

auto zpt::RESTServer::mutations() -> zpt::mutation::emitter {
	return this->__emitter->mutations();
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

void zpt::RESTServer::start(zpt::json _sems) {
	try {
		if (this->credentials()["endpoints"]["mqtt"]->ok() || (this->__options["mqtt"]->ok() && this->__options["mqtt"]["bind"]->ok())) {
			this->__mqtt->start();
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

					for (; true; ) {
						int _fd = -1;
						_ss->accept(& _fd);
				
						zpt::socketstream_ptr _cs(new zpt::socketstream(_fd));
						try {
							if (this->route_http(_cs)) {
								_cs->close();
							}
						}
						catch(zpt::assertion& _e) {
							zpt::http::rep _reply = zpt::rest::zmq2http(
								{
									"performative", zpt::ev::Reply,
									"status", 500,
									"headers", zpt::ev::init_reply(),
									"payload", {
										"text", _e.what(),
										"assertion_failed", _e.description(),
										"code", _e.code()
										}
								}
							);
							(*_cs) << _reply << flush;
							_cs->close();				
						}
					}
				}
			);
			_http.detach();
		}

		if (std::string(this->__options["$mutations"]["enabled"]) == "true") {
			std::thread _mutations(
				[ this ] () -> void {
					zlog(std::string("starting 0MQ mutation listener"), zpt::info);
					((zpt::RESTMutationEmitter*) this->events()->mutations().get())->loop();
				}
			);
			_mutations.detach();
		}

		for (auto _callback : this->__initializers) {
			_callback(this->__emitter);
		}

		if (_sems->ok()) {
			struct sembuf _dec[2] = { { (short unsigned int) 0, -1 }, { (short unsigned int) 1, -1 } };
			semop(int(_sems[0]), _dec, 2);	
		}
		std::string _NAME(this->__name.data());
		std::transform(_NAME.begin(), _NAME.end(), _NAME.begin(), ::toupper);
		zlog(std::string("loaded *") + _NAME + std::string("*"), zpt::notice);
		this->__poll->loop();
	}
	catch (zpt::InterruptedException& e) {
		return;
	}
}

bool zpt::RESTServer::route_http(zpt::socketstream_ptr _cs) {
	zpt::http::rep _reply;
	_reply->status(zpt::HTTP200);
	zpt::http::req _request;
	try {
		(*_cs) >> _request;
	}
	catch(zpt::SyntaxErrorException& _e) {
		zlog(std::string("error while parsing HTTP request: syntax error exception"), zpt::error);
		zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::bad_request());
		zverbose(std::string("sending HTTP reply:\n") + std::string(_reply));
		(*_cs) << _reply << flush;
		return true;
	}
	zverbose(std::string("received HTTP request:\n") + std::string(_request));

	bool _return = false;
	zpt::json _container = this->__emitter->lookup(_request->url());
	if (_container->ok()) {
		short _type = zpt::str2type(_container["type"]->str());
		zpt::json _in = zpt::rest::http2zmq(_request);
		switch(_type) {
			case ZMQ_REP :
			case ZMQ_REQ :
			case ZMQ_ROUTER_DEALER : {
				zpt::socket _client = this->__poll->bind(ZMQ_ASSYNC_REQ, _container["connect"]->str());
				_client->relay_for(_cs,
					[] (zpt::json _envelope) -> std::string {
						std::string _reply = std::string(zpt::rest::zmq2http(_envelope));
						zverbose(std::string("sending HTTP reply:\n") + std::string(_reply));
						return _reply;
					}
				);
				_client->send(_in);
				_return = false;
				break;
			}
			case ZMQ_PUB_SUB : {
				std::string _connect = _container["connect"]->str();
				zpt::socket _client = this->__poll->bind(ZMQ_PUB, _connect.substr(0, _connect.find(",")));
				_client->send(_in);
				zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::accepted(_request->url()));
				zverbose(std::string("sending HTTP reply:\n") + std::string(_reply));
				(*_cs) << _reply << flush;
				_return = true;
				_client->unbind();
				break;
			}
			case ZMQ_PUSH : {
				zpt::socket _client = this->__poll->bind(ZMQ_PUSH, _container["connect"]->str());
				_client->send(_in);
				zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::accepted(_request->url()));
				zverbose(std::string("sending HTTP reply:\n") + std::string(_reply));
				(*_cs) << _reply << flush;
				_return = true;
				_client->unbind();
				break;
			}
		}
	}
	else {
		zlog(std::string("error processing '") + _request->url() + std::string("': listener not found"), zpt::error);
		zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::not_found(_request->url()));
		zverbose(std::string("sending HTTP reply:\n") + std::string(_reply));
		(*_cs) << _reply << flush;
		_return = true;
	}
	return _return;
}

bool zpt::RESTServer::route_mqtt(zpt::mqtt::data _data) {
	zpt::json _envelope = zpt::json::object();
	if (!_data->__message["performative"]->ok()) {
		_envelope << "performative" << int(zpt::ev::Reply);
	}
	else {
		if (_data->__message["performative"]->is_string()) {
			_envelope << "performative" << zpt::ev::from_str(std::string(_data->__message["performative"]));
		}
		else {
			_envelope << "performative" << zpt::ev::performative(int(_data->__message["performative"]));
		}
	}
	if (!_data->__message["channel"]->ok()) {
		_envelope << "channel" << _data->__topic;
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
	ztrace(std::string("MQTT ") + std::string(_data->__topic));
	zverbose(_envelope);
	zpt::ev::performative _performative = (zpt::ev::performative) int(_envelope["performative"]);
	zpt::json _result = this->events()->trigger(_performative, std::string(_data->__topic), _envelope);
	if (_result->ok()) {
		ztrace(std::string("MQTT ") + std::string(_data->__topic));
		zverbose(_result);
		this->__mqtt->publish(std::string(_data->__topic), _result);
	}
	return true;
}

auto zpt::RESTServer::publish(std::string _topic, zpt::json _payload) -> void {
	ztrace(std::string("MQTT ") + _topic);
	zverbose(_payload);
	this->__mqtt->publish(_topic, _payload);
}

auto zpt::RESTServer::subscribe(std::string _topic, zpt::json _opts) -> void {
	if (bool(_opts["mqtt"])) {
		zpt::json _topics = this->get_subscription_topics(_topic);
		for (auto _t : _topics->arr()) {
			this->__mqtt->subscribe(_t->str());
		}
	}
}

auto zpt::RESTServer::get_subscription_topics(std::string _pattern) -> zpt::json {
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
			_return.push_back('#');
		}
		_topics << _return;
	}
	return _topics;
}

zpt::RESTClientPtr::RESTClientPtr(zpt::json _options) : std::shared_ptr<zpt::RESTClient>(new zpt::RESTClient(_options)) {
}

zpt::RESTClientPtr::RESTClientPtr(zpt::RESTClient * _ptr) : std::shared_ptr<zpt::RESTClient>(_ptr) {
}

zpt::RESTClientPtr::~RESTClientPtr() {
}

zpt::rest::client zpt::RESTClientPtr::launch(int argc, char* argv[]) {
	zpt::json _options = zpt::conf::rest::init(argc, argv)->obj()->begin()->second;
	if (!_options["rest"]->ok() || !_options["zmq"]->ok()) {
		std::cout << "unable to start client: unsufficient configurations" << endl << flush;
		exit(-10);
	}
	zpt::conf::setup(_options);
	zpt::rest::client _client(_options);
	return _client;
}

zpt::RESTClient::RESTClient(zpt::json _options) : __emitter((new zpt::RESTEmitter(_options))->self()), __poll((new zpt::ZMQPoll(_options, __emitter))->self()), __options(_options) {
	((zpt::RESTEmitter*) this->__emitter.get())->poll(this->__poll);
}

zpt::RESTClient::~RESTClient(){
}

zpt::poll zpt::RESTClient::poll() {
	return this->__poll;
}

zpt::ev::emitter zpt::RESTClient::events() {
	return this->__emitter;
}

zpt::mutation::emitter zpt::RESTClient::mutations() {
	return this->__emitter->mutations();
}

zpt::json zpt::RESTClient::options() {
	return this->__options;
}

zpt::socket zpt::RESTClient::bind(std::string _object_path) {
	zpt::json _zmq_cnf = this->__options->getPath(_object_path);
	short _type = zpt::str2type(_zmq_cnf["type"]->str());
	return this->bind(_type, _zmq_cnf["bind"]->str());
}

zpt::socket zpt::RESTClient::bind(short _type, std::string _connection) {
	return this->__poll->bind(_type, _connection);
}

void zpt::RESTClient::start() {
	try {
		this->__poll->loop();
	}
	catch (zpt::InterruptedException& e) {
		return;
	}
}

auto zpt::rest::http2zmq(zpt::http::req _request) -> zpt::json {
	zpt::json _return = zpt::json::object();
	_return <<
		"channel" << _request->url() <<
		"performative" << _request->method() <<
		"resource" << _request->url();
	
	zpt::json _payload;
	if (_request->body() != "") {
		if (_request->header("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos) {
			_payload = zpt::rest::http::deserialize(_request->body());
		}
		else if (_request->header("Content-Type").find("application/json") != std::string::npos) {
			try {
				_payload = zpt::json(_request->body());
			}
			catch(zpt::SyntaxErrorException& _e) {
			}
		}
		else {
			_payload = { "text", _request->body() };
		}
	}
	else {
		_payload = zpt::json::object();
	}
	_return << "payload" << _payload;

	if (_request->params().size() != 0) {
		zpt::json _params = zpt::json::object();
		for (auto _param : _request->params()) {
			_params << _param.first << zpt::url::r_decode(_param.second);
		}
		_return << "params" << _params;
	}
	
	zpt::json _headers = zpt::json::object();
	for (auto _header : _request->headers()) {
		_headers << _header.first << _header.second;
	}
	if (_headers->obj()->size() != 0) {
		_return << "headers" << _headers;
	}
	
	return _return;
}

auto zpt::rest::http2zmq(zpt::http::rep _reply) -> zpt::json {
	zpt::json _return = zpt::json::object();
	_return <<
	"status" << (int) _reply->status() <<
	"channel" << zpt::generate::r_uuid() <<
	"performative" << zpt::ev::Reply <<
	"resource" << zpt::generate::r_uuid();
	
	zpt::json _payload;
	if (_reply->body() != "") {
		if (_reply->header("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos) {
			_payload = zpt::rest::http::deserialize(_reply->body());
		}
		else if (_reply->header("Content-Type").find("application/json") != std::string::npos) {
			try {
				_payload = zpt::json(_reply->body());
			}
			catch(zpt::SyntaxErrorException& _e) {
			}
		}
		else {
			_payload = { "text", _reply->body() };
		}
	}
	else {
		_payload = zpt::json::object();
	}
	_return << "payload" << _payload;

	zpt::json _headers = zpt::json::object();
	for (auto _header : _reply->headers()) {
		_headers << _header.first << _header.second;
	}
	if (_headers->obj()->size() != 0) {
		_return << "headers" << _headers;
	}
	
	return _return;
}

auto zpt::rest::zmq2http(zpt::json _out) -> zpt::http::rep {
	zpt::http::rep _return;
	_return->status((zpt::HTTPStatus) ((int) _out["status"]));
	
	_out << "headers" << (zpt::ev::init_reply() + _out["headers"]);
	for (auto _header : _out["headers"]->obj()) {
		_return->header(_header.first, ((std::string) _header.second));
	}
	
	if (_out["payload"]->ok()) {
		if (_out["payload"]->is_object() || _out["payload"]->is_array()) {
			_return->header("Content-Type", "application/json");
		}
		else {
			_return->header("Content-Type", "text/plain");
		}
		std::string _body = (std::string) _out["payload"];
		_return->body(_body);
		_return->header("Content-Length", std::to_string(_body.length()));
	}
	
	return _return;
}

zpt::json zpt::rest::http::deserialize(std::string _body) {
	zpt::json _return = zpt::json::object();
	std::string _name;
	std::string _collected;
	for (const auto& _c : _body) {
		switch(_c) {
			case '=' : {
				_name.assign(_collected.data());
				_collected.assign("");
				break;
			}
			case '&' : {
				zpt::url::decode(_collected);
				_return << _name << _collected;
				_name.assign("");
				_collected.assign("");
				break;
			}
			default : {
				_collected.push_back(_c);
			}
		}
	}
	zpt::url::decode(_collected);
	_return << _name << _collected;
	return _return;
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
		return std::string(zpt::split(_envelope["headers"]["Authorization"], " ")[1]);
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
		return std::string(zpt::split(_envelope["headers"]["Cookie"], ";")[0]);
	}
	return "";
}

auto zpt::rest::authorization::headers(std::string _token) -> zpt::json {
	return { "Authorization", (std::string("OAuth2.0 ") + _token) };
}

auto zpt::rest::authorization::validate(std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter, zpt::json _roles_needed) -> zpt::json {
	return _emitter->authorize(_topic, _envelope, _roles_needed);
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
	if (_ptr->type() == zpt::JSObject) {
		for (auto _proc : _ptr->obj()) {
			if (_proc.first.find("$") == std::string::npos) {
				_proc.second << "argv" << _args;
			}
		}
	}

	zpt::log_lvl = _log_level;
	return _ptr;
}
