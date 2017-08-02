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

	if (!_new_connection && this->relay(zpt::type2str(_type) + std::string(">") + _connection) != nullptr) {
		return zpt::socket_ref(zpt::type2str(_type) + std::string(">") + _connection, this->self());
	}

	zpt::ZMQ* _underlying = this->bind(_type, _connection);
	assertz(_underlying != nullptr, std::string("could not connection to ") + zpt::type2str(_type) + std::string("@") + _connection, 500, 0);
	
	if (_new_connection) {
		_key.assign(_underlying->id());
	}
	else {
		_key.assign(zpt::type2str(_type) + std::string(">") + _underlying->connection());
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
	try {
		zpt::ev::performative _performative = (zpt::ev::performative) ((int) _envelope["performative"]);

		if (_performative == zpt::ev::Reply) {
			this->__emitter->reply(_envelope, _envelope);
		}
		else {
			this->__emitter->trigger(_performative, _envelope["resource"]->str(), _envelope, zpt::undefined,
				[=] (zpt::ev::performative _p_performative, std::string _p_topic, zpt::json _result, zpt::ev::emitter _p_emitter) mutable -> void {
					if (_result->ok()) {
						if (*_socket != nullptr) {
							_result = zpt::json{ "headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + _p_emitter->options()["$defaults"]["headers"]["response"] } + _result + 
								zpt::json{ 
									"channel", _envelope["channel"],
									"performative", zpt::ev::Reply,
									"resource", _envelope["resource"]
								};
							_socket->send(_result);
						}
					}
				}
			);
		}
	}
	catch(zpt::assertion& _e) {
		if (*_socket != nullptr) {
			_socket->send(
				{
					"channel", _envelope["channel"],
					"performative", zpt::ev::Reply,
					"status", _e.status(),
					"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + this->options()["$defaults"]["headers"]["response"],
					"payload", {
						"text", _e.what(),
						"assertion_failed", _e.description(),
						"code", _e.code()
					}
				}
			);
		}
	}
	catch(std::exception& _e) {
		if (*_socket != nullptr) {
			_socket->send(
				{
					"channel", _envelope["channel"],
					"performative", zpt::ev::Reply,
					"status", 500,
					"headers", zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) + this->options()["$defaults"]["headers"]["response"],
					"payload", {
						"text", _e.what(),
						"code", 0
					}
				}
			);
		}
	}
			
}

auto zpt::ZMQPoll::loop() -> void {
	try {
		this->__id = pthread_self();
		std::vector< zpt::socket_ref > _to_add;
		std::vector< zpt::socket_ref > _to_remove;

		uint64_t _sd_watchdog_usec = 30000;
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
			// else {
			// 	_n_events = zmq::poll(&this->__items[0], this->__items.size(), -1);
			// }

			if (_n_events == 0) {
				for (size_t _k = 0; _k != this->__items.size() - 1; _k++) {
					zpt::socket_ref _socket = this->__by_socket[_k];
					_socket->loop_iteration();
				}
				continue;
			}
			//zdbg(std::string("events ") + std::to_string(_n_events));
			
			for (size_t _k = 0; _k != this->__items.size() - 1; _k++) {
				if (this->__items[_k].revents & ZMQ_POLLIN) {
					//zdbg(std::string("communication event on ") + std::to_string(_k));
					zpt::socket_ref _socket = this->__by_socket[_k];
					if (!_socket->available()) {
						zlog(std::string("could not consume data from socket: peer has closed the connection"), zpt::verbose);
						_to_remove.push_back(this->__by_socket[_k]);
						continue;
					}

					zpt::json _envelope;
					try {
						_envelope = _socket->recv();
					}
					catch (zpt::assertion& _e) {
						zlog(std::string("could not consume data from socket: peer has closed the connection"), zpt::verbose);
						_to_remove.push_back(this->__by_socket[_k]);
						continue;
					}
					catch (zmq::error_t& _e) {
						zlog(std::string("could not consume data from socket: peer has closed the connection"), zpt::verbose);
						_to_remove.push_back(this->__by_socket[_k]);
						continue;
					}
					catch (std::exception& _e) {
						zlog(std::string("could not consume data from socket: peer has closed the connection"), zpt::verbose);
						_to_remove.push_back(this->__by_socket[_k]);
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
						}
					}
					else if (_envelope->ok()) {
						this->reply(_envelope, _socket);
					}

					if (_envelope["headers"]["Connection"] == zpt::json::string("close")) {
						_to_remove.push_back(_socket);
					}
				}
				else {
					zpt::socket_ref _socket = this->__by_socket[_k];
					_socket->loop_iteration();
				}
			}

			if (this->__items[this->__items.size() - 1].revents & ZMQ_POLLIN) {
				//zdbg("synchronization event");
				zmq::message_t _frame;
				this->__sync[0]->recv(&_frame);
				
				std::string _uuid(std::string(static_cast<char*>(_frame.data()), _frame.size()));
				try {
					zpt::socket_ref _socket = this->get(_uuid);
					//zdbg(std::string("adding socket ") + _socket);
					_to_add.push_back(_socket);
				}
				catch(zpt::assertion& _e) {
				}
				this->notify(_uuid);
			}

			for (auto _socket : _to_remove) {
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
			_to_remove.clear();
			_to_remove.resize(0);

			for (; _to_add.size() != 0; ) {
				this->__by_socket.push_back(_to_add[0]);
				_to_add.erase(_to_add.begin());
				this->__needs_rebuild = true;
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

// #define ZMQ_POLL_BASED_ON_POLL
// #define ZMQ_POLL_BASED_ON_SELECT

// auto zmq::__poll(zmq_pollitem_t *items_, int nitems_, long timeout_) -> int {
// #if defined ZMQ_POLL_BASED_ON_POLL
// 	if (!items_) {
// 		return -1;
// 	}

// 	uint64_t now = 0;
// 	uint64_t end = 0;
// 	pollfd spollfds[ZMQ_POLLITEMS_DFLT];
// 	pollfd *pollfds = spollfds;

// 	if (nitems_ > ZMQ_POLLITEMS_DFLT) {
// 		pollfds = (pollfd*) malloc (nitems_ * sizeof (pollfd));
// 	}

// 	//  Build pollset for poll () system call.
// 	for (int i = 0; i != nitems_; i++) {

// 		//  If the poll item is a 0MQ socket, we poll on the file descriptor
// 		//  retrieved by the ZMQ_FD socket option.
// 		if (items_ [i].socket) {
// 			size_t zmq_fd_size = sizeof (int);
// 			if (zmq_getsockopt (items_ [i].socket, ZMQ_FD, &pollfds [i].fd,
// 					&zmq_fd_size) == -1) {
// 				if (pollfds != spollfds)
// 					free (pollfds);
// 				return -1;
// 			}
// 			pollfds [i].events = items_ [i].events ? POLLIN : 0;
// 		}
// 		//  Else, the poll item is a raw file descriptor. Just convert the
// 		//  events to normal POLLIN/POLLOUT for poll ().
// 		else {
// 			pollfds [i].fd = items_ [i].fd;
// 			            pollfds [i].events =
// 				    (items_ [i].events & ZMQ_POLLIN ? POLLIN : 0) |
// 				    (items_ [i].events & ZMQ_POLLOUT ? POLLOUT : 0);
// 		}
// 	}

// 	bool first_pass = true;
// 	int nevents = 0;

// 	while (true) {
// 		//  Compute the timeout for the subsequent poll.
// 		int timeout;
// 		if (first_pass)
// 			timeout = 0;
// 		else
// 		        if (timeout_ < 0)
// 				timeout = -1;
// 		        else
// 				timeout = end - now;

// 		//  Wait for events.
// 		while (true) {
// 			int rc = poll(pollfds, nitems_, timeout);
// 			if (rc == -1 && errno == EINTR) {
// 				if (pollfds != spollfds)
// 					free(pollfds);
// 				return -1;
// 			}
// 			break;
// 		}
// 		//  Check for the events.
// 		for (int i = 0; i != nitems_; i++) {

// 			items_ [i].revents = 0;

// 			//  The poll item is a 0MQ socket. Retrieve pending events
// 			//  using the ZMQ_EVENTS socket option.
// 			if (items_ [i].socket) {
// 				size_t zmq_events_size = sizeof (uint32_t);
// 				uint32_t zmq_events;
// 				if (zmq_getsockopt (items_ [i].socket, ZMQ_EVENTS, &zmq_events,
// 						&zmq_events_size) == -1) {
// 					if (pollfds != spollfds)
// 						free (pollfds);
// 					return -1;
// 				}
// 				if ((items_ [i].events & ZMQ_POLLOUT) &&
// 					(zmq_events & ZMQ_POLLOUT))
// 					items_ [i].revents |= ZMQ_POLLOUT;
// 				if ((items_ [i].events & ZMQ_POLLIN) &&
// 					(zmq_events & ZMQ_POLLIN))
// 					items_ [i].revents |= ZMQ_POLLIN;
// 			}
// 			//  Else, the poll item is a raw file descriptor, simply convert
// 			//  the events to zmq_pollitem_t-style format.
// 			else {
// 				if (pollfds [i].revents & POLLIN)
// 					items_ [i].revents |= ZMQ_POLLIN;
// 				if (pollfds [i].revents & POLLOUT)
// 					items_ [i].revents |= ZMQ_POLLOUT;
// 				if (pollfds [i].revents & ~(POLLIN | POLLOUT))
// 					items_ [i].revents |= ZMQ_POLLERR;
// 			}

// 			if (items_ [i].revents)
// 				nevents++;
// 		}

// 		//  If timout is zero, exit immediately whether there are events or not.
// 		if (timeout_ == 0)
// 			break;

// 		//  If there are events to return, we can exit immediately.
// 		if (nevents)
// 			break;

// 		//  At this point we are meant to wait for events but there are none.
// 		//  If timeout is infinite we can just loop until we get some events.
// 		if (timeout_ < 0) {
// 			if (first_pass)
// 				first_pass = false;
// 			continue;
// 		}

// 		//  The timeout is finite and there are no events. In the first pass
// 		//  we get a timestamp of when the polling have begun. (We assume that
// 		//  first pass have taken negligible time). We also compute the time
// 		//  when the polling should time out.
// 		if (first_pass) {
// 			now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
// 			end = now + timeout_;
// 			if (now == end)
// 				break;
// 			first_pass = false;
// 			continue;
// 		}

// 		//  Find out whether timeout have expired.
// 		now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
// 		if (now >= end)
// 			break;
// 	}

// 	if (pollfds != spollfds)
// 	        free (pollfds);
// 	return nevents;

// #elif defined ZMQ_POLL_BASED_ON_SELECT

// 	if (nitems_ < 0) {
// 		errno = EINVAL;
// 		return -1;
// 	}
// 	if (nitems_ == 0) {
// 		if (timeout_ == 0)
// 			return 0;
// 		return usleep (timeout_ * 1000);
// 	}
// 	uint64_t now = 0;
// 	uint64_t end = 0;

// 	fd_set pollset_in;
// 	FD_ZERO (&pollset_in);
// 	fd_set pollset_out;
// 	FD_ZERO (&pollset_out);
// 	fd_set pollset_err;
// 	FD_ZERO (&pollset_err);

// 	int maxfd = 0;

// 	//  Build the fd_sets for passing to select ().
// 	for (int i = 0; i != nitems_; i++) {

// 		//  If the poll item is a 0MQ socket we are interested in input on the
// 		//  notification file descriptor retrieved by the ZMQ_FD socket option.
// 		if (items_ [i].socket) {
// 			size_t zmq_fd_size = sizeof (int);
// 			int notify_fd;
// 			if (zmq_getsockopt (items_ [i].socket, ZMQ_FD, &notify_fd,
// 					&zmq_fd_size) == -1)
// 				return -1;
// 			if (items_ [i].events) {
// 				FD_SET (notify_fd, &pollset_in);
// 				if (maxfd < notify_fd)
// 					maxfd = notify_fd;
// 			}
// 		}
// 		//  Else, the poll item is a raw file descriptor. Convert the poll item
// 		//  events to the appropriate fd_sets.
// 		else {
// 			if (items_ [i].events & ZMQ_POLLIN)
// 				FD_SET (items_ [i].fd, &pollset_in);
// 			if (items_ [i].events & ZMQ_POLLOUT)
// 				FD_SET (items_ [i].fd, &pollset_out);
// 			if (items_ [i].events & ZMQ_POLLERR)
// 				FD_SET (items_ [i].fd, &pollset_err);
// 			if (maxfd < items_ [i].fd)
// 				maxfd = items_ [i].fd;
// 		}
// 	}

// 	bool first_pass = true;
// 	int nevents = 0;
// 	fd_set inset, outset, errset;

// 	while (true) {

// 		//  Compute the timeout for the subsequent poll.
// 		timeval timeout;
// 		timeval *ptimeout;
// 		if (first_pass) {
// 			timeout.tv_sec = 0;
// 			timeout.tv_usec = 0;
// 			ptimeout = &timeout;
// 		}
// 		else
// 		        if (timeout_ < 0)
// 				ptimeout = NULL;
// 		        else {
// 				timeout.tv_sec = (long) ((end - now) / 1000);
// 				timeout.tv_usec = (long) ((end - now) % 1000 * 1000);
// 				ptimeout = &timeout;
// 			}

// 		//  Wait for events. Ignore interrupts if there's infinite timeout.
// 		while (true) {
// 			memcpy (&inset, &pollset_in, sizeof (fd_set));
// 			memcpy (&outset, &pollset_out, sizeof (fd_set));
// 			memcpy (&errset, &pollset_err, sizeof (fd_set));
// 			int rc = select (maxfd + 1, &inset, &outset, &errset, ptimeout);
// 			if (rc == -1) {
// 				return -1;
// 			}
// 			break;
// 		}

// 		//  Check for the events.
// 		for (int i = 0; i != nitems_; i++) {

// 			items_ [i].revents = 0;

// 			//  The poll item is a 0MQ socket. Retrieve pending events
// 			//  using the ZMQ_EVENTS socket option.
// 			if (items_ [i].socket) {
// 				size_t zmq_events_size = sizeof (uint32_t);
// 				uint32_t zmq_events;
// 				if (zmq_getsockopt (items_ [i].socket, ZMQ_EVENTS, &zmq_events,
// 						&zmq_events_size) == -1)
// 					return -1;
// 				if ((items_ [i].events & ZMQ_POLLOUT) &&
// 					(zmq_events & ZMQ_POLLOUT))
// 					items_ [i].revents |= ZMQ_POLLOUT;
// 				if ((items_ [i].events & ZMQ_POLLIN) &&
// 					(zmq_events & ZMQ_POLLIN))
// 					items_ [i].revents |= ZMQ_POLLIN;
// 			}
// 			//  Else, the poll item is a raw file descriptor, simply convert
// 			//  the events to zmq_pollitem_t-style format.
// 			else {
// 				if (FD_ISSET (items_ [i].fd, &inset))
// 					items_ [i].revents |= ZMQ_POLLIN;
// 				if (FD_ISSET (items_ [i].fd, &outset))
// 					items_ [i].revents |= ZMQ_POLLOUT;
// 				if (FD_ISSET (items_ [i].fd, &errset))
// 					items_ [i].revents |= ZMQ_POLLERR;
// 			}

// 			if (items_ [i].revents)
// 				nevents++;
// 		}

// 		//  If timout is zero, exit immediately whether there are events or not.
// 		if (timeout_ == 0)
// 			break;

// 		//  If there are events to return, we can exit immediately.
// 		if (nevents)
// 			break;

// 		//  At this point we are meant to wait for events but there are none.
// 		//  If timeout is infinite we can just loop until we get some events.
// 		if (timeout_ < 0) {
// 			if (first_pass)
// 				first_pass = false;
// 			continue;
// 		}

// 		//  The timeout is finite and there are no events. In the first pass
// 		//  we get a timestamp of when the polling have begun. (We assume that
// 		//  first pass have taken negligible time). We also compute the time
// 		//  when the polling should time out.
// 		if (first_pass) {
// 			now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
// 			end = now + timeout_;
// 			if (now == end)
// 				break;
// 			first_pass = false;
// 			continue;
// 		}

// 		//  Find out whether timeout have expired.
// 		now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
// 		if (now >= end)
// 			break;
// 	}

// 	return nevents;

// #endif
// }
