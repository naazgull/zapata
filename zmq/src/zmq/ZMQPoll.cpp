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

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options, zpt::ev::emitter _emiter) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options, _emiter)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::json _options) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::ZMQPoll * _ptr) : std::shared_ptr<zpt::ZMQPoll>(_ptr) {
}

zpt::ZMQPollPtr::~ZMQPollPtr() {
}

zpt::ZMQPoll::ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter) : __options( _options), __id(0), __poll(nullptr), __interrupt(nullptr), __signal(nullptr), __emitter(_emiter), __self(this) {
	zsys_init();
	zsys_handler_set(nullptr);
	assertz(zsys_has_curve(), "no security layer for 0mq. Is libcurve (https://github.com/zeromq/libcurve) installed?", 500, 0);
	
	this->__interrupt = zsock_new(ZMQ_PAIR);
	bool _attached = false;
	size_t _try = 0;
	std::string _connection;
	do {
		try {
			_connection.assign(std::string("@inproc://notify-") + std::to_string(_try));
			assertz(zsock_attach(this->__interrupt, _connection.data(), true) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__interrupt)) + std::string(" socket to ") + _connection, 500, 0);
			_attached = true;
		}
		catch(zpt::AssertionException& _e) {
			_try++;
		}
	}
	while(!_attached);
	this->__signal = zsock_new(ZMQ_PAIR);
	assertz(zsock_attach(this->__signal, (std::string(">inproc://notify-") + std::to_string(_try)).data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__signal)) + std::string(" socket to ") + (std::string(">inproc://notify-") + std::to_string(_try)), 500, 0);

	this->__poll = zpoller_new(this->__interrupt, nullptr);
}

zpt::ZMQPoll::ZMQPoll(zpt::json _options) : __options( _options), __id(0), __poll(nullptr), __interrupt(nullptr), __signal(nullptr), __emitter(nullptr), __self(this) {
	zsys_init();
	zsys_handler_set(nullptr);
	assertz(zsys_has_curve(), "no security layer for 0mq. Is libcurve (https://github.com/zeromq/libcurve) installed?", 500, 0);
	
	this->__interrupt = zsock_new(ZMQ_PAIR);
	bool _attached = false;
	size_t _try = 0;
	std::string _connection;
	do {
		try {
			_connection.assign(std::string("@inproc://notify-") + std::to_string(_try));
			assertz(zsock_attach(this->__interrupt, _connection.data(), true) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__interrupt)) + std::string(" socket to ") + _connection, 500, 0);
			_attached = true;
		}
		catch(zpt::AssertionException& _e) {
			_try++;
		}
	}
	while(!_attached);
	this->__signal = zsock_new(ZMQ_PAIR);
	assertz(zsock_attach(this->__signal, (std::string(">inproc://notify-") + std::to_string(_try)).data(), false) == 0, std::string("could not attach ") + std::string(zsock_type_str(this->__signal)) + std::string(" socket to ") + (std::string(">inproc://notify-") + std::to_string(_try)), 500, 0);

	this->__poll = zpoller_new(this->__interrupt, nullptr);
}

zpt::ZMQPoll::~ZMQPoll() {
	zsock_destroy(&this->__interrupt);
	zsock_destroy(&this->__signal);
	zpoller_destroy(&this->__poll);	
	zlog(string("zmq poll clean up"), zpt::notice);
}

zpt::json zpt::ZMQPoll::options() {
	return this->__options;
}

zpt::ev::emitter zpt::ZMQPoll::emitter() {
	return this->__emitter;
}

zpt::ZMQPollPtr zpt::ZMQPoll::self() {
	return this->__self;
}

void zpt::ZMQPoll::unbind() {
	this->__self.reset();
}

void zpt::ZMQPoll::signal_poller() {
	zframe_t* _frame = zframe_new("SIGNAL", 6);
	bool _message_sent = (zsock_send(this->__signal, "f", _frame) == 0);
	zframe_destroy(&_frame);
	assertz(_message_sent, std::string("unable to send message to >inproc://notify"), 500, 0);
}

void zpt::ZMQPoll::poll(zpt::socket _socket) {
	std::unique_lock< std::mutex > _synchronize(this->__mtx);
	_synchronize.unlock();

	_synchronize.lock();
	zpoller_add(this->__poll, _socket->in());
	this->__by_socket.insert(make_pair(_socket->in(), _socket));
	this->signal_poller();
	_synchronize.unlock();
}

void zpt::ZMQPoll::unpoll(zpt::ZMQ& _socket) {
	std::unique_lock< std::mutex > _synchronize(this->__mtx);
	_synchronize.unlock();

	_synchronize.lock();
	auto _found = this->__by_socket.find(_socket.in());
	if (_found != this->__by_socket.end()) {
		this->__by_socket.erase(_found);
	}
	zpoller_remove(this->__poll, _socket.in());
	this->signal_poller();
	_synchronize.unlock();
}

void zpt::ZMQPoll::unpoll_no_mutex(zpt::socket _socket) {
	auto _found = this->__by_socket.find(_socket->in());
	if (_found != this->__by_socket.end()) {
		this->__by_socket.erase(_found);
	}
	zpoller_remove(this->__poll, _socket->in());
}

void zpt::ZMQPoll::loop() {
	std::unique_lock< std::mutex > _synchronize(this->__mtx);
	_synchronize.unlock();
	this->__id = pthread_self();

	for(; true; ) {
		zsock_t* _awaken = (zsock_t*) zpoller_wait(this->__poll, -1);

		if (_awaken == nullptr) {
			continue;
		}

		if (_awaken == this->__interrupt) {
			zframe_t* _frame;
			if (zsock_recv(this->__interrupt, "f", &_frame) == 0) {
				zframe_destroy(&_frame);
			}
			continue;
		}
		
		_synchronize.lock();
		auto _found = this->__by_socket.find(_awaken);
		if (_found != this->__by_socket.end()) {
			zpt::socket _socket = _found->second;
			zpt::json _envelope = _socket->recv();
			_synchronize.unlock();
			
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
					catch(zpt::AssertionException& _e) {}
				}
			}
			if (_socket->once()) {
				this->unpoll_no_mutex(_socket);
				_socket->unbind();
			}
		}
		else {
			_synchronize.unlock();
		}
	}
}

zpt::socket zpt::ZMQPoll::bind(short _type, std::string _connection) {
	std::unique_lock< std::mutex > _synchronize(this->__mtx);
	_synchronize.unlock();
	switch(_type) {
		case ZMQ_ROUTER_DEALER : {
			std::string _key(_connection.data());
			zpt::replace(_key, "*", ((string) this->__options["host"]));
			_synchronize.lock();
			auto _found = this->__by_name.find(_key);
			if (_found != this->__by_name.end()) {
				_synchronize.unlock();
				return _found->second;
			}
			else {
				_synchronize.unlock();
				zpt::ZMQRouterDealer * _socket = new zpt::ZMQRouterDealer(_connection, this->__options, this->__emitter);
				_socket->listen(this->__self);
				_synchronize.lock();
				this->__by_name.insert(make_pair(_key, _socket->self()));
				_synchronize.unlock();
				return _socket->self();
			}
		}
		case ZMQ_REQ : {
			zpt::ZMQReq * _socket = new zpt::ZMQReq(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_REP : {
			std::string _key(_connection.data());
			zpt::replace(_key, "*", ((string) this->__options["host"]));
			_synchronize.lock();
			auto _found = this->__by_name.find(_key);
			if (_found != this->__by_name.end()) {
				_synchronize.unlock();
				return _found->second;
			}
			else {
				_synchronize.unlock();
				zpt::ZMQRep * _socket = new zpt::ZMQRep(_connection, this->__options, this->__emitter);
				_socket->listen(this->__self);
				_synchronize.lock();
				this->__by_name.insert(make_pair(_key, _socket->self()));
				_synchronize.unlock();
				return _socket->self();
			}
		}
		case ZMQ_XPUB_XSUB : {
			zpt::ZMQXPubXSub * _socket = new zpt::ZMQXPubXSub(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_PUB_SUB : {
			std::string _key(_connection.data());
			zpt::replace(_key, "*", ((string) this->__options["host"]));
			_synchronize.lock();
			auto _found = this->__by_name.find(_key);
			if (_found != this->__by_name.end()) {
				_synchronize.unlock();
				return _found->second;
			}
			else {
				_synchronize.unlock();
				zpt::ZMQPubSub * _socket = new zpt::ZMQPubSub(_connection, this->__options, this->__emitter);
				_socket->listen(this->__self);
				_synchronize.lock();
				this->__by_name.insert(make_pair(_key, _socket->self()));
				_synchronize.unlock();
				return _socket->self();
			}
		}
		case ZMQ_PUB : {
			std::string _key(_connection.data());
			zpt::replace(_key, "*", ((string) this->__options["host"]));
			_synchronize.lock();
			auto _found = this->__by_name.find(_key);
			if (_found != this->__by_name.end()) {
				_synchronize.unlock();
				return _found->second;
			}
			else {
				_synchronize.unlock();
				zpt::ZMQPub * _socket = new zpt::ZMQPub(_connection, this->__options, this->__emitter);
				_socket->listen(this->__self);
				_synchronize.lock();
				this->__by_name.insert(make_pair(_key, _socket->self()));
				_synchronize.unlock();
				return _socket->self();
			}
		}
		case ZMQ_SUB : {
			std::string _key(_connection.data());
			zpt::replace(_key, "*", ((string) this->__options["host"]));
			_synchronize.lock();
			auto _found = this->__by_name.find(_key);
			if (_found != this->__by_name.end()) {
				_synchronize.unlock();
				return _found->second;
			}
			else {
				_synchronize.unlock();
				zpt::ZMQSub * _socket = new zpt::ZMQSub(_connection, this->__options, this->__emitter);
				_socket->listen(this->__self);
				_synchronize.lock();
				this->__by_name.insert(make_pair(_key, _socket->self()));
				_synchronize.unlock();
				return _socket->self();
			}
		}
		case ZMQ_PUSH : {
			std::string _key(_connection.data());
			zpt::replace(_key, "*", ((string) this->__options["host"]));
			_synchronize.lock();
			auto _found = this->__by_name.find(_key);
			if (_found != this->__by_name.end()) {
				_synchronize.unlock();
				return _found->second;
			}
			else {
				_synchronize.unlock();
				zpt::ZMQPush * _socket = new zpt::ZMQPush(_connection, this->__options, this->__emitter);
				_socket->listen(this->__self);
				_synchronize.lock();
				this->__by_name.insert(make_pair(_key, _socket->self()));
				_synchronize.unlock();
				return _socket->self();
			}
		}
		case ZMQ_PULL : {
			zpt::ZMQPull * _socket = new zpt::ZMQPull(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();
		}
	}
	return zpt::socket(nullptr);
}

