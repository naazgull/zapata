#include <resource/RESTDocument.h>

zapata::RESTDocument::RESTDocument(string _url_pattern) : RESTResource(_url_pattern) {
}

zapata::RESTDocument::~RESTDocument(){
}

void zapata::RESTDocument::get(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTDocument::put(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP201);
}

void zapata::RESTDocument::post(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP201);
}

void zapata::RESTDocument::remove(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP201);
}

void zapata::RESTDocument::head(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTDocument::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP201);
}
