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

#include <zapata/thread/RESTJob.h>

#include <zapata/net.h>
#include <zapata/http.h>
#include <zapata/exceptions/SyntaxErrorException.h>

zapata::RESTJob::RESTJob(zapata::JSONObj& _options) : Job(), __options(_options) {
	size_t _max = 1024;
	this->__epoll_fd = epoll_create1 (0);

	(* this)->loop([ this, _max ] (Job& _job) -> void {
		struct epoll_event _events[_max];
		for (; true; ) {
			int _rv = epoll_wait(this->__epoll_fd, _events, _max, -1);
			if (_rv > 0) {
				for (int _idx = 0; _idx != _rv; _idx++) {
					if ((_events[_idx].events & EPOLLERR) || (_events[_idx].events & EPOLLRDHUP) || (_events[_idx].events & EPOLLHUP) || (!(_events[_idx].events & EPOLLIN))) {
						epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _events[_idx].data.fd, nullptr);
						close(_events[_idx].data.fd);
					}
					else {
						zapata::socketstream _cs(_events[_idx].data.fd);
						
						if (!_cs.is_open()) {
							epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _events[_idx].data.fd, nullptr);
							close(_events[_idx].data.fd);
						}
						else {
							zapata::HTTPRep _rep;
							zapata::HTTPReq _req;
							try  {
								_cs >> _req;
								cout << _req << endl << flush;
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
									epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _events[_idx].data.fd, nullptr);
									close(_events[_idx].data.fd);
								}
							}
							catch(zapata::SyntaxErrorException& e) {
								zapata::log(e.what(), zapata::error, __HOST__, __LINE__, __FILE__);

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
								epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _events[_idx].data.fd, nullptr);
								close(_events[_idx].data.fd);
							}
							catch(zapata::ClosedException& e) {
								zapata::log(e.what(), zapata::error, __HOST__, __LINE__, __FILE__);
								epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _events[_idx].data.fd, nullptr);
								close(_events[_idx].data.fd);
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
}

void zapata::RESTJob::assign(int _cs_fd) {
	pthread_mutex_lock((* this)->__mtx);
	epoll_event_t _event;
	_event.data.fd = _cs_fd;
	_event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	epoll_ctl(this->__epoll_fd, EPOLL_CTL_ADD, _cs_fd, & _event);
	pthread_mutex_unlock((* this)->__mtx);
}

zapata::RESTPoolPtr zapata::RESTJob::pool() {
	return this->__pool;
}

void zapata::RESTJob::pool(zapata::RESTPoolPtr _pool) {
	this->__pool = _pool;
}

zapata::JSONObj& zapata::RESTJob::options() {
	return this->__options;
}

void zapata::RESTJob::log(zapata::HTTPReq& _req, zapata::HTTPRep& _rep) {
	string _text(zapata::method_names[_req->method()]);
	_text.insert(_text.length(), " ");
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
	_text.insert(_text.length(), " <-> ");
	_text.insert(_text.length(), zapata::status_names[_rep->status()]);
	zapata::log(_text, zapata::info, __HOST__, __LINE__, __FILE__);
}
