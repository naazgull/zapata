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

#include <zapata/zmq/ZMQPolling.h>
#include <systemd/sd-daemon.h>
#include <future>
#include <chrono>

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options, zpt::ev::emitter _emiter) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options, _emiter)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::ZMQPoll * _ptr) : std::shared_ptr<zpt::ZMQPoll>(_ptr) {
}

zpt::ZMQPollPtr::~ZMQPollPtr() {
}

zpt::ZMQPoll::ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter) : __options( _options), __id(0), __self(this), __emitter(_emiter),__needs_rebuild(true)/*, __context(0)*/ {
	std::string _uuid = zpt::generate::r_uuid();
	std::string _connection(std::string("inproc://") + _uuid);
	this->__sync[0] = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_REP));
	this->__sync[0]->bind(_connection);
	this->__sync[1] = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_REQ));
	this->__sync[1]->connect(_connection);

}

zpt::ZMQPoll::~ZMQPoll() {
	for (short _i = 0; _i != 4; _i++) {
		this->__sync[_i]->close();
	}
	zlog(string("zmq poll clean up"), zpt::notice);
}

auto zpt::ZMQPoll::options() -> zpt::json {
	return this->__options;
}

auto zpt::ZMQPoll::emitter() -> zpt::ev::emitter {
	return this->__emitter;
}

auto zpt::ZMQPoll::self() const -> zpt::ZMQPollPtr {
	return this->__self;
}

auto zpt::ZMQPoll::get(std::string _key) -> zpt::socket_ref {
	return zpt::socket_ref(zpt::r_replace(_key, "*", ((string) this->__options["host"])), this->self());
}

auto zpt::ZMQPoll::relay(std::string _key) -> zpt::ZMQ* {
	{ std::lock_guard< std::mutex > _lock(this->__mtx[0]);
		auto _found = this->__by_refs.find(_key);
		if (_found != this->__by_refs.end()) {
			return _found->second;
		} }
	return nullptr;
}

auto zpt::ZMQPoll::add(short _type, std::string _connection, bool _new_connection) -> zpt::socket_ref {
	std::string _key;

	if (!_new_connection) {
		_key = std::string(_connection.data());
		if (_key.front() == '@') {
			_key[0] = '>';
		}
		else if (_key.front() != '>') {
			_key = '>' + _key;
		}
		if (this->relay(_key) != nullptr) {
			//zdbg(std::string("already connected socket ") + _key);
			return zpt::socket_ref(_key, this->self());
		}
	}

	zpt::ZMQ* _underlying = this->bind(_type, _connection);
	assertz(_underlying != nullptr, std::string("could not connection to ") + zpt::type2str(_type) + std::string("@") + _connection, 500, 0);
	
	if (_new_connection) {
		_key.assign(_underlying->id());
	}
	
	{ std::lock_guard< std::mutex > _lock(this->__mtx[0]);
		this->__by_refs.insert(std::make_pair(_key, _underlying)); }
	return zpt::socket_ref(_key, this->self());
}

auto zpt::ZMQPoll::add(zpt::ZMQ* _underlying) -> zpt::socket_ref {
	std::string _key = _underlying->id();
	
	if (this->relay(_key) != nullptr) {
		return zpt::socket_ref(_key, this->self());
	}

	{ std::lock_guard< std::mutex > _lock(this->__mtx[0]);
		this->__by_refs.insert(std::make_pair(_key, _underlying)); }
	return zpt::socket_ref(_key, this->self());
}

auto zpt::ZMQPoll::vanished(std::string _connection) -> void {
	std::string _key(_connection.data());
	if (_key.front() == '@') {
		_key[0] = '>';
	}
	else if (_key.front() != '>') {
		_key = '>' + _key;
	}

	if (this->relay(_key) != nullptr) {
		zpt::socket_ref _socket(_key, this->self());
		this->__to_remove.insert(std::make_pair(_socket, "zpt::ZMQPoll::vanished"));
		this->__needs_rebuild = true;
	}
	
}

auto zpt::ZMQPoll::remove(zpt::socket_ref _socket) -> void {
	std::string _key(_socket.data());

	{ std::lock_guard< std::mutex > _lock(this->__mtx[0]);
		auto _found = this->__by_refs.find(_key);
		if (_found != this->__by_refs.end()) {
			_found->second->close();
			delete _found->second;
			this->__by_refs.erase(_found);
		} }		
}

auto zpt::ZMQPoll::signal(std::string _key) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx[1]);
	zmq::message_t _frame(_key.length());
	memcpy(_frame.data(), _key.data(), _key.length());
	this->__sync[1]->send(_frame);
	this->wait();
}

auto zpt::ZMQPoll::notify(std::string _key) -> void {
	zmq::message_t _frame(_key.length());
	memcpy(_frame.data(), _key.data(), _key.length());
	this->__sync[0]->send(_frame);
}

auto zpt::ZMQPoll::wait() -> void {
	zmq::message_t _frame;
	this->__sync[1]->recv(&_frame);
}

auto zpt::ZMQPoll::poll(zpt::socket_ref _socket) -> void {
	if (this->__id) {
		if (this->__id != pthread_self()) {
			this->signal(_socket.data());
		}
		else {
			for (auto _already : this->__by_socket) {
				if (_already == _socket) {
					return;
				}
			}
			this->__to_add.insert(std::make_pair(_socket, "zpt::ZMQPoll::poll"));
			this->__needs_rebuild = true;
		}
	}
	else {
		this->__by_socket.push_back(_socket);
		this->__needs_rebuild = true;
	}
}

auto zpt::ZMQPoll::repoll() -> void {
	if (!this->__needs_rebuild) {
		return;
	}
	// zdbg("rebuilding");
	this->__items.clear();
	this->__items.resize(0);
	
	for (size_t _k = 0; _k != this->__by_socket.size(); _k++) {
		if (this->__by_socket[_k]->in().get() != nullptr) {
			this->__items.push_back({ (void*)(*this->__by_socket[_k]->in()), 0, ZMQ_POLLIN, 0 });
		}
		else {
			this->__items.push_back({ 0, this->__by_socket[_k]->fd(), ZMQ_POLLIN, 0 });
		}
	}
	this->__items.push_back({ (void*)(*this->__sync[0]), 0, ZMQ_POLLIN, 0 });
	this->__needs_rebuild = false;
}

auto zpt::ZMQPoll::reply(zpt::json _envelope, zpt::socket_ref _socket) -> void {
	zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);

	if (_performative == zpt::ev::Reply) {
		try {
			this->__emitter->reply(_envelope, _envelope);
		}
		catch(zpt::assertion& _e) {
			zlog(std::string("uncaught assertion while processing response, unable to proceed: ") + _e.what() + std::string(": ") + _e.description(), zpt::emergency);
		}
		catch(std::exception& _e) {
			zlog(std::string("uncaught exception while processing response, unable to proceed: ") + _e.what(), zpt::emergency);
		}
	}
	else {
		try {
			this->__emitter->trigger(_performative, _envelope["resource"]->str(), _envelope, zpt::undefined,
				[ = ] (zpt::ev::performative _p_performative, std::string _p_topic, zpt::json _result, zpt::ev::emitter _p_emitter) mutable -> void {
					if (_result->ok()) {
						if (*_socket != nullptr) {
							_result = zpt::json{ "headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + _p_emitter->options()["$defaults"]["headers"]["response"] } + _result + 
							zpt::json{ 
								"channel", _envelope["channel"],
								"performative", zpt::ev::Reply,
								"resource", _envelope["resource"]
							};
							_socket->send(_result);
							this->clean_up(_socket, _envelope["headers"]["Connection"] == zpt::json::string("close"));
						}
					}
				}
			);
		}
		catch(zpt::assertion& _e) {
			if (*_socket != nullptr) {
				_socket->send(zpt::ev::assertion_error(std::string(_envelope["resource"]), _e, zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + this->options()["$defaults"]["headers"]["response"]) + zpt::json{ "channel", _envelope["channel"] });
				this->clean_up(_socket, _envelope["headers"]["Connection"] == zpt::json::string("close"));
			}
		}
		catch(std::exception& _e) {
			if (*_socket != nullptr) {
				_socket->send(zpt::ev::internal_server_error(std::string(_envelope["resource"]), _e, zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + this->options()["$defaults"]["headers"]["response"]) + zpt::json{ "channel", _envelope["channel"] });
				this->clean_up(_socket, _envelope["headers"]["Connection"] == zpt::json::string("close"));
			}
		}
	}
			
}

auto zpt::ZMQPoll::loop() -> void {
	try {
		this->__id = pthread_self();

		uint64_t _sd_watchdog_usec = 100000;
		bool _sd_watchdog_enabled = sd_watchdog_enabled(0, &_sd_watchdog_usec) != 0;
		zlog(std::string("watchdog flag is ") + (_sd_watchdog_enabled ? std::string("enabled") + std::string(" and timeout is set to ") + std::to_string(_sd_watchdog_usec / 1000 / 1000) + std::string(" seconds") : std::string("disabled")), zpt::notice);
		
		for(; true; ) {
			this->repoll();
			// zdbg(std::string("socket list size is ") + std::to_string(this->__items.size()));
			
			int _n_events = 0;
			_n_events = zmq::poll(&this->__items[0], this->__items.size(), _sd_watchdog_usec / 1000 / 2);
			if (_sd_watchdog_enabled) {
				sd_notify(0, "WATCHDOG=1");
			}

			if (_n_events == 0) {
				for (size_t _k = 0; _k != this->__items.size() - 1; _k++) {
					zpt::socket_ref _socket = this->__by_socket[_k];
					_socket->loop_iteration();
				}
				continue;
			}
			// zdbg(std::string("events ") + std::to_string(_n_events));
			
			for (size_t _k = 0; _k != this->__items.size() - 1; _k++) {
				if (this->__items[_k].revents & ZMQ_POLLIN) {
					//zdbg(std::string("communication event on ") + std::to_string(_k));
					zpt::socket_ref _socket = this->__by_socket[_k];
					zverbose(std::string("poll event on ") + _socket->protocol() + std::string("/") + _socket->connection());
					if (!_socket->available()) {
						// zdbg(std::string("could not consume data from socket: ") + _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					}

					zpt::json _envelope;
					try {
						_envelope = _socket->recv();
					}
					catch (zpt::assertion& _e) {
						// zdbg(std::string("could not consume data from socket: ") + _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					}
					catch (zmq::error_t& _e) {
						// zdbg(std::string("could not consume data from socket: ") + _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					}
					catch (std::exception& _e) {
						// zdbg(std::string("could not consume data from socket: ") + _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					}

					if (bool(_envelope["error"])) {
						if (*_socket != nullptr) {
							_socket->send(_envelope +
								zpt::json{
									"channel", _envelope["channel"],
									"performative", zpt::ev::Reply,
									"status", 400,
									"resource", "/bad-request"
								}
							);
							// zdbg(std::string("could not consume data from socket: ") + _socket->protocol() + std::string(" ") + _socket->connection());
							this->clean_up(_socket);
							continue;
						}
					}
					else if (_envelope->ok()) {
						_socket->loop_iteration();
						this->reply(_envelope, _socket);
					}
				}
				else {
					this->__by_socket[_k]->loop_iteration();
				}
			}

			if (this->__items[this->__items.size() - 1].revents & ZMQ_POLLIN) {
				zmq::message_t _frame;
				this->__sync[0]->recv(&_frame);
				
				std::string _uuid(std::string(static_cast<char*>(_frame.data()), _frame.size()));
				try {
					zpt::socket_ref _socket = this->get(_uuid);
					this->__to_add.insert(std::make_pair(_socket, "zpt::ZMQPoll::loop"));
				}
				catch(zpt::assertion& _e) {
				}
				this->notify(_uuid);
			}

			for (auto _pair : this->__to_remove) {
				zpt::socket_ref _socket = _pair.first;
				// zdbg(std::string("removing socket ") + _socket + std::string(" (") + _pair.second + std::string(")"));
				for (size_t _k = 0; _k != this->__by_socket.size(); _k++) {
					if (this->__by_socket[_k] == _socket) {
						this->__by_socket.erase(this->__by_socket.begin() + _k);
						break;
					}
				}
				_socket->close();
				this->remove(_socket);
				this->__needs_rebuild = true;
			}
			this->__to_remove.clear();

			for (auto _pair : this->__to_add) {
				zpt::socket_ref _socket = _pair.first;
				// zdbg(std::string("adding socket ") + _socket);
				this->__by_socket.push_back(_socket);
				this->__needs_rebuild = true;
			}
			this->__to_add.clear();
		}
	}
	catch(zpt::assertion& _e) {
		zlog(_e.what() + std::string(": ") + _e.description() + std::string("\n") + _e.backtrace(), zpt::emergency);
		exit(-1);
	}
}

auto zpt::ZMQPoll::bind(short _type, std::string _connection) -> zpt::ZMQ* {
	switch(_type) {
		case ZMQ_ROUTER_DEALER : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}			
			zpt::ZMQ* _socket = new zpt::ZMQRouterDealer(_connection, this->__options);
			return _socket;
		}
		case ZMQ_ROUTER : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}			
			zpt::ZMQ* _socket = new zpt::ZMQRouter(_connection, this->__options);
			return _socket;
		}
		case ZMQ_DEALER : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}			
			zpt::ZMQ* _socket = new zpt::ZMQDealer(_connection, this->__options);
			return _socket;
		}
		case ZMQ_REQ : {
			zpt::ZMQ* _socket = new zpt::ZMQReq(_connection, this->__options);
			return _socket;
		}
		case ZMQ_REP : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}			
			zpt::ZMQ* _socket = new zpt::ZMQRep(_connection, this->__options);
			return _socket;
		}
		case ZMQ_XPUB_XSUB : {
			zpt::ZMQ* _socket = new zpt::ZMQXPubXSub(_connection, this->__options);
			return _socket;
		}
		case ZMQ_PUB_SUB : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}
			zpt::ZMQ* _socket = new zpt::ZMQPubSub(_connection, this->__options);
			return _socket;
		}
		case ZMQ_PUB : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}
			zpt::ZMQ* _socket = new zpt::ZMQPub(_connection, this->__options);
			return _socket;
		}
		case ZMQ_SUB : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}
			zpt::ZMQ* _socket = new zpt::ZMQSub(_connection, this->__options);
			return _socket;
		}
		case ZMQ_PUSH : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}
			zpt::ZMQ* _socket = new zpt::ZMQPush(_connection, this->__options);
			return _socket;
		}
		case ZMQ_PULL : {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				return *_found;
			}
			zpt::ZMQ* _socket = new zpt::ZMQPull(_connection, this->__options);
			return _socket;
		}
		case ZMQ_HTTP_RAW : {
			zpt::json _uri = zpt::uri::parse(_connection);
			bool _is_ssl = _uri["scheme"] == zpt::json::string("https");
			zpt::socketstream_ptr _cs;
			_cs->open(std::string(_uri["domain"]), _uri["port"]->ok() ? int(_uri["port"]) : (_is_ssl ? 443 : 80), _is_ssl);
			zpt::ZMQ* _socket = new zpt::ZMQHttp(_cs, this->__options);
			return _socket;
		}
		case ZMQ_MQTT_RAW : {
			zpt::socket_ref _found = this->get("__mqtt_connection__");
			if ((*_found) != nullptr) {
				return *_found;
			}
			break;
		}
		case ZMQ_UPNP_RAW : {
			zpt::socket_ref _found = this->get("__upnp_connection__");
			if ((*_found) != nullptr) {
				return *_found;
			}
			break;
		}
	}
	return nullptr;
}

auto zpt::ZMQPoll::clean_up(zpt::socket_ref _socket, bool _force) -> void {
	switch(_socket->type()) {
		case ZMQ_ROUTER_DEALER : {
			break;
		}
		case ZMQ_ROUTER : {
			break;
		}
		case ZMQ_DEALER : {
			break;
		}
		case ZMQ_REQ : {
			this->__to_remove.insert(std::make_pair(_socket, "zpt::ZMQPoll::clean_up / ZMQ_REQ"));
			this->__needs_rebuild = true;
			break;
		}
		case ZMQ_REP : {
			break;
		}
		case ZMQ_XPUB_XSUB : {
			break;
		}
		case ZMQ_PUB_SUB : {
			break;
		}
		case ZMQ_PUB : {
			break;
		}
		case ZMQ_SUB : {
			break;
		}
		case ZMQ_PUSH : {
			break;
		}
		case ZMQ_PULL : {
			break;
		}
		case ZMQ_HTTP_RAW : {
			if (_force || ((zpt::ZMQHttp*) *_socket)->underlying()->host() != "" || !_socket->available()) {
				this->__to_remove.insert(std::make_pair(_socket, "ztp::ZMQPoll::clean_up / ZMQ_HTTP_RAW"));
				this->__needs_rebuild = true;
			}
			break;
		}
		case ZMQ_MQTT_RAW : {
			break;
		}
		case ZMQ_UPNP_RAW : {
			break;
		}
	}
}

