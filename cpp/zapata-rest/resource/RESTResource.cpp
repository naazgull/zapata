#include <resource/RESTResource.h>

zapata::RESTResource::RESTResource(string _url_pattern) {
	this->__url_pattern = new regex_t();
	if (regcomp(this->__url_pattern, _url_pattern.c_str(), REG_EXTENDED | REG_NOSUB) != 0) {
	}
}

zapata::RESTResource::~RESTResource(){
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
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::options(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTResource::connect(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

