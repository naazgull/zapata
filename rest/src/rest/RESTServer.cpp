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
	zpt::conf::setup(_options);
	zpt::rest::server _server(_name, _options);
	return _server;
}

int zpt::RESTServerPtr::launch(int argc, char* argv[]) {
	zpt::json _ptr = zpt::conf::init(argc, argv);
	zpt::json _to_spawn = zpt::json::object();
	for (auto _spawn : _ptr->obj()) {
		if (_spawn.second["enabled"]->ok() && !((bool) _spawn.second["enabled"])) {
			continue;
		}
		_to_spawn << _spawn.first << _spawn.second;
	}		
		
	size_t _spawned = 0;
	std::string _name;
	zpt::json _options;
	for (auto _spawn : _to_spawn->obj()) {
		if (_spawned == _to_spawn->obj()->size() - 1) {
			_name.assign(_spawn.first.data());
			_options = _spawn.second;
		}
		else {
			pid_t _pid = fork();
			if (_pid == 0) {
				_name.assign(_spawn.first.data());
				_options = _spawn.second;
				break;
			}
			else {
				_spawned++;
			}
		}
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
	
	zlog(std::string("starting RESTful server instance: ") + _name, zpt::alert);
	zpt::rest::server _server = zpt::rest::server::setup(_options, _name);
	_server->start();

	return 0;
}

zpt::RESTServer::RESTServer(std::string _name, zpt::json _options) : __name(_name), __emitter( new zpt::RESTEmitter(_options)), __poll(new zpt::ZMQPoll(_options, __emitter)), __options(_options) {
	assertz(this->__options["zmq"]->ok() && this->__options["zmq"]->type() == zpt::JSArray && this->__options["zmq"]->arr()->size() != 0, "zmq settings (bind, type) must be provided in the configuration file", 500, 0);
	((zpt::RESTEmitter*) this->__emitter.get())->poll(this->__poll);

	for (auto _definition : this->__options["zmq"]->arr()) {
		short int _type = zpt::str2type(_definition["type"]->str());

		switch(_type) {
			case ZMQ_ROUTER_DEALER : {
				this->__router_dealer.push_back(this->__poll->bind(ZMQ_ROUTER_DEALER, _definition["bind"]->str()));
				break;
			}
			case ZMQ_PUB_SUB : {
				this->__pub_sub.push_back(this->__poll->bind(ZMQ_XPUB_XSUB, _definition["bind"]->str()));
				break;
			}
			default : {
				this->__poll->bind(_type, _definition["bind"]->str());
			}
		}
	}

	if (this->__options["rest"]["modules"]->ok()) {
		for (auto _i : this->__options["rest"]["modules"]->arr()) {
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
					void (*_populate)(zpt::ev::emitter);
					_populate = (void (*)(zpt::ev::emitter)) dlsym(hndl, "restify");
					_populate(this->__emitter);
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
}

std::string zpt::RESTServer::name() {
	return this->__name;
}

zpt::poll zpt::RESTServer::poll() {
	return this->__poll;
}

zpt::ev::emitter zpt::RESTServer::emitter() {
	return this->__emitter;
}

zpt::json zpt::RESTServer::options() {
	return this->__options;
}

void zpt::RESTServer::start() {
	try {
		if (this->__options["http"]->ok() && this->__options["http"]["bind"]->ok() && this->__options["http"]["port"]->ok()) {
			std::shared_ptr< std::thread > _http(
				new std::thread(
					[ & ] () -> void {
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
							catch(zpt::AssertionException& _e) {
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
	_reply->status((zpt::HTTPStatus) 200);
	zpt::http::req _request;
	try {
		(*_cs) >> _request;
	}
	catch(zpt::SyntaxErrorException& _e) {
		assertz(false, "error parsing HTTP data", 500, 0);
	}

	bool _return = false;
	bool _api_found = false;
	std::string _prefix(_request->url());
	if (this->__options["directory"]->ok()) {
		for (auto _api : this->__options["directory"]->obj()) {
			bool _endpoint_found = false;
			for (auto _endpoint : _api.second["endpoints"]->arr()) {
				if (_prefix.find(_endpoint->str()) == 0) {
					_endpoint_found = true;
					_api_found = true;
					short _type = zpt::str2type(_api.second["type"]->str());
					zpt::json _in = zpt::rest::http2zmq(_request);
					switch(_type) {
						case ZMQ_REP :
						case ZMQ_REQ :
						case ZMQ_ROUTER_DEALER : {
							zpt::socket _client = this->__poll->bind(ZMQ_ASSYNC_REQ, _api.second["connect"]->str());
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
							std::string _connect = _api.second["connect"]->str();
							zpt::socket _client = this->__poll->bind(ZMQ_PUB, _connect.substr(0, _connect.find(",")));
							_client->send(_in);
							zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::accepted(_prefix));
							(*_cs) << _reply << flush;
							_return = true;
							_client->unbind();
							break;
						}
						case ZMQ_PUSH : {
							zpt::socket _client = this->__poll->bind(ZMQ_PUSH, _api.second["connect"]->str());
							_client->send(_in);
							zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::accepted(_prefix));
							(*_cs) << _reply << flush;
							_return = true;
							_client->unbind();
							break;
						}
					}
					break;
				}
			}
			if (_endpoint_found) {
				break;
			}
		}
	}
				
	if (!_api_found) {
		zlog(string("-> | \033[33;40m") + zpt::method_names[_request->method()] + string("\033[0m ") + _request->url(), zpt::info);
		zlog(string("<- | \033[31;40m404\033[0m"), zpt::info);
		zpt::http::rep _reply = zpt::rest::zmq2http(zpt::rest::not_found(_prefix));
		(*_cs) << _reply << flush;
		_return = true;
	}
	return _return;
}

bool zpt::RESTServer::route_mqtt(std::iostream& _cs) {
	return true;
}

zpt::RESTClientPtr::RESTClientPtr(zpt::json _options) : std::shared_ptr<zpt::RESTClient>(new zpt::RESTClient(_options)) {
}

zpt::RESTClientPtr::RESTClientPtr(zpt::RESTClient * _ptr) : std::shared_ptr<zpt::RESTClient>(_ptr) {
}

zpt::RESTClientPtr::~RESTClientPtr() {
}

zpt::rest::client zpt::RESTClientPtr::launch(int argc, char* argv[]) {
	zpt::json _options = zpt::conf::init(argc, argv)->obj()->begin()->second;
	if (!_options["rest"]->ok() || !_options["zmq"]->ok()) {
		std::cout << "unable to start client: unsufficient configurations" << endl << flush;
		exit(-10);
	}
	zpt::conf::setup(_options);
	zpt::rest::client _client(_options);
	return _client;
}

zpt::RESTClient::RESTClient(zpt::json _options) : __emitter( new zpt::RESTEmitter(_options)), __poll(new zpt::ZMQPoll(_options, __emitter)), __options(_options) {
	((zpt::RESTEmitter*) this->__emitter.get())->poll(this->__poll);
}

zpt::RESTClient::~RESTClient(){
}

zpt::poll zpt::RESTClient::poll() {
	return this->__poll;
}

zpt::ev::emitter zpt::RESTClient::emitter() {
	return this->__emitter;
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

zpt::json zpt::rest::http2zmq(zpt::http::req _request) {
	zpt::json _return = zpt::json::object();
	_return <<
		"channel" << _request->url() <<
		"performative" << _request->method() <<
		"resource" << _request->url();
	
	zpt::json _payload;
	if (_request->body() != "") {
		if (_request->header("Content-Type").find("application/x-www-form-urlencoded") != string::npos) {
			_payload = zpt::rest::http::deserialize(_request->body());
		}
		else if (_request->header("Content-Type").find("application/json") != string::npos) {
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

zpt::http::rep zpt::rest::zmq2http(zpt::json _out) {
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

std::string zpt::rest::authorization::serialize(zpt::json _info) {
	assertz(
		_info["owner"]->type() == zpt::JSString &&
		_info["application"]->type() == zpt::JSString &&
		_info["grant_type"]->type() == zpt::JSString,
		"token serialization failed: required fields are 'owner', 'application' and 'grant_type'", 412, 0);
	return _info["owner"]->str() + std::string("@") + _info["application"]->str() + std::string("/") + _info["grant_type"]->str() + std::string("/") + (_info["key"]->type() == zpt::JSString ? _info["key"]->str() : zpt::generate_key(64));
}

zpt::json zpt::rest::authorization::deserialize(std::string _token) {
	zpt::json _return = zpt::json::object();

	zpt::json _splitted = zpt::split(_token, "@");
	_return << "owner" << _splitted[0];

	_splitted = zpt::split(_splitted[1], "/");
	_return << "application" << _splitted[0] << "grant_type" << _splitted[1] << "key" << _splitted[2];

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

std::string zpt::rest::authorization::extract(zpt::json _envelope) {
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
	return zpt::undefined;
}
