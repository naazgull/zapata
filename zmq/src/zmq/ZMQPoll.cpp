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

zpt::ZMQPollPtr::ZMQPollPtr(zpt::JSONPtr _options, zpt::EventEmitterPtr _emiter) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options, _emiter)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::JSONPtr _options) : std::shared_ptr<zpt::ZMQPoll>(new zpt::ZMQPoll(_options)) {
}

zpt::ZMQPollPtr::ZMQPollPtr(zpt::ZMQPoll * _ptr) : std::shared_ptr<zpt::ZMQPoll>(_ptr) {
}

zpt::ZMQPollPtr::~ZMQPollPtr() {
}

zpt::ZMQPoll::ZMQPoll(zpt::JSONPtr _options, zpt::EventEmitterPtr _emiter) : __options( _options), __context(1), __id(0), __poll(nullptr), __emitter(_emiter), __self(this) {
	this->__internal = new zmq::socket_t * [2];

	this->__internal[0] = new zmq::socket_t(this->__context, ZMQ_REP);
	this->__internal[1] = new zmq::socket_t(this->__context, ZMQ_REQ);

	std::string _bind("inproc://notify");
	this->__internal[0]->bind(_bind.data());
	this->__internal[1]->connect(_bind.data());

	this->__poll_size = this->__sockets.size() + 1;
	this->__poll = (zmq::pollitem_t *) realloc(this->__poll, (this->__sockets.size() + 1) * sizeof(zmq::pollitem_t));
	this->__poll[0] = { static_cast<void *>(* this->__internal[0]), 0, ZMQ_POLLIN, 0 };

	this->__pt_mtx = new pthread_mutex_t();
	this->__pt_attr = new pthread_mutexattr_t();
	pthread_mutexattr_init(this->__pt_attr);
	pthread_mutex_init(this->__pt_mtx, this->__pt_attr);	
}

zpt::ZMQPoll::ZMQPoll(zpt::JSONPtr _options) : __options( _options), __context(1), __id(0), __poll(nullptr), __emitter(nullptr), __self(this) {
	this->__internal = new zmq::socket_t * [2];

	this->__internal[0] = new zmq::socket_t(this->__context, ZMQ_REP);
	this->__internal[1] = new zmq::socket_t(this->__context, ZMQ_REQ);

	std::string _bind("inproc://notify");
	this->__internal[0]->bind(_bind.data());
	this->__internal[1]->connect(_bind.data());

	this->__poll_size = this->__sockets.size() + 1;
	this->__poll = (zmq::pollitem_t *) realloc(this->__poll, (this->__sockets.size() + 1) * sizeof(zmq::pollitem_t));
	this->__poll[0] = { static_cast<void *>(* this->__internal[0]), 0, ZMQ_POLLIN, 0 };

	this->__pt_mtx = new pthread_mutex_t();
	this->__pt_attr = new pthread_mutexattr_t();
	pthread_mutexattr_init(this->__pt_attr);
	pthread_mutex_init(this->__pt_mtx, this->__pt_attr);	
}

zpt::ZMQPoll::~ZMQPoll() {
	this->__internal[0]->close();
	this->__internal[1]->close();
	delete this->__internal[0];
	delete this->__internal[1];
 	delete [] this->__internal;
 	free(this->__poll);

	pthread_mutexattr_destroy(this->__pt_attr);
	pthread_mutex_destroy(this->__pt_mtx);
	delete this->__pt_mtx;
	delete this->__pt_attr;

	zlog(string("zmq poll clean up"), zpt::notice);
}

zpt::JSONPtr zpt::ZMQPoll::options() {
	return this->__options;
}

zpt::EventEmitterPtr zpt::ZMQPoll::emitter() {
	return this->__emitter;
}

zpt::ZMQPollPtr zpt::ZMQPoll::self() {
	return this->__self;
}

void zpt::ZMQPoll::lock() {
	pthread_mutex_lock(this->__pt_mtx);
}

void zpt::ZMQPoll::unlock() {
	pthread_mutex_unlock(this->__pt_mtx);	
}

void zpt::ZMQPoll::poll(zpt::ZMQPtr _socket) {
	this->__sockets.push_back(_socket);

	if (this->__id != 0 && this->__id != pthread_self()) {
		zmq::message_t _signal(6);
		memcpy ((void *) _signal.data(), "SIGNAL", 6);
		this->__internal[1]->send(_signal);
		zmq::message_t _reply;
		try {
			this->__internal[1]->recv(& _reply);
		}
		catch(zmq::error_t& e) {
			zlog("got a zmq::error_t, signaling polling", zpt::error);
		}
	}
	else {
		this->repoll();
	}
}

void zpt::ZMQPoll::repoll() {
	zlog(string("scanning poll"), zpt::notice);
	this->__poll = (zmq::pollitem_t *) realloc(this->__poll, (this->__sockets.size() + 1) * sizeof(zmq::pollitem_t));

	size_t _i = 1;
	for (auto _socket : this->__sockets) {
		if (_i < this->__poll_size) {
			_i++;
			continue;
		}
		this->__poll[_i] = { static_cast<void *>(_socket->in()), 0, ZMQ_POLLIN, 0 };

		_i++;
	}
	this->__poll_size = this->__sockets.size() + 1;
}

void zpt::ZMQPoll::loop() {
	this->__id = pthread_self();

	for(; true; ) {
		try {			
			zmq::poll(this->__poll, this->__poll_size, -1);
		}
		catch(zmq::error_t& _e) {
			zlog(string("while polling sockets: ") + _e.what(), zpt::error);
		}

		if (this->__poll[0].revents & ZMQ_POLLIN) {
			this->repoll();

			zmq::message_t _reply;
			try {
				this->__internal[0]->recv(& _reply);
			}
			catch(zmq::error_t& _e) {
			}
			zmq::message_t _signal(2);
			memcpy ((void *) _signal.data(), "ok", 2);
			this->__internal[0]->send(_signal);
		}
		else {
			for (size_t _k = 1; _k != this->__sockets.size() + 1; _k++) {
				if (this->__poll[_k].revents & ZMQ_POLLIN) {
					zpt::JSONPtr _envelope = this->__sockets[_k - 1]->recv();
					zpt::ev::Performative _performative = (zpt::ev::Performative) ((int) _envelope["performative"]);
					zpt::JSONPtr _result = this->__emitter->trigger(_performative, _envelope["resource"]->str(), _envelope);
					if (_result->ok()) {
						try {
							_result << 
								"channel" << _envelope["headers"]["X-Cid"] <<
								"performative" << zpt::ev::Reply <<
								"resource" << _envelope["resource"];
							this->__sockets[_k - 1]->send(_result);
						}
						catch(zpt::AssertionException& _e) {}
					}
				}
			}
		}
	}
}

zpt::ZMQPtr zpt::ZMQPoll::bind(short _type, std::string _connection) {
	switch(_type) {
		case ZMQ_REQ : {
			zpt::ZMQReq * _socket = new zpt::ZMQReq(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_REP : {
			zpt::ZMQRep * _socket = new zpt::ZMQRep(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_XPUB_XSUB : {
			zpt::ZMQXPubXSub * _socket = new zpt::ZMQXPubXSub(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();
		}
		case ZMQ_PUB_SUB : {
			zpt::ZMQPubSub * _socket = new zpt::ZMQPubSub(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();
		}			
		case ZMQ_PUB : {
			/*zpt::ZMQPub * _socket = new zpt::ZMQPub(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();*/
		}
		case ZMQ_SUB : {
			/*zpt::ZMQSub * _socket = new zpt::ZMQSub(_connection, this->__options, this->__emitter);
			_socket->listen(this->__self);
			return _socket->self();*/
		}
	}
	return zpt::ZMQPtr(nullptr);
}

extern "C" int zapata_zmq() {
	return 1;
}
