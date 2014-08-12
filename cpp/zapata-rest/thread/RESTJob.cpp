/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <thread/RESTJob.h>

#include <zapata/net.h>
#include <zapata/http.h>
#include <exceptions/SyntaxErrorException.h>

zapata::RESTJob::RESTJob(string _key_file_path) : Job(_key_file_path) {
}

zapata::RESTJob::~RESTJob() {
}

void zapata::RESTJob::run() {
	bool _debug = !!this->configuration()["zapata"]["rest"]["debug"];

	for (; true; ) {
		this->wait();

		if (this->__cur_fd.size() != 0) {
			pthread_mutex_lock(this->__mtx);
			int _cur_fd = this->__cur_fd.front();
			this->__cur_fd.pop();
			pthread_mutex_unlock(this->__mtx);
			socketstream _cs(_cur_fd);

			HTTPRep _rep;
			HTTPReq _req;
			for (; true; ) {
				try  {
					zapata::fromstream(_cs, _req);
					this->__pool->trigger(_req, _rep);

					string _origin = _req->header("Origin");
					if (_origin.length() != 0) {
						_rep
							<< "Access-Control-Allow-Origin" << _origin
						                    << "Access-Control-Expose-Headers" << REST_ACCESS_CONTROL_HEADERS;
					}

					if (zapata::log_lvl) {
						if (_debug) {
							string _text;
							zapata::tostr(_text, _req);
							_text.insert(_text.length(), "\n\n<->\n\n");
							zapata::tostr(_text, _rep);
							zapata::log(_text, zapata::sys);
						}
						else {
							string _text(zapata::method_names[_req->method()]);
							_text.insert(0, "\033[38;5;105m");
							_text.insert(_text.length(), "\033[0m");
							_text.insert(_text.length(), " ");
							_text.insert(_text.length(), "\033[38;5;15m");
							_text.insert(_text.length(),  _req->url());
							if (_req->params().size() != 0) {
								_text.insert(_text.length(), "?");
								for (HTTPReqRef::iterator i = _req->params().begin(); i != _req->params().end(); i++) {
									if (i != _req->params().begin()) {
										_text.insert(_text.length(), "&");
									}
									_text.insert(_text.length(), (*i)->first);
									_text.insert(_text.length(), "=");
									_text.insert(_text.length(), *(*i)->second);
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
					}
					_cs << _rep << flush;
					break;
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
					_rep << "Content-Type" << "application/json" << "Content-Length" << (long) _text.length();

					string _origin = _req->header("Origin");
					if (_origin.length() != 0) {
						_rep
							<< "Access-Control-Allow-Origin" << _origin
						                    << "Access-Control-Expose-Headers" << REST_ACCESS_CONTROL_HEADERS;
					}
				}
				catch(zapata::ClosedException& e) {
					zapata::log(e.what(), zapata::error);
					break;
				}
			}
		}
	}
}

void zapata::RESTJob::assign(int _cs_fd) {
	pthread_mutex_lock(this->__mtx);
	this->__cur_fd.push(_cs_fd);
	pthread_mutex_unlock(this->__mtx);
}

zapata::RESTPool& zapata::RESTJob::pool() {
	return *this->__pool;
}

void zapata::RESTJob::pool(RESTPool* _pool) {
	this->__pool = _pool;
}
