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

#include <zapata/rest/RESTEmitter.h>
#include <zapata/rest/requester.h>

zapata::RESTEmitter::RESTEmitter(zapata::JSONObj& _options) : __options( _options ), __self(this) {
	this->__default_get = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTEmitterPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_put = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTEmitterPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_post = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTEmitterPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_delete = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTEmitterPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_head = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTEmitterPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
	this->__default_options = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTEmitterPtr& _pool) -> void {
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
	this->__default_patch = [] (zapata::HTTPReq& _req, zapata::HTTPRep& _rep, zapata::JSONPtr _config, zapata::RESTEmitterPtr& _pool) -> void {
		_rep->status(zapata::HTTP405);
	};
}

zapata::RESTEmitter::~RESTEmitter() {
	for (auto _i : this->__resources) {
		regfree(_i.first);
		delete _i.first;
	}
}

void zapata::RESTEmitter::on(zapata::ev::Performative _event, string _regex,  zapata::ev::Handler _handler) {
	regex_t * _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	std::vector< zapata::ev::Handler> _handlers;
	_handlers.push_back((_handler == nullptr || _event != zapata::ev::Get? this->__default_get : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::ev::Put ? this->__default_put : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::ev::Post ? this->__default_post : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::ev::Delete ? this->__default_delete : _handler));
	_handlers.push_back((_handler == nullptr || _event != zapata::ev::Head ? this->__default_head : _handler));
	_handlers.push_back(this->__default_options);
	_handlers.push_back((_handler == nullptr || _event != zapata::ev::Patch ? this->__default_patch : _handler));

	this->__resources.push_back(pair<regex_t*, vector< zapata::ev::Handler> >(_url_pattern, _handlers));

}

void zapata::RESTEmitter::on(string _regex,  zapata::ev::Handler _handler_set[7]) {
	regex_t* _url_pattern = new regex_t();
	if (regcomp(_url_pattern, _regex.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}

	vector< zapata::ev::Handler> _handlers;
	_handlers.push_back(_handler_set[zapata::ev::Get] == nullptr ?  this->__default_get : _handler_set[zapata::ev::Get]);
	_handlers.push_back(_handler_set[zapata::ev::Put] == nullptr ?  this->__default_put : _handler_set[zapata::ev::Put]);
	_handlers.push_back(_handler_set[zapata::ev::Post] == nullptr ?  this->__default_post : _handler_set[zapata::ev::Post]);
	_handlers.push_back(_handler_set[zapata::ev::Delete] == nullptr ?  this->__default_delete : _handler_set[zapata::ev::Delete]);
	_handlers.push_back(_handler_set[zapata::ev::Head] == nullptr ?  this->__default_head : _handler_set[zapata::ev::Head]);
	_handlers.push_back(this->__default_options);
	_handlers.push_back(_handler_set[zapata::ev::Patch] == nullptr ?  this->__default_patch : _handler_set[zapata::ev::Patch]);

	this->__resources.push_back(pair<regex_t*, vector< zapata::ev::Handler> >(_url_pattern, _handlers));
}

zapata::JSONPtr zapata::RESTEmitter::trigger(zapata::ev::Performative _method, std::string _url, zapata::JSONPtr _payload) {
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
		return this->process(_req, _rep);
	}
	else if ((_host.find("127.0.0") != string::npos || _host.find("localhost") != string::npos) && _port == (string) this->options()["zapata"]["rest"]["port"]) {
		return this->process(_req, _rep);
	}
}

void zapata::RESTEmitter::init(zapata::HTTPRep& _rep) {
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

void zapata::RESTEmitter::init(zapata::HTTPReq& _req) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	_req->method(zapata::ev::Get);
	_req->header("Host", "localhost");
	_req->header("Accept", "application/json");
	_req->header("Accept-Charset", "utf-8");
	_req->header("Cache-Control", "no-cache");
	_req->header("Date", string(_buffer_date));
	_req->header("Expires", string(_buffer_expires));
	_req->header("User-Agent", "zapata RESTful server");
}

void zapata::RESTEmitter::resttify(zapata::JSONPtr _body, zapata::HTTPReq& _request) {
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

void zapata::RESTEmitter::process(zapata::HTTPReq& _req, zapata::HTTPRep& _rep) {
	this->init(_rep);
	
	for (auto _i : this->__resources) {
		if (regexec(_i.first, _req->url().c_str(), (size_t) (0), nullptr, 0) == 0) {
			try {
				std::istringstream _iss;
				_iss.str(_req->body());
				zapata::JSONPtr _payload;
				try {
					_iss >> _payload;
				}
				catch (...) {}
				zapata::JSONPtr _result = _i.second[_req->method()](_req->method(), _req->url(), _payload, make_ptr(this->__options), this->__self);
				if (_result->ok()) {
					_return << _result;
				}
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
