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
	ifstream _key_file;
	_key_file.open(_key_file_path.data());
	this->__configuration = zapata::fromfile(_key_file);

	(* this)->loop([ this ] (Job& _job) -> void {
		bool _debug = !!this->__configuration["zapata"]["rest"]["debug"];

		for (; true; ) {
			(* this)->wait();

			if (this->__cur_fd.size() != 0) {
				pthread_mutex_lock((* this)->__mtx);
				int _cur_fd = (* this)->__cur_fd.front();
				(* this)->__cur_fd.pop();
				pthread_mutex_unlock((* this)->__mtx);
				socketstream _cs(_cur_fd);

				HTTPRep _rep;
				HTTPReq _req;
				for (; true; ) {
					try  {
						zapata::fromstream(_cs, _req);
						(* this)->__pool->trigger(_req, _rep);

						string _origin = _req.header("Origin");
						if (_origin.length() != 0) {
							_rep["headers"]
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
								string _text(zapata::method_names[_req.method()]);
								_text.insert(0, "\033[38;5;105m");
								_text.insert(_text.length(), "\033[0m");
								_text.insert(_text.length(), " ");
								_text.insert(_text.length(), "\033[38;5;15m");
								_text.insert(_text.length(),  _req.url());
								if (_req["params"]->obj()->size() != 0) {
									_text.insert(_text.length(), "?");
									JSONObj _params = (JSONObj&) _req["params"];
									bool _first = true;
									for (auto i : *_params) {
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
								if (_rep.status() < 300) {
									_text.insert(_text.length(), "\033[38;5;118m");
								}
								else if (_rep.status() < 400) {
									_text.insert(_text.length(), "\033[38;5;172m");
								}
								else {
									_text.insert(_text.length(), "\033[38;5;88m");
								}
								_text.insert(_text.length(), zapata::status_names[_rep.status()]);
								_text.insert(_text.length(), "\033[0m");
								zapata::log(_text, zapata::sys);
							}
						}
						_rep.stringify(_cs);
						_cs << flush;
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

						_rep.body(_text);
						_rep.status(zapata::HTTP400);
						_rep["headers"] << "Content-Type" << "application/json" << "Content-Length" << (long) _text.length();

						string _origin = _req.header("Origin");
						if (_origin.length() != 0) {
							_rep["headers"]
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
	});
}

zapata::RESTJob::~RESTJob() {
}

void zapata::RESTJob::assign(int _cs_fd) {
	pthread_mutex_lock(this->__mtx);
	this->__cur_fd.push(_cs_fd);
	pthread_mutex_unlock(this->__mtx);
}

zapata::RESTPool& zapata::RESTJob::pool() {
	return * this->__pool;
}

void zapata::RESTJob::pool(RESTPool* _pool) {
	this->__pool = _pool;
}

zapata::JSONObj& zapata::RESTJob::configuration() {
	return this->__configuration;
}
