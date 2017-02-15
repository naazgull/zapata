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
	zpt::rest::server _server(_name, _options);
	return _server;
}

auto zpt::rest::terminate(int _signal) -> void {
	if (zpt::rest::pids == nullptr) {
		return;
	}
	semctl(zpt::rest::m_sem, 0, IPC_RMID);
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

	zpt::json _to_spawn = zpt::json::object();
	for (auto _spawn : _ptr->obj()) {
		if (_spawn.first.find("$") != std::string::npos) {
			continue;
		}
		if (_spawn.second["enabled"]->ok() && !((bool) _spawn.second["enabled"])) {
			continue;
		}
		_to_spawn << _spawn.first << _spawn.second;
	}

	zpt::rest::n_pid = 0;
	zpt::rest::pids = new pid_t[_to_spawn->obj()->size()];
		
	if (bool(_ptr["$mutations"]["run"])) {
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
	for (auto _spawn : _to_spawn->obj()) {
		if (_spawned == _to_spawn->obj()->size() - 1) {
			_name.assign(_spawn.first.data());
			_options = _spawn.second;
			::signal(SIGINT, zpt::rest::terminate);
			::signal(SIGTERM, zpt::rest::terminate);
			::signal(SIGABRT, zpt::rest::terminate);
			::signal(SIGSEGV, zpt::rest::terminate);
		}
		else {
			pid_t _pid = fork();
			if (_pid == 0) {
				_name.assign(_spawn.first.data());
				_options = _spawn.second;
				break;
			}
			else {
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
		pid_t _pid = fork();
		if (_pid == 0) {
			break;
		}
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
	if (_options["log"]["file"]->ok()) {
		zpt::log_fd = new std::ofstream();
		((std::ofstream *) zpt::log_fd)->open(((std::string) _options["log"]["file"]).data(), std::ofstream::out | std::ofstream::app | std::ofstream::ate);
	}
	
	zlog(std::string("starting RESTful service container: ") + _name, zpt::warning);
	zpt::rest::server _server = zpt::rest::server::setup(_options, _name);
	_server->start();

	return 0;
}

zpt::RESTServer::RESTServer(std::string _name, zpt::json _options) : __name(_name), __emitter((new zpt::RESTEmitter(_options))->self()), __poll((new zpt::ZMQPoll(_options, __emitter))->self()), __options(_options), __self(this) {
	assertz(this->__options["zmq"]->ok() && this->__options["zmq"]->type() == zpt::JSArray && this->__options["zmq"]->arr()->size() != 0, "zmq settings (bind, type) must be provided in the configuration file", 500, 0);
	((zpt::RESTEmitter*) this->__emitter.get())->poll(this->__poll);
	((zpt::RESTEmitter*) this->__emitter.get())->server(this->__self);

	for (auto _definition : this->__options["zmq"]->arr()) {
		short int _type = zpt::str2type(_definition["type"]->str());

		switch(_type) {
			case ZMQ_ROUTER_DEALER : {
				zpt::socket _socket = this->__poll->bind(ZMQ_ROUTER_DEALER, _definition["bind"]->str());
				_definition << "connect" << zpt::r_replace(_socket->connection(), "@tcp", ">tcp");
				this->__router_dealer.push_back(_socket);
				break;
			}
			case ZMQ_PUB_SUB : {
				zpt::socket _socket = this->__poll->bind(ZMQ_XPUB_XSUB, _definition["bind"]->str());
				_definition << "connect" << zpt::r_replace(_socket->connection(), "@tcp", ">tcp");
				this->__pub_sub.push_back(_socket);
				break;
			}
			default : {
				zpt::socket _socket = this->__poll->bind(_type, _definition["bind"]->str());
				_definition << "connect" << zpt::r_replace(_socket->connection(), "@tcp", ">tcp");
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

	if (!!this->__options["rest"]["uploads"]["upload_controller"]) {
		/*
		 *  definition of handlers for the file upload controller
		 *  registered as a Controller
		 */
		this->__emitter->on(zpt::ev::Post, zpt::rest::url_pattern({ this->__emitter->version(), "files" }),
			[] (zpt::ev::performative _performative, std::string _resource, zpt::json _payload, zpt::ev::emitter _pool) -> zpt::json {
				return zpt::undefined;
			}
		);

		/*
		 *  definition of handlers for the file upload removal controller
		 *  registered as a Controller
		 */
		this->__emitter->on(zpt::ev::Delete, zpt::rest::url_pattern({ this->__emitter->version(), "files", "(.+)" }),
			[] (zpt::ev::performative _performative, std::string _resource, zpt::json _payload, zpt::ev::emitter _pool) -> zpt::json {
				return zpt::undefined;
			}
		);
	}

}

zpt::RESTServer::~RESTServer(){
	zpt::ZMQPoll* _poll = this->__poll.get();
	this->__poll.reset();
	_poll->unbind();
	this->__self.reset();
}

std::string zpt::RESTServer::name() {
	return this->__name;
}

zpt::poll zpt::RESTServer::poll() {
	return this->__poll;
}

zpt::ev::emitter zpt::RESTServer::events() {
	return this->__emitter;
}

zpt::mutation::emitter zpt::RESTServer::mutations() {
	return this->__emitter->mutations();
}

zpt::json zpt::RESTServer::options() {
	return this->__options;
}

void zpt::RESTServer::start() {
	try {
		if (this->__options["http"]->ok() && this->__options["http"]["bind"]->ok() && this->__options["http"]["port"]->ok()) {
			std::shared_ptr< std::thread > _http(
				new std::thread(
					[ this ] () -> void {
						zlog(std::string("starting HTTP listener on port ") + std::to_string((uint) this->__options["http"]["port"]), zpt::notice);

						zpt::serversocketstream_ptr _ss(new zpt::serversocketstream());
						_ss->bind((uint) this->__options["http"]["port"]);

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
				)
			);
			this->__threads.push_back(_http);
		}
		
		if (bool(this->__options["$mutations"]["enabled"])) {
			std::shared_ptr< std::thread > _mutations(
				new std::thread(
					[ this ] () -> void {
						zlog(std::string("starting 0MQ mutation listener"), zpt::notice);
						((zpt::ZMQMutationEmitter*) this->events()->mutations().get())->loop();
					}
				)
			);
			this->__threads.push_back(_mutations);
		}

		this->__poll->loop();
		for (auto _thread : this->__threads) {
			_thread->join();
		}
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
		assertz(false, "error parsing HTTP data", 500, 0);
	}

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
						return std::string(zpt::rest::zmq2http(_envelope));
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
				(*_cs) << _reply << flush;
				_return = true;
				_client->unbind();
				break;
			}
			case ZMQ_PUSH : {
				zpt::socket _client = this->__poll->bind(ZMQ_PUSH, _container["connect"]->str());
				_client->send(_in);
				zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::accepted(_request->url()));
				(*_cs) << _reply << flush;
				_return = true;
				_client->unbind();
				break;
			}
		}
	}
	else {
		zlog(std::string("didn't produce anything for HTTP request"), zpt::trace);
		zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::not_found(_request->url()));
		(*_cs) << _reply << flush;
		_return = true;
	}
	return _return;
}

bool zpt::RESTServer::route_mqtt(std::iostream& _cs) {
	return true;
}

auto zpt::RESTServer::assync_on(std::string _regex, zpt::json _opts) -> void {
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
	for (auto _param : _request->params()) {
		_payload << _param.first << _param.second;
	}
	_return << "payload" << _payload;

	zpt::json _headers = zpt::json::object();
	for (auto _header : _request->headers()) {
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
	
	if (_out["payload"]->ok() && _out["payload"]->obj()->size() != 0) {
		std::string _body = (std::string) _out["payload"];
		_return->body(_body);
		_return->header("Content-Type", "application/json");
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
	if (_envelope["headers"]["Cookie"]->ok()) {
		return std::string(zpt::split(_envelope["headers"]["Cookie"], ";")[0]);
	}
	return "";
}

auto zpt::rest::authorization::headers(std::string _token) -> zpt::json {
	return { "Authorization", (std::string("OAuth2.0 ") + _token) };
}

auto zpt::rest::authorization::validate(zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
	return _emitter->authorize(_envelope);
}

auto zpt::conf::rest::init(int argc, char* argv[]) -> zpt::json {
	zpt::log_fd = &std::cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new string(argv[0]);

	zpt::json _args = zpt::conf::getopt(argc, argv);
	
	short _log_level = (_args["l"]->ok() ? int(_args["l"][0]) : -1);
	std::string _conf_file = (_args["c"]->ok() ? std::string(_args["c"][0]) : "");
	zpt::log_format = (bool(_args["r"]) ? 0 : (bool(_args["j"]) ? 2 : 1));

	zpt::json _ptr;
	if (_conf_file.length() == 0) {
		zlog("loading default configuration file '/etc/zapata/zapata.conf'", zpt::warning);
		_conf_file.assign("/etc/zapata/zapata.conf");
	}

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
	if (_ptr->type() == zpt::JSObject) {
		for (auto _proc : _ptr->obj()) {
			if (_proc.first.find("$") == std::string::npos) {
				_proc.second << "argv" << _args;
			}
		}
	}

	zpt::log_lvl = _log_level;
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

	return _ptr;
}
