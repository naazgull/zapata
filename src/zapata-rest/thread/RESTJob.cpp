/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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

#include <zapata/thread/RESTJob.h>

#include <zapata/net.h>
#include <zapata/http.h>
#include <zapata/exceptions/SyntaxErrorException.h>

zapata::RESTJob::RESTJob(const RESTJob& _rhs) : Job(_rhs.get()) {
}

zapata::RESTJob::RESTJob(string _key_file_path) : Job() {
	ifstream _key_file;
	_key_file.open(_key_file_path.data());
	this->__configuration = zapata::fromfile(_key_file);

	sigset_t _mask;
	sigemptyset(&_mask);
	sigaddset(&_mask, SIGPOLL);
	pthread_sigmask(SIG_BLOCK, &_mask, NULL);
	struct pollfd _sfd;
	_sfd.fd = signalfd(-1, & _mask, 0);
	_sfd.events = POLLIN;
	this->__peers.push_back(_sfd);

	(* this)->loop([ this ] (Job& _job) -> void {
		bool _debug = !!this->configuration()["zapata"]["rest"]["debug"];
	
		size_t _size_of = sizeof(thr_signal_t);
		for (; true; ) {
			int _rv = poll(& this->__peers[0], this->__peers.size(), -1);
			if (_rv > 0) {
				if (this->__peers[0].revents & POLLIN) {
					zapata::log("sweet, got a new client!", zapata::debug);
					thr_signal_t _fdsi;
					if (read(this->__peers[0].fd, &_fdsi, _size_of) == _size_of) {
					}
					continue;
				}
				for (auto _fd : this->__peers) {
					if (_fd.revents & POLLIN) {
						socketstream _cs(_fd.fd);

						HTTPRep _rep;
						HTTPReq _req;
						try  {
							zapata::fromhttpstream(_cs, _req);
							this->__pool->trigger(_req, _rep);

							string _origin = _req->header("Origin");
							if (_origin.length() != 0) {
								_rep->header("Access-Control-Allow-Origin", _origin);
								_rep->header("Access-Control-Expose-Headers", REST_ACCESS_CONTROL_HEADERS);
							}

							if (zapata::log_lvl) {
								this->log(_req, _rep);
							}
							_rep->stringify(_cs);
							_cs << flush;
						}
						catch(zapata::SyntaxErrorException& e) {
							zapata::log(e.what(), zapata::error);

							zapata::JSONObj _body;
							_body
								<< "error" << true
								<< "assertion_failed" << e.what()
								<< "message" << e.what()
								<< "code" << 400;

							string _text;
							zapata::tostr(_text, _body);

							_rep->body(_text);
							_rep->status(zapata::HTTP400);
							_rep->header("Content-Type", "application/json");
							string _length;
							zapata::tostr(_length, _text.length());
							_rep->header("Content-Length", _length);

							string _origin = _req->header("Origin");
							if (_origin.length() != 0) {
								_rep->header("Access-Control-Allow-Origin", _origin);
								_rep->header("Access-Control-Expose-Headers", REST_ACCESS_CONTROL_HEADERS);
							}
						}
						catch(zapata::ClosedException& e) {
							zapata::log(e.what(), zapata::error);
						}
					}
				}
			}
		}
	});
}

zapata::RESTJob::~RESTJob() {
}

void zapata::RESTJob::assign(int _cs_fd) {
	pthread_mutex_lock((* this)->__mtx);

	struct pollfd _fd;
	_fd.fd = _cs_fd;
	_fd.events = POLLIN;
	this->__peers.push_back(_fd);

	pthread_mutex_unlock((* this)->__mtx);
}

zapata::RESTPool& zapata::RESTJob::pool() {
	return * this->__pool;
}

void zapata::RESTJob::pool(zapata::RESTPool * _pool) {
	this->__pool = _pool;
}

zapata::JSONObj& zapata::RESTJob::configuration() {
	return this->__configuration;
}

void zapata::RESTJob::log(zapata::HTTPReq& _req, zapata::HTTPRep& _rep) {
	string _text(zapata::method_names[_req->method()]);
	_text.insert(0, "\033[38;5;105m");
	_text.insert(_text.length(), "\033[0m");
	_text.insert(_text.length(), " ");
	_text.insert(_text.length(), "\033[38;5;15m");
	_text.insert(_text.length(),  _req->url());
	if (_req->params().size() != 0) {
		_text.insert(_text.length(), "?");
		bool _first = true;
		for (auto i : _req->params()) {
			if (!_first) {
				_text.insert(_text.length(), "&");
			}
			_first = false;
			_text.insert(_text.length(), i.first);
			_text.insert(_text.length(), "=");
			_text.insert(_text.length(), i.second);
		}
	}
	_text.insert(_text.length(), "\033[0m");
	_text.insert(_text.length(), " <-> ");
	if (_rep->status() < 300) {
		_text.insert(_text.length(), "\033[38;5;118m");
	}
	else if (_rep->status() < 400) {
		_text.insert(_text.length(), "\033[38;5;172m");
	}
	else {
		_text.insert(_text.length(), "\033[38;5;88m");
	}
	_text.insert(_text.length(), zapata::status_names[_rep->status()]);
	_text.insert(_text.length(), "\033[0m");
	zapata::log(_text, zapata::sys);
}
