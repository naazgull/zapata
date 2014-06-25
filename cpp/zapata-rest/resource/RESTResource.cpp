#include <resource/RESTResource.h>

#include <api/RESTPool.h>

zapata::RESTResource::RESTResource(string _url_pattern) {
	this->__url_pattern = new regex_t();
	if (regcomp(this->__url_pattern, _url_pattern.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}
	this->__pool = NULL;
}

zapata::RESTResource::~RESTResource() {
	delete this->__url_pattern;
}

void zapata::RESTResource::get(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::put(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::post(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::remove(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::head(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::trace(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP200);

	string _body;
	zapata::tostr(_body, _req);
	_rep->body(_body);
}

void zapata::RESTResource::options(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP200);

	string _origin = _req->header("Origin");
	if (_origin.length() != 0) {
		_rep
			<< "Access-Control-Allow-Origin" << _origin
		                    << "Access-Control-Allow-Methods" << "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY"
		                    << "Access-Control-Allow-Headers" << REST_ACCESS_CONTROL_HEADERS
		                    << "Access-Control-Expose-Headers" << REST_ACCESS_CONTROL_HEADERS
		                    << "Access-Control-Max-Age" << "1728000";
	}

}

void zapata::RESTResource::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::connect(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

bool zapata::RESTResource::relations(HTTPReq& _req, JSONObj& _out) {
	return false;
}

void zapata::RESTResource::fields(HTTPReq& _req, JSONObj& _in_out) {

}

void zapata::RESTResource::embed(HTTPReq& _req, JSONObj& _in_out) {

}

bool zapata::RESTResource::matches(string _url) {
	if (regexec(this->__url_pattern, _url.c_str(), (size_t) (0), NULL, 0) == 0) {
		return true;
	}
	return false;
}

regex_t* zapata::RESTResource::pattern() {
	return this->__url_pattern;
}

zapata::JSONObj& zapata::RESTResource::configuration() {
	return *this->__configuration;
}

void zapata::RESTResource::configuration(JSONObj* _conf) {
	this->__configuration = _conf;
}

zapata::RESTPool& zapata::RESTResource::pool() {
	return *this->__pool;
}

void zapata::RESTResource::pool(RESTPool* _pool) {
	this->__pool = _pool;
}

bool zapata::RESTResource::allowed(HTTPReq& _req) {
	return true;
}

void zapata::RESTResource::invoke(HTTPReq& _req, HTTPRep& _rep, bool _is_ssl) {
	this->__pool->invoke(_req, _rep, _is_ssl);
}
