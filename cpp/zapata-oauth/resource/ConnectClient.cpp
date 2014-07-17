#include <api/codes_rest.h>
#include <base/assert.h>
#include <base/smart_ptr.h>
#include <exceptions/AssertionException.h>
#include <http/HTTPObj.h>
#include <json/JSONObj.h>
#include <parsers/json.h>
#include <resource/ConnectClient.h>
#include <text/html.h>

zapata::ConnectClient::ConnectClient() :
	zapata::RESTResource("^/auth/connect") {
}

zapata::ConnectClient::~ConnectClient() {
}

void zapata::ConnectClient::get(HTTPReq& _req, HTTPRep& _rep) {
	//assertz(_req->param("grant_type").length() != 0, "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_req->param("client_code").length() != 0, "Parameter 'client_code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

	string _grant_type("code");
	string _client_code(_req->param("client_code"));
	string _redirect_uri(_req->param("redirect_uri"));

	assertz(!!this->configuration()["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _client_config(this->configuration()["zapata"]["auth"]["clients"][_client_code]);

	string _client_id(_client_config["client_id"]);
	string _client_secret(_client_config["client_secret"]);
	string _type(_client_config["type"]);

	assertz(!!this->configuration()["zapata"]["auth"]["endpoints"][_type], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _endpoint_config(this->configuration()["zapata"]["auth"]["endpoints"][_type]);

	string _auth_endpoint(_endpoint_config["authorization"]);
	if (_auth_endpoint.find("?") != string::npos) {
		_auth_endpoint.insert(_auth_endpoint.length(), "&");
	}
	else {
		_auth_endpoint.insert(_auth_endpoint.length(), "?");
	}

	_auth_endpoint.insert(_auth_endpoint.length(), "client_id=");
	_auth_endpoint.insert(_auth_endpoint.length(), _client_id);

	_auth_endpoint.insert(_auth_endpoint.length(), "&client_secret=");
	_auth_endpoint.insert(_auth_endpoint.length(), _client_secret);

	_auth_endpoint.insert(_auth_endpoint.length(), "&grant_type=");
	_auth_endpoint.insert(_auth_endpoint.length(), _grant_type);

	string _self_uri(this->configuration()["zapata"]["rest"]["rest"]["bind_url"]);
	_self_uri.insert(_self_uri.length(), "/auth/collect");
	zapata::url_encode(_self_uri);
	_auth_endpoint.insert(_auth_endpoint.length(), "&redirect_uri=");
	_auth_endpoint.insert(_auth_endpoint.length(), _self_uri);

	if (_redirect_uri.length() != 0) {
		_auth_endpoint.insert(_auth_endpoint.length(), "&state=");
		_redirect_uri.insert(0, "||");
		_redirect_uri.insert(0, _client_code);
		zapata::base64url_encode(_redirect_uri);
		_auth_endpoint.insert(_auth_endpoint.length(), _redirect_uri);
	}

	_rep->status(zapata::HTTP307);
	_req << "Location" << _auth_endpoint;
}

void zapata::ConnectClient::post(HTTPReq& _req, HTTPRep& _rep) {
	assertz(_req->param("grant_type").length() != 0, "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_req->param("client_code").length() != 0, "Parameter 'client_code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

	string _client_code(_req->param("client_code"));
	string _grant_type(_req->param("grant_type"));
	string _redirect_uri(_req->param("redirect_uri"));

	assertz(!!this->configuration()["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _client_config(this->configuration()["zapata"]["auth"]["clients"][_client_code]);

	string _client_id(_client_config["client_id"]);
	string _client_secret(_client_config["client_secret"]);
	string _type(_client_config["type"]);

	assertz(!!this->configuration()["zapata"]["auth"]["endpoints"][_type], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _endpoint_config(this->configuration()["zapata"]["auth"]["endpoints"][_type]);

	string _auth_endpoint(_endpoint_config["authorization"]);
	if (_auth_endpoint.find("?") != string::npos) {
		_auth_endpoint.insert(_auth_endpoint.length(), "&");
	}
	else {
		_auth_endpoint.insert(_auth_endpoint.length(), "?");
	}

	_auth_endpoint.insert(_auth_endpoint.length(), "client_id=");
	_auth_endpoint.insert(_auth_endpoint.length(), _client_id);

	_auth_endpoint.insert(_auth_endpoint.length(), "&client_secret=");
	_auth_endpoint.insert(_auth_endpoint.length(), _client_secret);

	_auth_endpoint.insert(_auth_endpoint.length(), "&grant_type=");
	_auth_endpoint.insert(_auth_endpoint.length(), _grant_type);

	string _self_uri(this->configuration()["zapata"]["rest"]["rest"]["bind_url"]);
	_self_uri.insert(_self_uri.length(), "/auth/collect");
	zapata::url_encode(_self_uri);
	_auth_endpoint.insert(_auth_endpoint.length(), "&redirect_uri=");
	_auth_endpoint.insert(_auth_endpoint.length(), _self_uri);

	if (_redirect_uri.length() != 0) {
		_auth_endpoint.insert(_auth_endpoint.length(), "&state=");
		zapata::base64url_encode(_redirect_uri);
		_auth_endpoint.insert(_auth_endpoint.length(), _redirect_uri);
	}

	_rep->status(zapata::HTTP303);
	_req << "Location" << _auth_endpoint;

}

