#include <thread/RESTJob.h>

#include <zapata/net.h>
#include <zapata/http.h>
#include <exceptions/SyntaxErrorException.h>

zapata::RESTJob::RESTJob(string _key_file_path) : Job(_key_file_path), __cur_fd(-1) {
}

zapata::RESTJob::~RESTJob() {
}

void zapata::RESTJob::run() {
	bool _debug = !!this->__pool->configuration()["zapata"]["rest"]["debug"];

	for (; true; ) {
		this->wait();
		if (this->__cur_fd != -1) {
			socketstream _cs(this->__cur_fd);

			HTTPRep _rep;
			HTTPReq _req;
			for (; true; ) {
				try  {
					zapata::fromstream(_cs, _req);
					this->__pool->process(_req, _rep);

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
			this->__cur_fd = -1;
		}

	}
}

void zapata::RESTJob::assign(int _cs_fd) {
	this->__cur_fd = _cs_fd;
}

zapata::RESTPool& zapata::RESTJob::pool() {
	return *this->__pool;
}

void zapata::RESTJob::pool(RESTPool* _pool) {
	this->__pool = _pool;
}
