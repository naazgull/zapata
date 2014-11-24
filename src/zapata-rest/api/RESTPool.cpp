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

#include <zapata/api/RESTPool.h>

#include <zapata/http/requester.h>

zapata::RESTPool::RESTPool() :
	__configuration(nullptr) {
	this->__default_get = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_put = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_post = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_delete = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_head = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_trace = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP200);

		string _body;
		zapata::tostr(_body, _req);
		_rep->body(_body);
		string _length;
		zapata::tostr(_length,  _body.length());
		_rep->header("Content-Length", _length);
	};
	this->__default_options = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP200);

		string _origin = _req->header("Origin");
		if (_origin.length() != 0) {
			_rep->header("Access-Control-Allow-Origin", _origin);
			_rep->header("Access-Control-Allow-Methods", "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY");
			_rep->header("Access-Control-Allow-Headers", REST_ACCESS_CONTROL_HEADERS);
			_rep->header("Access-Control-Expose-Headers", REST_ACCESS_CONTROL_HEADERS);
			_rep->header("Access-Control-Max-Age", "1728000");
		}
	};
	this->__default_patch = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_connect = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONObj& _config, zapata::RESTPool& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};

}

zapata::RESTPool::~RESTPool() {
	for (zapata::RESTHandlerStack::iterator _i = this->__resources.begin(); _i != this->__resources.end(); _i++) {
		delete _i->first;
	}
}

zapata::JSONObj& zapata::RESTPool::configuration() {
	return *this->__configuration;
}

void zapata::RESTPool::configuration(JSONObj* _conf) {
	this->__configuration = _conf;
}

void zapata::RESTPool::on(vector<zapata::HTTPMethod> _events, string _regex, zapata::RESTHandler _handler, zapata::RESTfulType _resource_type) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	vector<zapata::RESTHandler> _handlers;

	_handlers.push_back( this->__default_get);
	_handlers.push_back(this->__default_put);
	_handlers.push_back(this->__default_post);
	_handlers.push_back( this->__default_delete);
	_handlers.push_back( this->__default_head);
	_handlers.push_back(this->__default_trace);
	_handlers.push_back(this->__default_options);
	_handlers.push_back(this->__default_patch);
	_handlers.push_back(this->__default_connect);

	for (size_t _i = 0; _i != _events.size(); _i++) {
		switch (_events[_i]) {
			case zapata::HTTPGet : {
				_handlers[zapata::HTTPGet] =  (_resource_type == zapata::RESTfulController ? this->__default_get : _handler);
				break;
			}
			case zapata::HTTPPut : {
				_handlers[zapata::HTTPPut] = (_resource_type != zapata::RESTfulDocument && _resource_type != zapata::RESTfulStore ? this->__default_put : _handler);
				break;
			}
			case zapata::HTTPPost : {
				_handlers[zapata::HTTPPost] = (_resource_type != zapata::RESTfulController && _resource_type != zapata::RESTfulCollection ? this->__default_post : _handler);
				break;
			}
			case zapata::HTTPDelete : {
				_handlers[zapata::HTTPDelete] = (_resource_type != zapata::RESTfulDocument ? this->__default_delete : _handler);
				break;
			}
			case zapata::HTTPHead : {
				_handlers[zapata::HTTPHead] = (_resource_type == zapata::RESTfulController ? this->__default_head : _handler);
				break;
			}
			case zapata::HTTPPatch : {
				_handlers[zapata::HTTPPatch] = (_resource_type != zapata::RESTfulDocument ? this->__default_patch : _handler);
				break;
			}
			case zapata::HTTPConnect : {
				_handlers[zapata::HTTPConnect] = _handler;
				break;
			}
			default : {
			}

		}
	}

	this->__resources.push_back(pair<regex_t*, vector<zapata::RESTHandler> >(_url_pattern, _handlers));
}

void zapata::RESTPool::on(zapata::HTTPMethod _event, string _regex, zapata::RESTHandler _handler, zapata::RESTfulType _resource_type) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	vector<zapata::RESTHandler> _handlers;
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPGet || (_resource_type == zapata::RESTfulController) ? this->__default_get : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPPut || (_resource_type != zapata::RESTfulDocument && _resource_type != zapata::RESTfulStore) ? this->__default_put : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPPost || (_resource_type != zapata::RESTfulController && _resource_type != zapata::RESTfulCollection) ? this->__default_post : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPDelete || (_resource_type != zapata::RESTfulDocument) ? this->__default_delete : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPHead || (_resource_type == zapata::RESTfulController) ? this->__default_head : _handler));
	_handlers.push_back(this->__default_trace);
	_handlers.push_back(this->__default_options);
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPPatch || (_resource_type != zapata::RESTfulDocument) ? this->__default_patch : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPConnect ? this->__default_connect : _handler));

	this->__resources.push_back(pair<regex_t*, vector<zapata::RESTHandler> >(_url_pattern, _handlers));

}

void zapata::RESTPool::on(string _regex, zapata::RESTHandler _handler_set[9], zapata::RESTfulType _resource_type) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	vector<zapata::RESTHandler> _handlers;
	_handlers.push_back(_handler_set[zapata::HTTPGet] == nullptr ?  this->__default_get : _handler_set[zapata::HTTPGet]);
	_handlers.push_back(_handler_set[zapata::HTTPPut] == nullptr ?  this->__default_put : _handler_set[zapata::HTTPPut]);
	_handlers.push_back(_handler_set[zapata::HTTPPost] == nullptr ?  this->__default_post : _handler_set[zapata::HTTPPost]);
	_handlers.push_back(_handler_set[zapata::HTTPDelete] == nullptr ?  this->__default_delete : _handler_set[zapata::HTTPDelete]);
	_handlers.push_back(_handler_set[zapata::HTTPHead] == nullptr ?  this->__default_head : _handler_set[zapata::HTTPHead]);
	_handlers.push_back(this->__default_trace);
	_handlers.push_back(this->__default_options);
	_handlers.push_back(_handler_set[zapata::HTTPPatch] == nullptr ?  this->__default_patch : _handler_set[zapata::HTTPPatch]);
	_handlers.push_back(_handler_set[zapata::HTTPConnect] == nullptr ?  this->__default_connect : _handler_set[zapata::HTTPConnect]);

	this->__resources.push_back(pair<regex_t*, vector<zapata::RESTHandler> >(_url_pattern, _handlers));
}

void zapata::RESTPool::on(string _regex, zapata::RESTHandler _get, zapata::RESTHandler _put, zapata::RESTHandler _post, zapata::RESTHandler _delete, zapata::RESTHandler _head, zapata::RESTHandler _trace, zapata::RESTHandler _options, zapata::RESTHandler _patch, zapata::RESTHandler _connect, zapata::RESTfulType _resource_type) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	vector<zapata::RESTHandler> _handlers;
	_handlers.push_back(_get == nullptr ?  this->__default_get : _get);
	_handlers.push_back(_put == nullptr ?  this->__default_put : _put);
	_handlers.push_back(_post == nullptr ?  this->__default_post : _post);
	_handlers.push_back(_delete == nullptr ?  this->__default_delete : _delete);
	_handlers.push_back(_head == nullptr ?  this->__default_head : _head);
	_handlers.push_back(this->__default_trace);
	_handlers.push_back(this->__default_options);
	_handlers.push_back(_patch == nullptr ?  this->__default_patch : _patch);
	_handlers.push_back(_connect == nullptr ?  this->__default_connect : _connect);

	this->__resources.push_back(pair<regex_t*, vector<zapata::RESTHandler> >(_url_pattern, _handlers));
}

void zapata::RESTPool::trigger(HTTPReq& _req, HTTPRep& _rep, bool _is_ssl) {
	string _host(_req->header("Host"));
	string _uri(_req->url());
	size_t _port_idx = string::npos;
	string _port((_port_idx = _host.find(":")) != string::npos ? _host.substr(_port_idx + 1) : "");
	_uri.insert(0, _host);
	_uri.insert(0, _is_ssl ? "https://" : "http://");
	string _bind_url((string) this->configuration()["zapata"]["rest"]["bind_url"]);

	if (_host.length() == 0 || _uri.find(_bind_url) != string::npos) {
		zapata::replace(_uri, _bind_url, "");
		_req->url(_uri);
		this->process(_req, _rep);
	}
	else if ((_host.find("127.0.0") != string::npos || _host.find("localhost") != string::npos) && _port == (string) this->configuration()["zapata"]["rest"]["port"]) {
		this->process(_req, _rep);
	}
	else {
		zapata::send(_req, _rep, _is_ssl);
	}
}

void zapata::RESTPool::trigger(string _url, HTTPReq& _req, HTTPRep& _rep, bool _is_ssl) {
	size_t _b = _url.find("://") + 3;
	size_t _e = _url.find("/", _b);
	string _domain(_url.substr(_b, _e - _b));
	string _path(_url.substr(_e));
	_req->header("Host", _domain);
	_req->url(_path);
	this->trigger(_req, _rep, _is_ssl);
}

void zapata::RESTPool::trigger(string _url, HTTPMethod _method, HTTPRep& _rep, bool _is_ssl) {
	zapata::HTTPReq _req;
	_req->method(_method);
	this->trigger(_url, _req, _rep, _is_ssl);
}

void zapata::RESTPool::init(HTTPRep& _rep) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	_rep->status(zapata::HTTP404);
	_rep->header("Connection", "close");
	_rep->header("Server", "zapata rest-ful server");
	_rep->header("Cache-Control", "max-age=3600");
	_rep->header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
	_rep->header("Date", string(_buffer_date));
	_rep->header("Expires", string(_buffer_expires));

}

void zapata::RESTPool::process(HTTPReq& _req, HTTPRep& _rep) {
	this->init(_rep);
	for (zapata::RESTHandlerStack::iterator _i = this->__resources.begin(); _i != this->__resources.end(); _i++) {
		if (regexec(_i->first, _req->url().c_str(), (size_t) (0), nullptr, 0) == 0) {
			try {
				_i->second[_req->method()](_req, _rep, this->configuration(), *this);
			}
			catch (zapata::AssertionException& _e) {
				if (_e.status() > 399) {
					string _text;
					zapata::tostr(_text, JSON(
						"error" <<  true
						<< "assertion_failed" << _e.description()
						<< "message" << _e.what()
						<< "code" << _e.code()
					));
					_rep->header("Content-Type", "application/json");
					string _length;
					zapata::tostr(_length, _text.length());
					_rep->header("Content-Length", _length);

					_rep->body(_text);
				}

				_rep->status((zapata::HTTPStatus) _e.status());

				string _origin = _req->header("Origin");
				if (_origin.length() != 0) {
					_rep->header("Access-Control-Allow-Origin", _origin);
					_rep->header("Access-Control-Expose-Headers", REST_ACCESS_CONTROL_HEADERS);
				}

			}
		}
	}
}

extern "C" int zapata_rest() {
	return 1;
}