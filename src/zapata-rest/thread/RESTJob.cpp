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

zapata::RESTJob::RESTJob(string _key_file_path) : Job() {
	std::ifstream _key_file;
	_key_file.open(_key_file_path.data());
	this->__configuration = zapata::fromfile(_key_file);

	size_t _max = this->__configuration["zapata"]["core"]["max_descriptors_per_job"]->ok() ? (size_t) this->__configuration["zapata"]["core"]["max_descriptors_per_job"] : 64;
	this->__events = new epoll_event_t[ _max ];
	this->__epoll_fd = epoll_create1 (0);

	(* this)->loop([ this, _max ] (Job& _job) -> void {
		bool _debug = !!this->configuration()["zapata"]["rest"]["debug"];
	
		for (; true; ) {
			int _rv = epoll_wait(this->__epoll_fd, this->__events, _max, -1);
			if (_rv > 0) {
				for (int _idx = 0; _idx != _rv; _idx++) {
					if ((this->__events[_idx].events & EPOLLERR) || (this->__events[_idx].events & EPOLLHUP) || (!(this->__events[_idx].events & EPOLLIN))) {
						::close(this->__events[_idx].data.fd);
					}
					else {
						zapata::socketstream _cs(this->__events[_idx].data.fd);
						
						if (!_cs.is_open()) {
							::close(this->__events[_idx].data.fd);
						}
						else {
							zapata::HTTPRep _rep;
							zapata::HTTPReq _req;
							try  {
								_cs >> _req;
								this->__pool->trigger(_req, _rep);

								string _origin = _req->header("Origin");
								if (_origin.length() != 0) {
									_rep->header("Access-Control-Allow-Origin", _origin);
									_rep->header("Access-Control-Expose-Headers", REST_ACCESS_CONTROL_HEADERS);
								}

								if (zapata::log_lvl) {
									this->log(_req, _rep);
								}
								if (_cs.is_open()) {
									_rep->stringify(_cs);
									_cs << flush;
								}
								else {
									::close(this->__events[_idx].data.fd);
								}
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
								if (_cs.is_open()) {
									_rep->stringify(_cs);
									_cs << flush;
								}
								::close(this->__events[_idx].data.fd);
							}
							catch(zapata::ClosedException& e) {
								zapata::log(e.what(), zapata::error);
								::close(this->__events[_idx].data.fd);
							}

							_cs.unassign();
						}
					}
				}
			}
		}
	});
}

zapata::RESTJob::~RESTJob() {
	delete [] this->__events;
}

void zapata::RESTJob::assign(int _cs_fd) {
	pthread_mutex_lock((* this)->__mtx);

	epoll_event_t _event;
	_event.data.fd = _cs_fd;
	_event.events = EPOLLIN;
	epoll_ctl (this->__epoll_fd, EPOLL_CTL_ADD, _cs_fd, & _event);

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
