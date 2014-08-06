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

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <api/codes_rest.h>
#include <base/assert.h>
#include <base/smart_ptr.h>
#include <exceptions/AssertionException.h>
#include <http/HTTPObj.h>
#include <json/JSONObj.h>
#include <parsers/json.h>
#include <resource/ExchangeToken.h>
#include <text/html.h>

zapata::ExchangeToken::ExchangeToken() :
	zapata::RESTResource("^/auth/token") {
}

zapata::ExchangeToken::~ExchangeToken() {
}

void zapata::ExchangeToken::get(HTTPReq& _req, HTTPRep& _rep) {
	string _grant_type(_req->param("grant_type"));
	assertz(_req->param("grant_type").length() != 0, "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_req->param("client_id").length() != 0, "Parameter 'client_id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_req->param("client_secret").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_req->param("code").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_grant_type == string("user_code") || _req->param("redirect_uri").length() != 0, "Parameter 'redirect_uri' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

	zapata::JSONObj _token;
	bool _has_token = (_grant_type == string("user_code") && this->usrtoken(_req->param("client_id"), _req->param("client_secret"), _req->param("code"), _token)) || (_grant_type == string("autorization_code") && this->apptoken(_req->param("client_id"), _req->param("client_secret"), _req->param("code"), _token));

	string _redirect_uri(_req->param("redirect_uri"));
	if (_redirect_uri.length() != 0) {
		if (_redirect_uri.find("?") != string::npos) {
			_redirect_uri.insert(_redirect_uri.length(), "&");
		}
		else {
			_redirect_uri.insert(_redirect_uri.length(), "?");
		}

		if (!_has_token) {
			_redirect_uri.insert(_redirect_uri.length(), "error=unauthorized code");
		}
		else {
			_redirect_uri.insert(_redirect_uri.length(), "access_token=");
			_redirect_uri.insert(_redirect_uri.length(), _token["access_token"]);
		}
		_rep->status(zapata::HTTP307);
		_req << "Location" << _redirect_uri;
	}
	else {
		assertz(_has_token, "Unauthorized code", zapata::HTTP401, zapata::ERRGeneric);

		string _body;
		zapata::tostr(_body, _token);
		_rep->status(zapata::HTTP200);
		_rep << "Content-Type" << "application/json" << "Content-Length" << (size_t) _body.length();
		_rep->body(_body);
	}
}

void zapata::ExchangeToken::post(HTTPReq& _req, HTTPRep& _rep) {
	string _body = _req->body();
	assertz(_body.length() != 0, "Body ENTITY must be provided", zapata::HTTP412, zapata::ERRBodyEntityMustBeProvided);

	bool _is_json = _req->header("Content-Type").find("application/json") != string::npos;
	bool _is_form_encoded = _req->header("Content-Type").find("application/x-www-form-urlencoded") != string::npos;
	assertz(_is_json || _is_form_encoded, "Body ENTITY must be provided either in JSON or Form URL encoded format", zapata::HTTP406, zapata::ERRBodyEntityWrongContentType);

	zapata::JSONObj _params;
	if (_is_json) {
		zapata::fromstr(_body, _params);
	}
	else {
		zapata::fromformurlencoded(_body, _params);
	}

	string _grant_type((string) _params["grant_type"]);
	assertz(!!_params["grant_type"], "Parameter 'grant_type' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(!!_params["client_id"], "Parameter 'client_id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(!!_params["client_secret"], "Parameter 'client_secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(!!_params["code"], "Parameter 'code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(_grant_type == string("user_code") || !!_params["redirect_uri"], "Parameter 'redirect_uri' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

	zapata::JSONObj _token;

	bool _has_token = (_grant_type == string("user_code") && this->usrtoken((string) _params["client_id"], (string) _params["client_secret"], (string) _params["code"], _token)) || (_grant_type == string("autorization_code") && this->apptoken((string) _params["client_id"], (string) _params["client_secret"], (string) _params["code"], _token));

	string _redirect_uri(_req->param("redirect_uri"));
	if (_redirect_uri.length() != 0) {
		if (_redirect_uri.find("?") != string::npos) {
			_redirect_uri.insert(_redirect_uri.length(), "&");
		}
		else {
			_redirect_uri.insert(_redirect_uri.length(), "?");
		}

		if (!_has_token) {
			_redirect_uri.insert(_redirect_uri.length(), "error=unauthorized code");
		}
		else {
			_redirect_uri.insert(_redirect_uri.length(), "access_token=");
			_redirect_uri.insert(_redirect_uri.length(), _token["access_token"]);
		}
		_rep->status(zapata::HTTP303);
		_req << "Location" << _redirect_uri;
	}
	else {
		assertz(_has_token, "Unauthorized code", zapata::HTTP401, zapata::ERRGeneric);

		string _body;
		zapata::tostr(_body, _token);
		_rep->status(zapata::HTTP200);
		_rep << "Content-Type" << "application/json" << "Content-Length" << (size_t) _body.length();
		_rep->body(_body);
	}

}

