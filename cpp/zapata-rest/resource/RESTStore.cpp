#include <resource/RESTStore.h>

zapata::RESTStore::RESTStore(string _url_pattern) : RESTResource(_url_pattern) {
}

zapata::RESTStore::~RESTStore(){
}

void zapata::RESTStore::get(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTStore::put(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP201);
}

void zapata::RESTStore::post(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTStore::remove(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTStore::head(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTStore::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}
