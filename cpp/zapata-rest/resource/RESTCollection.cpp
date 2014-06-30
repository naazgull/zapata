#include <resource/RESTCollection.h>

zapata::RESTCollection::RESTCollection(string _url_pattern) : RESTResource(_url_pattern) {
}

zapata::RESTCollection::~RESTCollection(){
}

void zapata::RESTCollection::get(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTCollection::put(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTCollection::post(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP202);
}

void zapata::RESTCollection::remove(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTCollection::head(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTCollection::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}
