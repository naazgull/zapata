#include <auth/OAuth2.h>

zapata::OAuth2::OAuth2(string _url_pattern) : zapata::RESTDocument(_url_pattern) {
}

zapata::OAuth2::OAuth2(string _url_pattern, string _client_id, string _client_secret, string _scopes) : zapata::RESTDocument(_url_pattern) {
}

zapata::OAuth2::~OAuth2() {
}

void zapata::OAuth2::id(string _in) {
	this->__id.assign(_in.data());
}

string& zapata::OAuth2::id() {
	return this->__id;
}

void zapata::OAuth2::secret(string _in) {
	this->__secret.assign(_in.data());
}

string& zapata::OAuth2::secret() {
	return this->__secret;
}

void zapata::OAuth2::scopes(string _in) {
	this->__scopes.assign(_in.data());
}

string& zapata::OAuth2::scopes() {
	return this->__scopes;
}

void zapata::OAuth2::redirect(string _in) {
	this->__redirect_uri.assign(_in.data());
}

string& zapata::OAuth2::redirect() {
	return this->__redirect_uri;
}

void zapata::OAuth2::get(HTTPReq& _req, HTTPRep& _rep) {
	string _auth_uri(this->authEndpoint());

	if (_req->param("code") != "") {
	}
	else {
	}

	_rep->status(zapata::HTTP307);
	_rep << "Location" << _auth_uri;
}

void zapata::OAuth2::put(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);}

void zapata::OAuth2::remove(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::OAuth2::head(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::OAuth2::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}
