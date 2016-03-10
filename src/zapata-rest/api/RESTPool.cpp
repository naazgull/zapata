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

#include <zapata/api/RESTPool.h>

#include <zapata/http/requester.h>

zapata::RESTPool::RESTPool(zapata::JSONObj& _options) : __options( _options ), __self(this) {
	this->__default_get = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_put = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_post = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_delete = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_head = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_trace = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP200);

		string _body;
		zapata::tostr(_body, _req);
		_rep->body(_body);
		string _length;
		zapata::tostr(_length,  _body.length());
		_rep->header("Content-Length", _length);
	};
	this->__default_options = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
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
	this->__default_patch = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_connect = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTPoolPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
}

zapata::RESTPool::~RESTPool() {
	for (zapata::RESTHandlerStack::iterator _i = this->__resources.begin(); _i != this->__resources.end(); _i++) {
		delete _i->first;
	}
}

zapata::JSONObj& zapata::RESTPool::options() {
	return this->__options;
}

void zapata::RESTPool::on(vector<zapata::HTTPMethod> _events, string _regex, zapata::RESTHandler _handler) {
	regex_t * _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	std::vector<zapata::RESTHandler> _handlers;

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
		_handlers[_events[_i]] = _handler;
	}

	this->__resources.push_back(pair<regex_t *, std::vector<zapata::RESTHandler> >(_url_pattern, _handlers));
}

void zapata::RESTPool::on(zapata::HTTPMethod _event, string _regex, zapata::RESTHandler _handler) {
	regex_t * _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	std::vector<zapata::RESTHandler> _handlers;
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPGet? this->__default_get : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPPut ? this->__default_put : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPPost ? this->__default_post : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPDelete ? this->__default_delete : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPHead ? this->__default_head : _handler));
	_handlers.push_back(this->__default_trace);
	_handlers.push_back(this->__default_options);
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPPatch ? this->__default_patch : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::HTTPConnect ? this->__default_connect : _handler));

	this->__resources.push_back(pair<regex_t*, vector<zapata::RESTHandler> >(_url_pattern, _handlers));

}

void zapata::RESTPool::on(string _regex, zapata::RESTHandler _handler_set[9]) {
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

/*void zapata::RESTPool::on(string _regex, zapata::RESTHandler _get, zapata::RESTHandler _put, zapata::RESTHandler _post, zapata::RESTHandler _delete, zapata::RESTHandler _head, zapata::RESTHandler _trace, zapata::RESTHandler _options, zapata::RESTHandler _patch, zapata::RESTHandler _connect) {
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
}*/

void zapata::RESTPool::trigger(zapata::HTTPReq& _req, zapata::HTTPRep& _rep, bool _is_ssl) {
	string _host(_req->header("Host"));
	string _uri(_req->url());
	size_t _port_idx = string::npos;
	string _port((_port_idx = _host.find(":")) != string::npos ? _host.substr(_port_idx + 1) : "");
	_uri.insert(0, _host);
	_uri.insert(0, _is_ssl ? "https://" : "http://");
	string _bind_url((string) this->options()["zapata"]["rest"]["bind_url"]);

	if (_host.length() == 0 || _uri.find(_bind_url) != string::npos) {
		zapata::replace(_uri, _bind_url, "");
		_req->url(_uri);
		this->process(_req, _rep);
	}
	else if ((_host.find("127.0.0") != string::npos || _host.find("localhost") != string::npos) && _port == (string) this->options()["zapata"]["rest"]["port"]) {
		this->process(_req, _rep);
	}
	else {
		zapata::send(_req, _rep, _is_ssl);
	}
}

void zapata::RESTPool::trigger(string _url, zapata::HTTPReq& _req, zapata::HTTPRep& _rep, bool _is_ssl) {
	size_t _b = _url.find("://") + 3;
	size_t _e = _url.find("/", _b);
	string _domain(_url.substr(_b, _e - _b));
	string _path(_url.substr(_e));
	_req->header("Host", _domain);
	_req->url(_path);
	this->trigger(_req, _rep, _is_ssl);
}

void zapata::RESTPool::trigger(string _url, zapata::HTTPMethod _method, zapata::HTTPRep& _rep, bool _is_ssl) {
	zapata::HTTPReq _req;
	_req->method(_method);
	this->trigger(_url, _req, _rep, _is_ssl);
}

void zapata::RESTPool::init(zapata::HTTPRep& _rep) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	_rep->status(zapata::HTTP404);
	_rep->header("Server", "zapata RESTful server");
	_rep->header("Cache-Control", "max-age=3600");
	_rep->header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
	_rep->header("Date", string(_buffer_date));
	_rep->header("Expires", string(_buffer_expires));
}

void zapata::RESTPool::init(zapata::HTTPReq& _req) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	_req->method(zapata::HTTPGet);
	_req->header("Host", "localhost");
	_req->header("Accept", "application/json");
	_req->header("Accept-Charset", "utf-8");
	_req->header("Cache-Control", "no-cache");
	_req->header("Date", string(_buffer_date));
	_req->header("Expires", string(_buffer_expires));
	_req->header("User-Agent", "zapata RESTful server");
}

void zapata::RESTPool::resttify(zapata::JSONPtr _body, zapata::HTTPReq& _request) {
	if (_request->param("fields").length()) {

	}
	std::string _embed = _request->param("embed");
	if (_embed.length()) {
		zapata::JSONPtr _links = _body["links"];
		std::string _link;
		std::istringstream _iss;
		_iss.str(_embed);
		while (_iss.good()) {
			std::getline(_iss, _link, ',');

			std::string _rep_embed;
			size_t _idx = _link.find(".");
			if (_idx != std::string::npos) {
				_link.assign(_link.substr(0, _idx));
				_rep_embed.assign(_link.substr(_idx + 1));
			}

			if (_links[_link]->ok()) {
				zapata::HTTPReq _req;
				zapata::HTTPRep _rep;
				this->init(_req);
				_req->url(_links[_link]->str());

				if (_rep_embed.length() != 0) {
					_req->param("embed", _rep_embed);
				}

				this->process(_req, _rep);
				if (_rep->body().length() != 0) {
					zapata::JSONPtr _embed_body;
					std::istringstream _bss;
					_bss.str(_rep->body());
					_bss >> _embed_body;
					_body << _link << _embed_body;
				}
			}
		}
	}
}

void zapata::RESTPool::process(zapata::HTTPReq& _req, zapata::HTTPRep& _rep) {
	this->init(_rep);
	
	for (zapata::RESTHandlerStack::iterator _i = this->__resources.begin(); _i != this->__resources.end(); _i++) {
		if (regexec(_i->first, _req->url().c_str(), (size_t) (0), nullptr, 0) == 0) {
			try {
				_i->second[_req->method()](_req, _rep, make_ptr(this->__options), this->__self);
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
	if (_rep->header("Content-Length").length() == 0) {
		_rep->header("Content-Length", "0");
	}
}

extern "C" int zapata_rest() {
	return 1;
}