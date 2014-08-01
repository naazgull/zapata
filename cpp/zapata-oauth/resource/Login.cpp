#include <api/codes_rest.h>
#include <base/assert.h>
#include <base/smart_ptr.h>
#include <exceptions/AssertionException.h>
#include <http/HTTPObj.h>
#include <json/JSONObj.h>
#include <parsers/json.h>
#include <resource/Login.h>
#include <text/convert.h>
#include <text/html.h>

zapata::Login::Login() :
	zapata::RESTController("^/auth/login$") {
}

zapata::Login::~Login() {
}

void zapata::Login::post(HTTPReq& _req, HTTPRep& _rep) {

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

	assertz(!!_params["id"], "Parameter 'id' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(!!_params["secret"], "Parameter 'secret' must be provided", zapata::HTTP412, zapata::ERRRequiredField);
	assertz(!!_params["client_code"], "Parameter 'client_code' must be provided", zapata::HTTP412, zapata::ERRRequiredField);

	string _client_code(_params["client_code"]);
	assertz(!!this->configuration()["zapata"]["auth"]["clients"][_client_code], "No such 'client_code' found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _client_config(this->configuration()["zapata"]["auth"]["clients"][_client_code]);

	string _type(_client_config["type"]);
	assertz(!!this->configuration()["zapata"]["auth"]["endpoints"][_type], "No such endpoint type found in the configuration file", zapata::HTTP404, zapata::ERRConfigParameterNotFound);
	zapata::JSONObj _endpoint_config(this->configuration()["zapata"]["auth"]["endpoints"][_type]);

	string _code;
	if (this->authenticate((string) _params["id"], (string) _params["secret"], _code)) {
		string _redirect_uri(_req->param("redirect_uri"));

		string _redirect(_endpoint_config["token"]);
		if (_redirect_uri.length() != 0) {
			_redirect.insert(_redirect.length(), "?code=");
			_redirect.insert(_redirect.length(), _code);

			zapata::url_encode(_redirect_uri);
			_redirect.insert(_redirect.length(), "redirect_uri=");
			_redirect.insert(_redirect.length(), _redirect_uri);

			_rep->status(zapata::HTTP303);
			_req << "Location" << _redirect;
		}
		else {
			string _token_body_s;
			zapata::JSONObj _token_body;
			_token_body << "grant_type" << "user_code" << "client_id" << (string) _params["id"] << "client_secret" << (string) _params["secret"] << "code" << _code;
			zapata::tostr(_token_body_s, _token_body);

			//_redirect.insert(0, this->configuration()["zapata"]["rest"]["bind_url"]);

			zapata::HTTPReq _token_req;
			_token_req << "Content-Length" << (long) _token_body_s.length() << "Content-Type" << "application/json";
			_token_req->method(zapata::HTTPPost);
			_token_req->body(_token_body_s);

			zapata::HTTPRep _token_rep;
			this->invoke(_redirect, _token_req, _token_rep);

			string _token = _token_rep->body();

			_rep->status(zapata::HTTP200);
			_rep << "Content-Type" << "application/json" << "Content-Length" << (long) _token.length();
			_rep->body(_token);
		}
	}
}
