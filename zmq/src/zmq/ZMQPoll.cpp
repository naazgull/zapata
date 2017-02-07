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

zpt::assync::AssyncReplyException::AssyncReplyException(zpt::socket _relay) : std::exception(), __relay(_relay) {
}

zpt::assync::AssyncReplyException::~AssyncReplyException() {
}

std::string zpt::assync::AssyncReplyException::what() {
	return std::string("creating assync request/reply attached to ") + this->__relay->connection();
}

zpt::socket zpt::assync::AssyncReplyException::relay() {
	return this->__relay;
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options, zpt::ev::emitter _emiter) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options, _emiter)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::ZMQPoll * _ptr) : std::shared_ptr<zpt::ZMQPoll>(_ptr) {
}

zpt::ZMQPollPtr::~ZMQPollPtr() {
}

zpt::ZMQPoll::ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter) : __options( _options), __id(0), __poll(nullptr), __self(this), __emitter(_emiter) {
	zsys_init();
	zsys_handler_set(nullptr);
	assertz(zsys_has_curve(), "no security layer for 0mq. Is libcurve (https://github.com/zeromq/libcurve) installed?", 500, 0);
	
	for (short _i = 0; _i < 4; _i += 2) {
		std::string _uuid = zpt::generate::r_uuid();
		std::string _connection1(std::string("@inproc://") + _uuid);
		std::string _connection2(std::string(">inproc://") + _uuid);
		this->__sync[_i] = zsock_new(ZMQ_REP);
		assertz(zsock_attach(this->__sync[_i], _connection1.data(), true) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__sync[_i])) + std::string(" socket to ") + _connection1, 500, 0);
		this->__sync[_i + 1] = zsock_new(ZMQ_REQ);
		assertz(zsock_attach(this->__sync[_i + 1], _connection2.data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__sync[_i + 1])) + std::string(" socket to ") + _connection2, 500, 0);
	}

	this->__poll = zpoller_new(this->__sync[0], nullptr);
	zpoller_add(this->__poll, this->__sync[2]);
	zpoller_ignore_interrupts(this->__poll);
}

zpt::ZMQPoll::~ZMQPoll() {
	for (short _i = 0; _i != 4; _i++) {
		zsock_destroy(&this->__sync[_i]);
	}
	zpoller_destroy(&this->__poll);	
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

auto  zpt::ZMQPoll::unbind() -> void {
	this->__self.reset();
}

auto zpt::ZMQPoll::get_by_name(std::string _name) -> zpt::socket {
	std::string _key(_name.data());
	zpt::replace(_key, "*", ((string) this->__options["host"]));
	
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	auto _found = this->__by_name.find(_key);
	assertz(_found != this->__by_name.end(), "socket not found", 406, 0);
	return _found->second;
}

auto zpt::ZMQPoll::get_by_uuid(std::string _uuid) -> zpt::socket {
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	auto _found = this->__by_uuid.find(_uuid);
	assertz(_found != this->__by_uuid.end(), "socket not found", 406, 0);
	return _found->second;
}

auto zpt::ZMQPoll::get_by_zsock(zsock_t* _sock) -> zpt::socket {
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	auto _found = this->__by_socket.find(_sock);
	assertz(_found != this->__by_socket.end(), "socket not found", 406, 0);
	return _found->second;
}

auto zpt::ZMQPoll::add_by_name(zpt::socket _socket) -> void {
	std::string _key(_socket->connection().data());
	zpt::replace(_key, "*", ((string) this->__options["host"]));
	
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	this->__by_name.insert(std::make_pair(_key, _socket));
}

auto zpt::ZMQPoll::add_by_uuid(zpt::socket _socket) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	this->__by_uuid.insert(std::make_pair(_socket->id(), _socket));
}

auto zpt::ZMQPoll::add_by_zsock(zpt::socket _socket) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	this->__by_socket.insert(std::make_pair(_socket->in(), _socket));
}

auto zpt::ZMQPoll::remove_by_name(zpt::socket _socket) -> void {
	std::string _key(_socket->connection().data());
	zpt::replace(_key, "*", ((string) this->__options["host"]));
	
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	auto _found = this->__by_name.find(_key);
	if (_found != this->__by_name.end()) {
		this->__by_name.erase(_found);
	}
}

auto zpt::ZMQPoll::remove_by_uuid(zpt::socket _socket) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	auto _found = this->__by_uuid.find(_socket->id());
	if (_found != this->__by_uuid.end()) {
		this->__by_uuid.erase(_found);
	}
}

auto zpt::ZMQPoll::remove_by_zsock(zpt::socket _socket) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx[0]);
	auto _found = this->__by_socket.find(_socket->in());
	if (_found != this->__by_socket.end()) {
		this->__by_socket.erase(_found);
	}
}

auto zpt::ZMQPoll::signal(std::string _message, int _idx) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx[1]);
	zframe_t* _frame = zframe_new(_message.data(), _message.length());
	bool _message_sent = (zsock_send(this->__sync[_idx], "f", _frame) == 0);
	zframe_destroy(&_frame);
	assertz(_message_sent, std::string("unable to send message to >inproc://notify"), 500, 0);
	this->wait(_idx);
}

auto zpt::ZMQPoll::notify(std::string _message, int _idx) -> void {
	zframe_t* _frame = zframe_new(_message.data(), _message.length());
	bool _message_sent = (zsock_send(this->__sync[_idx], "f", _frame) == 0);
	zframe_destroy(&_frame);
	assertz(_message_sent, std::string("unable to send message to >inproc://notify"), 500, 0);
}

auto zpt::ZMQPoll::wait(int _idx) -> void {
	zframe_t* _frame = nullptr;
	zsock_recv(this->__sync[_idx], "f", &_frame);
	zframe_destroy(&_frame);
}

auto zpt::ZMQPoll::poll(zpt::socket _socket) -> void {
	this->add_by_zsock(_socket);
	this->add_by_uuid(_socket);
	if (this->__id) {
		this->signal(_socket->id(), 1);
	}
	else {
		zpoller_add(this->__poll, _socket->in());
	}
}

auto zpt::ZMQPoll::unpoll(zpt::socket _socket, bool _signal) -> void {
	this->remove_by_zsock(_socket);
	if (_signal) {
		this->signal(_socket->id(), 3);
	}
	else {
		zpoller_remove(this->__poll, _socket->in());
		this->remove_by_uuid(_socket);
	}
}

auto zpt::ZMQPoll::loop() -> void {
	try {
		this->__id = pthread_self();

		for(; true; ) {
			zsock_t* _awaken = (zsock_t*) zpoller_wait(this->__poll, -1);
			assertz(zpoller_expired(this->__poll) == false, "zpoller_expired is true", 500, 0);
			assertz(zpoller_terminated(this->__poll) == false, "zpoller_terminated is true", 500, 0);

			if (_awaken == nullptr) {
				zlog(std::string("something wrong, no file descriptor for polled 0mq socket"), zpt::error);
				exit(-1);
				continue;
			}

			if (_awaken == this->__sync[0]) {
				zframe_t* _frame;
				zsock_recv(this->__sync[0], "f", &_frame);
				if (_frame != nullptr) {
					char* _bytes = zframe_strdup(_frame);
					std::string _uuid(std::string(_bytes, zframe_size(_frame)));
					std::free(_bytes);
					zframe_destroy(&_frame);
					try {
						zpt::socket _socket = this->get_by_uuid(_uuid);
						zpoller_add(this->__poll, _socket->in());
					}
					catch(zpt::assertion& _e) {
						zlog(std::string("something wrong, no such UUID for polled 0mq socket"), zpt::error);
						exit(-1);
					}
					this->notify(_uuid, 0);
				
				}
				continue;
			}

			if (_awaken == this->__sync[2]) {
				zframe_t* _frame;
				zsock_recv(this->__sync[2], "f", &_frame);
				if (_frame != nullptr) {
					char* _bytes = zframe_strdup(_frame);
					std::string _uuid(std::string(_bytes, zframe_size(_frame)));
					std::free(_bytes);
					zframe_destroy(&_frame);
					try {
						zpt::socket _socket = this->get_by_uuid(_uuid);
						zpoller_remove(this->__poll, _socket->in());
						this->remove_by_uuid(_socket);
					}
					catch(zpt::assertion& _e) {
						zlog(std::string("something wrong, no such UUID for polled 0mq socket"), zpt::error);
						exit(-1);
					}
					this->notify(_uuid, 2);
				}
				continue;
			}

			try {
				zpt::socket _socket = this->get_by_zsock(_awaken);
				zpt::json _envelope = _socket->recv();

				if (_envelope->ok()) {
					zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);
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
				}
				if (_socket->once()) {
					this->unpoll(_socket, false);
					_socket->unlisten();
					_socket->unbind();
				}
			}
			catch (zpt::assertion& _e) {
				zlog(std::string("something wrong, no such zsock_t* for polled 0mq socket"), zpt::error);
				exit(-1);
			}
		}
	}
	catch(zpt::assertion& _e) {
		zlog(_e.what() + std::string(": ") + _e.description(), zpt::emergency);
		exit(-_e.code());
	}
}

auto zpt::ZMQPoll::bind(short _type, std::string _connection) -> zpt::socket {
	switch(_type) {
		case ZMQ_ROUTER_DEALER : {
			try {
				return this->get_by_name(_connection);
			}
			catch (zpt::assertion& _e) {}
			
			zpt::ZMQ* _socket = new zpt::ZMQRouterDealer(_connection, this->__options);
			_socket->listen(this->__self);
			this->add_by_name(_socket->self());
			return _socket->self();
		}
		case ZMQ_ASSYNC_REQ : {
			zpt::ZMQ* _socket = new zpt::ZMQAssyncReq(_connection, this->__options);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_REQ : {
			zpt::ZMQ* _socket = new zpt::ZMQReq(_connection, this->__options);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_REP : {
			try {
				return this->get_by_name(_connection);
			}
			catch (zpt::assertion& _e) {}
			
			zpt::ZMQ* _socket = new zpt::ZMQRep(_connection, this->__options);
			_socket->listen(this->__self);
			this->add_by_name(_socket->self());
			return _socket->self();
		}
		case ZMQ_XPUB_XSUB : {
			zpt::ZMQ* _socket = new zpt::ZMQXPubXSub(_connection, this->__options);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_PUB_SUB : {
			try {
				return this->get_by_name(_connection);
			}
			catch (zpt::assertion& _e) {}

			zpt::ZMQ* _socket = new zpt::ZMQPubSub(_connection, this->__options);
			_socket->listen(this->__self);
			this->add_by_name(_socket->self());
			return _socket->self();
		}
		case ZMQ_PUB : {
			try {
				return this->get_by_name(_connection);
			}
			catch (zpt::assertion& _e) {}

			zpt::ZMQ* _socket = new zpt::ZMQPub(_connection, this->__options);
			_socket->listen(this->__self);
			this->add_by_name(_socket->self());
			return _socket->self();
		}
		case ZMQ_SUB : {
			try {
				return this->get_by_name(_connection);
			}
			catch (zpt::assertion& _e) {}

			zpt::ZMQ* _socket = new zpt::ZMQSub(_connection, this->__options);
			_socket->listen(this->__self);
			this->add_by_name(_socket->self());
			return _socket->self();
		}
		case ZMQ_PUSH : {
			try {
				return this->get_by_name(_connection);
			}
			catch (zpt::assertion& _e) {}

			zpt::ZMQ* _socket = new zpt::ZMQPush(_connection, this->__options);
			_socket->listen(this->__self);
			this->add_by_name(_socket->self());
			return _socket->self();
		}
		case ZMQ_PULL : {
			zpt::ZMQ* _socket = new zpt::ZMQPull(_connection, this->__options);
			_socket->listen(this->__self);
			return _socket->self();
		}
	}
	return zpt::socket(nullptr);
}

