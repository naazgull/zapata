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

#include <zapata/zmq/ZMQPolling.h>
#include <future>

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options, zpt::ev::emitter _emiter) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options, _emiter)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::ZMQPoll * _ptr) : std::shared_ptr<zpt::ZMQPoll>(_ptr) {
}

zpt::ZMQPollPtr::~ZMQPollPtr() {
}

zpt::ZMQPoll::ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter) : __options( _options), __id(0), __self(this), __emitter(_emiter)/*, __context(0)*/ {
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

auto zpt::ZMQPoll::add(short _type, std::string _connection) -> zpt::socket_ref {
	std::string _key(_connection.data());
	zpt::replace(_key, "*", ((string) this->__options["host"]));
	
	if (this->relay(_key) != nullptr) {
		return zpt::socket_ref(_key, this->self());
	}

	zpt::ZMQ* _underlying = this->bind(_type, _connection);
	if (_underlying == nullptr) {
		return zpt::socket_ref();
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
		this->signal(_socket.data());
	}
	else {
		this->__by_socket.push_back(_socket);
	}
}

auto zpt::ZMQPoll::repoll() -> void {
	if (this->__items.size() == this->__by_socket.size() + 1) {
		return;
	}
	// zdbg("rebuilding");
	this->__items.clear();
	
	for (size_t _k = 0; _k != this->__by_socket.size(); _k++) {
		if (this->__by_socket[_k]->in().get() != nullptr) {
			this->__items.push_back({ (void*)(*this->__by_socket[_k]->in()), 0, ZMQ_POLLIN, 0 });
		}
		else {
			this->__items.push_back({ 0, this->__by_socket[_k]->fd(), ZMQ_POLLIN, 0 });
		}
	}
	this->__items.push_back({ (void*)(*this->__sync[0]), 0, ZMQ_POLLIN, 0 });
}

auto zpt::ZMQPoll::reply(zpt::json _envelope, zpt::socket_ref _socket) -> void {
	try {
		zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);
		zpt::json _result = this->__emitter->trigger(_performative, _envelope["resource"]->str(), _envelope);
		if (_result->ok()) {
			_socket->send(_result +
				zpt::json{ 
					"channel", _envelope["channel"],
					"performative", zpt::ev::Reply,
					"resource", _envelope["resource"]
				}
			);
		}
	}
	catch(zpt::assertion& _e) {
		_socket->send(
			{
				"channel", _envelope["channel"],
				"performative", zpt::ev::Reply,
				"status", _e.status(),
				"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope),
				"payload", {
					"text", _e.what(),
					"assertion_failed", _e.description(),
					"code", _e.code()
				}
			}
		);
	}
	catch(std::exception& _e) {
		_socket->send(
			{
				"channel", _envelope["channel"],
				"performative", zpt::ev::Reply,
				"status", 500,
				"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope),
				"payload", {
					"text", _e.what(),
					"code", 0
				}
			}
		);
	}
}

auto zpt::ZMQPoll::loop() -> void {
	try {
		this->__id = pthread_self();
		std::vector< zpt::socket_ref > _to_add;
		std::vector< zpt::socket_ref > _to_remove;

		for(; true; ) {
			this->repoll();
			
			//zlog(std::string("polling for ") + std::to_string(this->__items.size()) + std::string(" sockets"), zpt::notice);
			zmq::poll(this->__items, -1);
			// zdbg("got something");

			for (size_t _k = 0; _k != this->__items.size() - 1; _k++) {
				if (this->__items[_k].revents & ZMQ_POLLIN) {
					//zdbg("its a read on a socket FD");
					try {
						zpt::socket_ref _socket = this->__by_socket[_k];
						zpt::json _envelope = _socket->recv();

						if (bool(_envelope["error"])) {
							_socket->send(_envelope +
								zpt::json{
									"channel", _envelope["channel"],
									"performative", zpt::ev::Reply,
									"resource", "/bad-request"
								}
							);
						}
						if (_envelope->ok()) {
							if (_socket->type() == ZMQ_REP) {
								this->reply(_envelope, _socket);
							}
							else {
								std::thread _worker(
									[] (zpt::json _envelope, zpt::socket_ref _socket, zpt::poll _poll) {
										_poll->reply(_envelope, _socket);
									},
									_envelope, _socket, this->self()
								);
								_worker.detach();
							}
						}
					}
					catch (zpt::assertion& _e) {
						zlog(std::string("error consuming socket: ") + _e.what() + std::string(", ") + _e.description(), zpt::error);
						_to_remove.push_back(this->__by_socket[_k]);
					}
					this->__items[_k].revents = 0;
				}
			}

			if (this->__items[this->__items.size() - 1].revents & ZMQ_POLLIN) {
				zmq::message_t _frame;
				this->__sync[0]->recv(&_frame);
				
				std::string _uuid(std::string(static_cast<char*>(_frame.data()), _frame.size()));
				//zdbg(std::string("its the add FD ") + _uuid);
				try {
					zpt::socket_ref _socket = this->get(_uuid);
					_to_add.push_back(_socket);
				}
				catch(zpt::assertion& _e) {
				}
				this->notify(_uuid);
				this->__items[this->__items.size() - 1].revents = 0;
			}

			for (auto _socket : _to_remove) {
				//zdbg(std::string("removing socket ") + _socket);
				for (size_t _k = 0; _k != this->__by_socket.size(); _k++) {
					if (this->__by_socket[_k] == _socket) {
						// zdbg("FOUND IT");
						this->__by_socket.erase(this->__by_socket.begin() + _k);
						break;
					}
				}
				_socket->close();
				this->remove(_socket);
			}
			_to_remove.clear();

			if (this->__by_socket.size() < 200) {
				for (; _to_add.size() != 0; ) {
					//zdbg(std::string("adding new socket ") + _to_add[0]);
					this->__by_socket.push_back(_to_add[0]);
					_to_add.erase(_to_add.begin());
					if (this->__by_socket.size() > 200) {
						break;
					}
				}
			}
		}
	}
	catch(zpt::assertion& _e) {
		zlog(_e.what() + std::string(": ") + _e.description(), zpt::emergency);
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
			zpt::ZMQ* _socket = new zpt::ZMQPull(_connection, this->__options);
			return _socket;
		}
	}
	return nullptr;
}

