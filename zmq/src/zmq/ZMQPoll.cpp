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

namespace zpt {
	namespace threads {
		bool interrupted = false;
	}
}


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

zpt::ZMQPoll::ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter) : __options( _options), __id(0), __self(this), __emitter(_emiter), __poll_set(nullptr), __need_rebuild(false) {
	for (short _i = 0; _i < 4; _i += 2) {
		std::string _uuid = zpt::generate::r_uuid();
		std::string _connection1(std::string("@inproc://") + _uuid);
		std::string _connection2(std::string(">inproc://") + _uuid);
		this->__sync[_i] = zpt::socket(new ZMQRep(_connection1, _options));
		this->__sync[_i + 1] = zpt::socket(new ZMQReq(_connection2, _options));
	}

	this->poll(this->__sync[0]);
	this->poll(this->__sync[2]);
}

zpt::ZMQPoll::~ZMQPoll() {
	for (short _i = 0; _i != 4; _i++) {
		this->__sync[_i]->unlisten();
		this->__sync[_i]->unbind();
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

auto zpt::ZMQPoll::signal(std::string _message, int _idx) -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx[1]);
	zframe_t* _frame = zframe_new(_message.data(), _message.length());
	bool _message_sent = (zsock_send(this->__sync[_idx]->out(), "f", _frame) == 0);
	zframe_destroy(&_frame);
	assertz(_message_sent, std::string("unable to send message to >inproc://notify"), 500, 0);
	this->wait(_idx);
}

auto zpt::ZMQPoll::notify(std::string _message, int _idx) -> void {
	zframe_t* _frame = zframe_new(_message.data(), _message.length());
	bool _message_sent = (zsock_send(this->__sync[_idx]->out(), "f", _frame) == 0);
	zframe_destroy(&_frame);
	assertz(_message_sent, std::string("unable to send message to >inproc://notify"), 500, 0);
}

auto zpt::ZMQPoll::wait(int _idx) -> void {
	zframe_t* _frame = nullptr;
	zsock_recv(this->__sync[_idx]->in(), "f", &_frame);
	zframe_destroy(&_frame);
}

auto zpt::ZMQPoll::poll(zpt::socket _socket, bool _signal) -> void {
	if (this->__id && _signal) {
		this->add_by_uuid(_socket);
		this->signal(_socket->id(), 1);
	}
	else {
		if (this->__id == 0) {
			this->add_by_uuid(_socket);
		}
		//zdbg("adding socket");
		this->__poll_sockets.push_back(_socket);
		this->__need_rebuild = true;
	}
}

auto zpt::ZMQPoll::unpoll(zpt::socket _socket, bool _signal) -> void {
	if (_signal) {
		this->signal(_socket->id(), 3);
	}
	else {
		//zdbg("removing socket");
		this->remove_by_uuid(_socket);
		if (_socket->poll_index() >= 0) {
			this->__poll_sockets.erase(this->__poll_sockets.begin() + _socket->poll_index());
			this->__need_rebuild = true;
		}
	}
}

auto zpt::ZMQPoll::repoll() -> void {
	if (!this->__need_rebuild) {
		return;
	}
	this->__poll_set_size = this->__poll_sockets.size();
	if (this->__poll_set != nullptr) {
		delete this->__poll_set;
		this->__poll_set = nullptr;
	}
	this->__poll_set = (zmq_pollitem_t*) malloc(this->__poll_set_size * sizeof(zmq_pollitem_t));
	size_t _k = 0;
	for (auto _socket : this->__poll_sockets) {
		this->__poll_set[_k].socket = zsock_resolve(_socket->in());
		this->__poll_set[_k].events = ZMQ_POLLIN;
		_socket->poll_index(_k);
		_k++;
	}
	this->__need_rebuild = false;	
}

auto zpt::ZMQPoll::loop() -> void {
	try {
		this->__id = pthread_self();
		std::vector< std::string > _to_add;
		std::vector< std::string > _to_remove;

		for(; true; ) {
			if (this->__need_rebuild) {
				//zdbg(std::string("rebuilding poll"));
				this->repoll();
			}
			int _rc = zmq_poll(this->__poll_set, this->__poll_set_size, -1);
			if (zpt::threads::interrupted) {
				return;
			}
			if (_rc > 0) {
				for (size_t _k = 0; _k != this->__poll_set_size; _k++) {
					if (this->__poll_set[_k].revents & ZMQ_POLLIN) {
						zpt::socket _socket = this->__poll_sockets[_k];

						if (_socket->id() == this->__sync[0]->id()) {
							zframe_t* _frame;
							zsock_recv(this->__sync[0]->in(), "f", &_frame);
							if (_frame != nullptr) {
								char* _bytes = zframe_strdup(_frame);
								std::string _uuid(std::string(_bytes, zframe_size(_frame)));
								std::free(_bytes);
								zframe_destroy(&_frame);
								_to_add.push_back(_uuid);
								this->notify(_uuid, 0);
							}
						}
						else if (_socket->id() == this->__sync[2]->id()) {
							zframe_t* _frame;
							zsock_recv(this->__sync[2]->in(), "f", &_frame);
							if (_frame != nullptr) {
								char* _bytes = zframe_strdup(_frame);
								std::string _uuid(std::string(_bytes, zframe_size(_frame)));
								std::free(_bytes);
								zframe_destroy(&_frame);
								_to_remove.push_back(_uuid);
								this->notify(_uuid, 2);
							}
						}
						else {
							bool _sent = false;
							std::string _cid = zpt::generate::r_uuid();
							std::string _resource("/bad-request");
							try {
								zpt::json _envelope = _socket->recv();

								if (bool(_envelope["error"]) && _socket->type() == ZMQ_REP) {
									_envelope << 
									"channel" << _resource <<
									"performative" << zpt::ev::Reply <<
									"resource" << _resource <<
									"status" << 400;
									_socket->send(_envelope);
									_sent = true;
								}
								else if (_envelope->ok()) {
									_cid.assign(std::string(_envelope["headers"]["X-Cid"]));
									_resource.assign(std::string(_envelope["resource"]));
					
									zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);
									zpt::json _result = this->__emitter->trigger(_performative, _envelope["resource"]->str(), _envelope);
									if (_result->ok() && (_socket->type() == ZMQ_REP || _socket->type() == ZMQ_PUB_SUB)) {
										try {
											_result << 
											"channel" << _envelope["headers"]["X-Cid"] <<
											"performative" << zpt::ev::Reply <<
											"resource" << _envelope["resource"];
											_socket->send(_result);
											_sent = true;
										}
										catch(zpt::assertion& _e) {}
									}
									else if (!_sent && _socket->type() == ZMQ_REP) {
										zpt::json _result{ 
											"channel", _cid,
											"performative", zpt::ev::Reply,
											"resource", _resource,
											"status", 204
											};
										_socket->send(_result);
										_sent = true;
									}
								}
							}
							catch (zpt::assertion& _e) {
								zlog(std::string("something wrong, no such zsock_t* for polled 0mq socket"), zpt::error);
								if (!_sent && _socket->type() == ZMQ_REP) {
									zpt::json _result{ 
										"channel", _cid,
										"performative", zpt::ev::Reply,
										"resource", _resource,
										"status", _e.status(),
										"payload", { "text", _e.what(), "assertion_failed", _e.description() }
									};
									_socket->send(_result);
									_sent = true;
								}
							}
							if (!_sent && _socket->type() == ZMQ_REP) {
								zpt::json _result{ 
									"channel", "/bad-request",
									"performative", zpt::ev::Reply,
									"resource", "/bad-request",
									"status", 400
									};
								_socket->send(_result);
							}
							if (_socket->once()) {
								_to_remove.push_back(_socket->id());
							}
						}
					}
				}
			}
			else {
				return;
			}
			
			for (auto _uuid : _to_add) {
				zpt::socket _new_socket = this->get_by_uuid(_uuid);
				this->poll(_new_socket, false);
			}
			_to_add.clear();

			for (auto _uuid : _to_remove) {
				zpt::socket _old_socket = this->get_by_uuid(_uuid);
				_old_socket->unlisten();
				_old_socket->unbind();
				this->unpoll(_old_socket, false);
			}
			_to_remove.clear();
			
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

