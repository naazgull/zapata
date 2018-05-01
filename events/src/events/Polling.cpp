/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#include <zapata/events/Polling.h>
#include <systemd/sd-daemon.h>
#include <future>
#include <chrono>

zpt::ChannelPoll::ChannelPoll(zpt::json _options, zpt::ev::emitter_factory _emiter)
    : zpt::Poll(), __options(_options), __id(0), __self(this), __emitter(_emiter),
      __needs_rebuild(true) /*, __context(0)*/
{
	std::string _uuid = zpt::generate::r_uuid();
	std::string _connection(std::string("inproc://") + _uuid);
	this->__sync[0] = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_REP));
	this->__sync[0]->bind(_connection);
	this->__sync[1] = zmq::socket_ptr(new zmq::socket_t(zpt::__context, ZMQ_REQ));
	this->__sync[1]->connect(_connection);
}

zpt::ChannelPoll::~ChannelPoll() {
	for (short _i = 0; _i != 4; _i++) {
		this->__sync[_i]->close();
	}
	zlog(string("zmq poll clean up"), zpt::notice);
}

auto zpt::ChannelPoll::options() -> zpt::json { return this->__options; }

auto zpt::ChannelPoll::emitter() -> zpt::ev::emitter_factory { return this->__emitter; }

auto zpt::ChannelPoll::self() const -> zpt::poll { return this->__self; }

auto zpt::ChannelPoll::get(std::string _key) -> zpt::socket_ref {
	return zpt::socket_ref(zpt::r_replace(_key, "*", ((string)this->__options["host"])), this->self());
}

auto zpt::ChannelPoll::relay(std::string _key) -> zpt::Channel* {
	{
		std::lock_guard<std::mutex> _lock(this->__mtx[0]);
		auto _found = this->__by_refs.find(_key);
		if (_found != this->__by_refs.end()) {
			return _found->second;
		}
	}
	return nullptr;
}

auto zpt::ChannelPoll::add(std::string _type, std::string _connection, bool _new_connection)
    -> zpt::socket_ref {
	std::string _key;

	if (!_new_connection) {
		_key = std::string(_connection.data());
		if (_key.front() == '@') {
			_key[0] = '>';
		} else if (_key.front() != '>') {
			_key = '>' + _key;
		}
		if (this->relay(_key) != nullptr) {
			// zdbg(std::string("already connected socket ") + _key);
			return zpt::socket_ref(_key, this->self());
		}
	}

	std::vector<zpt::socket> _underlying = this->bind(_type, _connection);
	assertz(_underlying.size() != 0,
		std::string("could not connection to ") + _type + std::string("@") + _connection,
		500,
		0);

	for (auto _u : _underlying) {
		if (_new_connection) {
			_key.assign(_u->id());
		}
		{
			std::lock_guard<std::mutex> _lock(this->__mtx[0]);
			this->__by_refs.insert(std::make_pair(_key, _u.get()));
		}
	}
	return zpt::socket_ref(_key, this->self());
}

auto zpt::ChannelPoll::add(zpt::Channel* _underlying) -> zpt::socket_ref {
	std::string _key = _underlying->id();

	if (this->relay(_key) != nullptr) {
		return zpt::socket_ref(_key, this->self());
	}

	{
		std::lock_guard<std::mutex> _lock(this->__mtx[0]);
		this->__by_refs.insert(std::make_pair(_key, _underlying));
	}
	return zpt::socket_ref(_key, this->self());
}

auto zpt::ChannelPoll::vanished(std::string _connection, zpt::ev::initializer _callback) -> void {
	std::string _key(_connection.data());
	if (_key.front() == '@') {
		_key[0] = '>';
	} else if (_key.front() != '>') {
		_key = '>' + _key;
	}

	if (this->relay(_key) != nullptr) {
		zpt::socket_ref _socket(_key, this->self());
		std::lock_guard<std::mutex> _lock(this->__mtx[0]);
		this->__to_remove.insert(std::make_pair(_socket, _callback));
		this->__needs_rebuild = true;
	}
}

auto zpt::ChannelPoll::vanished(zpt::Channel* _underlying, zpt::ev::initializer _callback) -> void {
	std::string _key = _underlying->id();

	if (this->relay(_key) != nullptr) {
		zpt::socket_ref _socket(_key, this->self());
		std::lock_guard<std::mutex> _lock(this->__mtx[0]);
		this->__to_remove.insert(std::make_pair(_socket, _callback));
		this->__needs_rebuild = true;
	}
}

auto zpt::ChannelPoll::remove(zpt::socket_ref _socket) -> void {
	std::string _key(_socket.data());

	{
		std::lock_guard<std::mutex> _lock(this->__mtx[0]);
		auto _found = this->__by_refs.find(_key);
		if (_found != this->__by_refs.end()) {
			_found->second->close();
			delete _found->second;
			this->__by_refs.erase(_found);
		}
	}
}

auto zpt::ChannelPoll::signal(std::string _key) -> void {
	std::lock_guard<std::mutex> _lock(this->__mtx[1]);
	zmq::message_t _frame(_key.length());
	memcpy(_frame.data(), _key.data(), _key.length());
	this->__sync[1]->send(_frame);
	this->wait();
}

auto zpt::ChannelPoll::notify(std::string _key) -> void {
	zmq::message_t _frame(_key.length());
	memcpy(_frame.data(), _key.data(), _key.length());
	this->__sync[0]->send(_frame);
}

auto zpt::ChannelPoll::wait() -> void {
	zmq::message_t _frame;
	this->__sync[1]->recv(&_frame);
}

auto zpt::ChannelPoll::poll(zpt::socket_ref _socket) -> void {
	if (this->__id) {
		if (this->__id != pthread_self()) {
			this->signal(_socket.data());
		} else {
			for (auto _already : this->__by_socket) {
				if (_already == _socket) {
					return;
				}
			}
			this->__to_add.insert(std::make_pair(_socket, "zpt::ChannelPoll::poll"));
			this->__needs_rebuild = true;
		}
	} else {
		this->__by_socket.push_back(_socket);
		this->__needs_rebuild = true;
	}
}

auto zpt::ChannelPoll::repoll() -> void {
	if (!this->__needs_rebuild) {
		return;
	}
	this->__items.clear();
	this->__items.resize(0);

	for (size_t _k = 0; _k != this->__by_socket.size(); _k++) {
		if (this->__by_socket[_k]->in().get() != nullptr) {
			this->__items.push_back({(void*)(*this->__by_socket[_k]->in()), 0, ZMQ_POLLIN, 0});
		} else {
			this->__items.push_back({0, this->__by_socket[_k]->fd(), ZMQ_POLLIN, 0});
		}
	}
	this->__items.push_back({(void*)(*this->__sync[0]), 0, ZMQ_POLLIN, 0});
	this->__needs_rebuild = false;
}

auto zpt::ChannelPoll::reply(zpt::json _envelope, zpt::socket_ref _socket) -> void {
	zpt::ev::performative _performative = (zpt::ev::performative)((int)_envelope["performative"]);

	if (_performative == zpt::ev::Reply) {
		if (this->__emitter->has_pending(_envelope)) {
			try {
				this->__emitter->reply(_envelope, _envelope);
			} catch (zpt::assertion& _e) {
				zlog(std::string("uncaught assertion while processing response, unable "
						 "to proceed: ") +
					 _e.what() + std::string(": ") + _e.description() +
					 std::string("\n| received message was:") + zpt::ev::pretty(_envelope),
				     zpt::emergency);
				zlog(std::string("\n") + _e.backtrace(), zpt::trace);
			} catch (std::exception& _e) {
				zlog(std::string("uncaught exception while processing response, unable "
						 "to proceed: ") +
					 _e.what() + std::string("\n| received message was:") +
					 zpt::ev::pretty(_envelope),
				     zpt::emergency);
			}
		} else {
			this->__emitter->trigger(_performative, _envelope["resource"]->str(), _envelope);
		}
		this->clean_up(_socket);
	} else {
		try {
			this->__emitter->trigger(
			    _performative,
			    _envelope["resource"]->str(),
			    _envelope,
			    zpt::undefined,
			    [=](zpt::ev::performative _p_performative,
				std::string _p_topic,
				zpt::json _result,
				zpt::ev::emitter _p_emitter) mutable -> void {
				    if (_result->ok()) {
					    if (*_socket != nullptr) {
						    _result =
							zpt::json{
							    "headers",
							    zpt::ev::init_reply(
								std::string(_envelope["headers"]["X-Cid"]), _envelope) +
								_p_emitter
								    ->options()["$defaults"]["headers"]["response"]} +
							_result + zpt::json{"channel",
									    _envelope["channel"],
									    "performative",
									    zpt::ev::Reply,
									    "resource",
									    _envelope["resource"]};
						    _socket->send(_result);
						    this->clean_up(_socket,
								   _envelope["headers"]["Connection"] ==
								       zpt::json::string("close"));
					    }
				    }
			    });
		} catch (zpt::assertion& _e) {
			if (*_socket != nullptr) {
				_socket->send(
				    zpt::ev::assertion_error(
					std::string(_envelope["resource"]),
					_e,
					zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) +
					    this->options()["$defaults"]["headers"]["response"]) +
				    zpt::json{"channel", _envelope["channel"]});
				this->clean_up(_socket,
					       _envelope["headers"]["Connection"] == zpt::json::string("close"));
			}
		} catch (std::exception& _e) {
			if (*_socket != nullptr) {
				_socket->send(
				    zpt::ev::internal_server_error(
					std::string(_envelope["resource"]),
					_e,
					zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"]), _envelope) +
					    this->options()["$defaults"]["headers"]["response"]) +
				    zpt::json{"channel", _envelope["channel"]});
				this->clean_up(_socket,
					       _envelope["headers"]["Connection"] == zpt::json::string("close"));
			}
		}
	}
}

auto zpt::ChannelPoll::pretty() -> std::string {
	std::string _return("\nAvailable sockets:\n");
	for (auto _socket : this->__by_refs) {
		_return += std::string("\t├─ (") + _socket.first + std::string(", ") + _socket.second->protocol() +
			   std::string(", ") + _socket.second->connection() + std::string(")\n");
	}
	return _return;
}

auto zpt::ChannelPoll::loop() -> void {
	try {
		this->__id = pthread_self();

		uint64_t _sd_watchdog_usec = 100000;
		bool _sd_watchdog_enabled = sd_watchdog_enabled(0, &_sd_watchdog_usec) != 0;
		uint64_t _poll_timeout = std::min(uint64_t(50), _sd_watchdog_usec / 1000 / 2);
		zlog(std::string("watchdog flag is ") +
			 (_sd_watchdog_enabled
			      ? std::string("enabled") + std::string(" and timeout is set to ") +
				    std::to_string(_sd_watchdog_usec / 1000 / 1000) + std::string(" seconds")
			      : std::string("disabled")),
		     zpt::notice);

		for (; true;) {
			this->repoll();

			int _n_events = 0;
			_n_events = zmq::poll(&this->__items[0], this->__items.size(), _poll_timeout);
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

			for (size_t _k = 0; _k != this->__items.size() - 1; _k++) {
				if (this->__items[_k].revents & ZMQ_POLLIN) {
					// zdbg(std::string("communication event on ") + std::to_string(_k));
					zpt::socket_ref _socket = this->__by_socket[_k];
					zverbose(std::string("poll event on ") + _socket->protocol() +
						 std::string("/") + _socket->connection());
					if (!_socket->available()) {
						// zdbg(std::string("could not consume data from socket: ") +
						// _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					}

					zpt::json _envelope;
					try {
						_envelope = _socket->recv();
					} catch (zpt::assertion& _e) {
						// zdbg(std::string("could not consume data from socket: ") +
						// _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					} catch (zmq::error_t& _e) {
						// zdbg(std::string("could not consume data from socket: ") +
						// _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					} catch (std::exception& _e) {
						// zdbg(std::string("could not consume data from socket: ") +
						// _socket->protocol() + std::string(" ") + _socket->connection());
						this->clean_up(_socket);
						continue;
					}

					if (bool(_envelope["error"])) {
						if (*_socket != nullptr) {
							_socket->loop_iteration();
							this->clean_up(_socket, true);
						}
						continue;
					} else if (_envelope->ok()) {
						_socket->loop_iteration();
						this->reply(_envelope, _socket);
					}
				} else {
					this->__by_socket[_k]->loop_iteration();
				}
			}

			if (this->__items[this->__items.size() - 1].revents & ZMQ_POLLIN) {
				zmq::message_t _frame;
				this->__sync[0]->recv(&_frame);

				std::string _uuid(std::string(static_cast<char*>(_frame.data()), _frame.size()));
				try {
					zpt::socket_ref _socket = this->get(_uuid);
					this->__to_add.insert(std::make_pair(_socket, "zpt::ChannelPoll::loop"));
				} catch (zpt::assertion& _e) {
				}
				this->notify(_uuid);
			}

			for (auto _pair : this->__to_remove) {
				zpt::socket_ref _socket = _pair.first;
				for (size_t _k = 0; _k != this->__by_socket.size(); _k++) {
					if (this->__by_socket[_k] == _socket) {
						this->__by_socket.erase(this->__by_socket.begin() + _k);
						break;
					}
				}
				if (_pair.second == nullptr) {
					_socket->close();
					this->remove(_socket);
				} else {
					this->__emitter->for_each(_pair.second);
				}
				this->__needs_rebuild = true;
			}
			this->__to_remove.clear();

			for (auto _pair : this->__to_add) {
				zpt::socket_ref _socket = _pair.first;
				this->__by_socket.push_back(_socket);
				this->__needs_rebuild = true;
			}
			this->__to_add.clear();
		}
	} catch (zpt::assertion& _e) {
		zlog(_e.what() + std::string(": ") + _e.description(), zpt::emergency);
		zlog(std::string("\n") + _e.backtrace(), zpt::trace);
		exit(-1);
	}
}

auto zpt::ChannelPoll::bind(std::string _type, std::string _connection) -> std::vector<zpt::socket> {
	std::vector<zpt::socket> _return;
	for (auto _factory : this->__emitter->channel(_type)) {
		if (_factory->is_reusable(_type)) {
			zpt::socket_ref _found = this->get(_connection);
			if ((*_found) != nullptr) {
				_return.push_back(zpt::socket(*_found));
				continue;
			}
		}
		_return.push_back(_factory->produce({"connection", _connection, "type", _type}));
	}
	return _return;
}

auto zpt::ChannelPoll::clean_up(zpt::socket_ref _socket, bool _force) -> void {
	if (!_socket->is_reusable()) {
		this->__to_remove.insert(std::make_pair(_socket, nullptr));
		this->__needs_rebuild = true;
	}
}

auto zpt::http2internal(zpt::http::req _request) -> zpt::json {
	zpt::json _return = zpt::json::object();
	_return << "channel"
		<< (_request->header("X-Cid").length() != 0 ? _request->header("X-Cid") : zpt::generate::r_uuid())
		<< "performative" << _request->method() << "resource" << _request->url();

	zpt::json _payload;
	if (_request->body() != "") {
		if (_request->header("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos) {
			_payload = zpt::http::deserialize(_request->body());
		} else if (_request->header("Content-Type").find("application/json") != std::string::npos) {
			_payload = zpt::json(_request->body());
		} else {
			_payload = {"text", _request->body()};
		}
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

auto zpt::http2internal(zpt::http::rep _reply) -> zpt::json {
	zpt::json _return = zpt::json::object();
	_return << "status" << (int)_reply->status() << "channel"
		<< (_reply->header("X-Cid").length() != 0 ? _reply->header("X-Cid") : zpt::undefined) << "performative"
		<< zpt::ev::Reply << "resource" << _reply->header("X-Resource");

	std::string _body = _reply->body();
	zpt::json _payload;
	zpt::trim(_body);
	if (_body != "") {
		if (_reply->header("Content-Type").find("application/x-www-form-urlencoded") != std::string::npos) {
			_payload = zpt::http::deserialize(_reply->body());
		} else if (_reply->header("Content-Type").find("application/json") != std::string::npos) {
			_payload = zpt::json(_reply->body());
		} else {
			_payload = {"text", _reply->body()};
		}
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

auto zpt::internal2http_rep(zpt::json _out) -> zpt::http::rep {
	zpt::http::rep _return;
	_return->status((zpt::HTTPStatus)((int)_out["status"]));

	if (_out["headers"]->is_object()) {
		;
		for (auto _header : _out["headers"]->obj()) {
			_return->header(_header.first, ((std::string)_header.second));
		}
	}
	if (_out["channel"]->ok()) {
		_return->header("X-Cid", std::string(_out["channel"]));
	}
	_return->header("X-Resource", _out["resource"]);

	if (_return->status() != zpt::HTTP204 && _return->status() != zpt::HTTP304 &&
	    _return->status() >= zpt::HTTP200) {
		if (((!_out["payload"]->is_object() && !_out["payload"]->is_array()) ||
		     (_out["payload"]->is_object() && _out["payload"]->obj()->size() != 0) ||
		     (_out["payload"]->is_array() && _out["payload"]->arr()->size() != 0))) {
			if (_out["headers"]["Content-Type"]->ok() &&
			    std::string(_out["headers"]["Content-Type"]) != "application/json") {
				if (std::string(_out["headers"]["Content-Type"]) ==
					"application/x-www-form-urlencoded" ||
				    std::string(_out["headers"]["Content-Type"]) == "multipart/form-data") {
					std::string _body;
					if (_out["payload"]->is_object()) {
						for (auto _param : _out["payload"]->obj()) {
							if (_body.length() != 0) {
								_body += std::string("&");
							}
							_body += _param.first + std::string("=") +
								 zpt::url::r_encode(std::string(_param.second));
						}
					} else {
						_body = std::string("payload=") +
							zpt::url::r_encode(std::string(_out["payload"]));
					}
					zpt::trim(_body);
					_return->header("Content-Length", std::to_string(_body.length()));
					_return->body(_body);
				} else {
					std::string _body = std::string(_out["payload"]);
					zpt::trim(_body);
					_return->header("Content-Type", "text/plain");
					_return->header("Content-Length", std::to_string(_body.length()));
					_return->body(_body);
				}
			} else {
				std::string _body = std::string(_out["payload"]);
				zpt::trim(_body);
				if (_out["payload"]->is_object() || _out["payload"]->is_array()) {
					_return->header("Content-Type", "application/json");
				} else {
					_return->header("Content-Type", "text/plain");
				}
				_return->body(_body);
				_return->header("Content-Length", std::to_string(_body.length()));
			}
		} else {
			_return->header("Content-Length", "0");
		}
	}
	return _return;
}

auto zpt::internal2http_req(zpt::json _out, std::string _host) -> zpt::http::req {
	zpt::http::req _return;
	_return->method(zpt::ev::performative(int(_out["performative"])));
	_return->url(std::string(_out["resource"]));

	if (!_out["headers"]->is_object()) {
		_out << "headers" << zpt::json::object();
	}
	_out["headers"] << "Host" << _host;
	for (auto _header : _out["headers"]->obj()) {
		_return->header(_header.first, ((std::string)_header.second));
	}

	if (_out["channel"]->ok()) {
		_return->header("X-Cid", std::string(_out["channel"]));
	}

	if (_out["params"]->is_object()) {
		for (auto _param : _out["params"]->obj()) {
			_return->param(_param.first, ((std::string)_param.second));
		}
	}

	if (((!_out["payload"]->is_object() && !_out["payload"]->is_array()) ||
	     (_out["payload"]->is_object() && _out["payload"]->obj()->size() != 0) ||
	     (_out["payload"]->is_array() && _out["payload"]->arr()->size() != 0))) {
		if (_out["headers"]["Content-Type"]->ok() &&
		    std::string(_out["headers"]["Content-Type"]) != "application/json") {
			if (std::string(_out["headers"]["Content-Type"]) == "application/x-www-form-urlencoded" ||
			    std::string(_out["headers"]["Content-Type"]) == "multipart/form-data") {
				std::string _body;
				if (_out["payload"]->is_object()) {
					for (auto _param : _out["payload"]->obj()) {
						if (_body.length() != 0) {
							_body += std::string("&");
						}
						_body += _param.first + std::string("=") +
							 zpt::url::r_encode(std::string(_param.second));
					}
				} else {
					_body =
					    std::string("payload=") + zpt::url::r_encode(std::string(_out["payload"]));
				}
				zpt::trim(_body);
				_return->header("Content-Length", std::to_string(_body.length()));
				_return->body(_body);
			} else {
				std::string _body = std::string(_out["payload"]);
				zpt::trim(_body);
				_return->header("Content-Type", "text/plain");
				_return->header("Content-Length", std::to_string(_body.length()));
				_return->body(_body);
			}
		} else {
			std::string _body = std::string(_out["payload"]);
			zpt::trim(_body);
			if (_body.length() != 0) {
				if (_out["payload"]->is_object() || _out["payload"]->is_array()) {
					_return->header("Content-Type", "application/json");
				} else {
					_return->header("Content-Type", "text/plain");
				}
				_return->header("Content-Length", std::to_string(_body.length()));
				_return->body(_body);
			} else {
				_return->header("Content-Length", "0");
			}
		}
	}
	return _return;
}

zpt::json zpt::http::deserialize(std::string _body) {
	zpt::json _return = zpt::json::object();
	std::string _name;
	std::string _collected;
	for (const auto& _c : _body) {
		switch (_c) {
		case '=': {
			_name.assign(_collected.data());
			_collected.assign("");
			break;
		}
		case '&': {
			zpt::url::decode(_collected);
			_return << _name << _collected;
			_name.assign("");
			_collected.assign("");
			break;
		}
		default: { _collected.push_back(_c); }
		}
	}
	zpt::url::decode(_collected);
	_return << _name << _collected;
	return _return;
}
