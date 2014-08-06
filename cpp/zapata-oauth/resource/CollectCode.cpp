/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <api/codes_rest.h>
#include <base/assert.h>
#include <base/smart_ptr.h>
#include <exceptions/AssertionException.h>
#include <http/HTTPObj.h>
#include <http/requester.h>
#include <json/JSONObj.h>
#include <resource/CollectCode.h>
#include <stddef.h>
#include <text/convert.h>

zapata::CollectCode::CollectCode() :
	zapata::RESTResource("^/auth/collect") {
}

zapata::CollectCode::~CollectCode() {
}

void zapata::CollectCode::get(HTTPReq& _req, HTTPRep& _rep) {
	assertz(_req->param("code").length() != 0, "Parameter 'code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_req->param("state").length() != 0, "Parameter 'state' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

	string _code(_req->param("code"));
	string _state(_req->param("state"));
	zapata::base64url_decode(_state);

	size_t _idx = _state.find("||");
	string _redirect_uri(_state.substr(_idx + 2));
	string _client_code(_state.substr(0, _idx));

	assertz(!!this->configuration()["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _client_config(this->configuration()["zapata"]["auth"]["clients"][_client_code]);

	string _client_id(_client_config["client_id"]);
	string _client_secret(_client_config["client_secret"]);
	string _type(_client_config["type"]);

	assertz(!!this->configuration()["zapata"]["auth"]["endpoints"][_type], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _endpoint_config(this->configuration()["zapata"]["auth"]["endpoints"][_type]);

	string _auth_endpoint(_endpoint_config["token"]);
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

	_auth_endpoint.insert(_auth_endpoint.length(), "&code=");
	_auth_endpoint.insert(_auth_endpoint.length(), _code);

	zapata::HTTPReq _token_req;
	zapata::HTTPRep _token_rep;
	_token_req->url(_auth_endpoint);
	zapata::init(_token_req);
	zapata::send(_token_req, _token_rep, true);

	string _access_token;
	if (_redirect_uri.find("?") != string::npos) {
		_redirect_uri.insert(_redirect_uri.length(), "&");
	}
	else {
		_redirect_uri.insert(_redirect_uri.length(), "?");
	}
	_redirect_uri.insert(_redirect_uri.length(), "access_token=");
	_redirect_uri.insert(_redirect_uri.length(), _access_token);

	_rep->status(zapata::HTTP307);
	_req << "Location" << _redirect_uri;
}

void zapata::CollectCode::post(HTTPReq& _req, HTTPRep& _rep) {
	assertz(_req->param("code").length() != 0, "Parameter 'code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_req->param("state").length() != 0, "Parameter 'state' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

	string _code(_req->param("code"));
	string _state(_req->param("state"));
	zapata::base64url_decode(_state);

	size_t _idx = _state.find("||");
	string _redirect_uri(_state.substr(_idx + 2));
	string _client_code(_state.substr(0, _idx));

	assertz(!!this->configuration()["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _client_config(this->configuration()["zapata"]["auth"]["clients"][_client_code]);

	string _client_id(_client_config["client_id"]);
	string _client_secret(_client_config["client_secret"]);
	string _type(_client_config["type"]);

	assertz(!!this->configuration()["zapata"]["auth"]["endpoints"][_type], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _endpoint_config(this->configuration()["zapata"]["auth"]["endpoints"][_type]);

	string _auth_endpoint(_endpoint_config["token"]);
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

	_auth_endpoint.insert(_auth_endpoint.length(), "&code=");
	_auth_endpoint.insert(_auth_endpoint.length(), _code);

	zapata::HTTPReq _token_req;
	zapata::HTTPRep _token_rep;
	_token_req->url(_auth_endpoint);
	zapata::init(_token_req);
	zapata::send(_token_req, _token_rep, true);

	string _access_token;
	if (_redirect_uri.find("?") != string::npos) {
		_redirect_uri.insert(_redirect_uri.length(), "&");
	}
	else {
		_redirect_uri.insert(_redirect_uri.length(), "?");
	}
	_redirect_uri.insert(_redirect_uri.length(), "access_token=");
	_redirect_uri.insert(_redirect_uri.length(), _access_token);

	_rep->status(zapata::HTTP303);
	_req << "Location" << _redirect_uri;
}

