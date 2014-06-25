#include <api/RESTPool.h>

#include <http/requester.h>

zapata::RESTPool::RESTPool() : __configuration(NULL) {
}

zapata::RESTPool::~RESTPool() {
}

zapata::JSONObj& zapata::RESTPool::configuration() {
	return *this->__configuration;
}

void zapata::RESTPool::configuration(JSONObj* _conf) {
	this->__configuration = _conf;
}

void zapata::RESTPool::add(RESTResource* _res) {
	_res->configuration(this->__configuration);
	this->__resources.push_back(_res);
}

void zapata::RESTPool::init(HTTPRep& _rep) {
	time_t _rawtime = time(NULL);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	_rep->status(zapata::HTTP404);
	_rep <<
		"Connection" << "close" <<
		"Server" << "zapata rest-ful server" <<
		"Cache-Control" << "max-age=3600" <<
		"Vary" << "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag" <<
		"Date" << string(_buffer_date) <<
		"Expires" << string(_buffer_expires);

}

void zapata::RESTPool::process(HTTPReq& _req, HTTPRep& _rep) {
	this->init(_rep);
	for (vector<RESTResource*>::iterator _i = this->__resources.begin(); _i != this->__resources.end(); _i++) {
		if ((*_i)->matches(_req->url())) {
			if (!(*_i)->allowed(_req)) {
				zapata::JSONObj _body;
				_body
					<< "error" << true
					<< "message" << "valid credentials are required to access this resource"
					<< "code" << 401;;

				_rep->status(zapata::HTTP401);
				_rep << "Content-Type" << "application/json";

				string _text;
				zapata::tostr(_text, _body);
				_rep->body(_text);
			}
			else {
				try {
					switch(_req->method()) {
						case zapata::HTTPGet : {
							(*_i)->get(_req, _rep);
							break;
						}
						case zapata::HTTPPut : {
							(*_i)->put(_req, _rep);
							break;
						}
						case zapata::HTTPPost : {
							(*_i)->post(_req, _rep);
							break;
						}
						case zapata::HTTPDelete : {
							(*_i)->remove(_req, _rep);
							break;
						}
						case zapata::HTTPHead : {
							(*_i)->head(_req, _rep);
							break;
						}
						case zapata::HTTPTrace : {
							(*_i)->trace(_req, _rep);
							break;
						}
						case zapata::HTTPOptions : {
							(*_i)->options(_req, _rep);
							break;
						}
						case zapata::HTTPPatch : {
							(*_i)->patch(_req, _rep);
							break;
						}
						case zapata::HTTPConnect : {
							(*_i)->connect(_req, _rep);
							break;
						}
					}
				}
				catch(zapata::AssertionException& _e) {
					zapata::JSONObj _body;
					_body
						<< "error" << true
						<< "assertion_failed" << _e.description()
						<< "message" << _e.what()
						<< "code" << _e.code();;

					string _text;
					zapata::tostr(_text, _body);

					_rep->body(_text);
					_rep->status((zapata::HTTPStatus) _e.code());
					_rep << "Content-Type" << "application/json" << "Content-Length" <<(int) _text.length();

					string _origin = _req->header("Origin");
					if (_origin.length() != 0) {
						_rep
							<< "Access-Control-Allow-Origin" << _origin
						                    << "Access-Control-Expose-Headers" << REST_ACCESS_CONTROL_HEADERS;
					}

				}
			}
			break;
		}
	}
}

void zapata::RESTPool::invoke(HTTPReq& _req, HTTPRep& _rep, bool _is_ssl) {
	string _host(_req->header("Host"));
	string _uri(_req->url());
	_uri.insert(0, _host);

	if (_host.length() == 0 || _uri.find((string) this->configuration()["zapata"]["rest"]["bind_url"]) != string::npos) {
		this->process(_req, _rep);
	}
	else {
		zapata::send(_req, _rep, _is_ssl);
	}
}

/*extern "C" void populate(zapata::RESTPool& _pool) {
}*/

