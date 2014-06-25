#include <resource/ExchangeToken.h>

zapata::ExchangeToken::ExchangeToken() : zapata::RESTResource("^/auth/token") {
}

zapata::ExchangeToken::~ExchangeToken() {
}

void zapata::ExchangeToken::get(HTTPReq& _req, HTTPRep& _rep) {
	string _grant_type(_req->param("grant_type"));
	assertz(_req->param("grant_type").length() != 0, "Parameter 'grant_type' must be provided", zapata::HTTP412);
	assertz(_req->param("client_id").length() != 0, "Parameter 'client_id' must be provided", zapata::HTTP412);
	assertz(_req->param("client_secret").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412);
	assertz(_req->param("code").length() != 0, "Parameter 'client_secret' must be provided", zapata::HTTP412);
	assertz(_grant_type == string("user_code") || _req->param("redirect_uri").length() != 0, "Parameter 'redirect_uri' must be provided", zapata::HTTP412);

	string _token;

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
			_redirect_uri.insert(_redirect_uri.length(), _token);
		}
		_rep->status(zapata::HTTP307);
		_req << "Location" << _redirect_uri;
	}
	else {
		assertz(_has_token, "Unauthorized code", zapata::HTTP401);

		_token.insert(0, "{ \"access_token\" : \"");
		_token.insert(_token.length(), "\" }");
		_rep->status(zapata::HTTP200);
		_rep << "Content-Length" << (long) _token.length() << "Content-Type" << "application/json";
		_rep->body(_token);
	}
}

void zapata::ExchangeToken::post(HTTPReq& _req, HTTPRep& _rep) {
	string _body = _req->body();
	assertz(_body.length() != 0, "Body ENTITY must be provided", zapata::HTTP412);

	bool _is_json = _req->header("Content-Type").find("application/json") != string::npos;
	bool _is_form_encoded = _req->header("Content-Type").find("application/x-www-form-urlencoded") != string::npos;
	assertz(_is_json || _is_form_encoded, "Body ENTITY must be provided either in JSON or Form URL encoded format", zapata::HTTP406);

	zapata::JSONObj _params;
	if (_is_json) {
		zapata::fromstr(_body, _params);
	}
	else {
		zapata::fromformurlencoded(_body, _params);
	}

	string _grant_type((string) _params["grant_type"]);
	assertz(!!_params["grant_type"], "Parameter 'grant_type' must be provided", zapata::HTTP412);
	assertz(!!_params["client_id"], "Parameter 'client_id' must be provided", zapata::HTTP412);
	assertz(!!_params["client_secret"], "Parameter 'client_secret' must be provided", zapata::HTTP412);
	assertz(!!_params["code"], "Parameter 'client_secret' must be provided", zapata::HTTP412);
	assertz(_grant_type == string("user_code") || !!_params["redirect_uri"], "Parameter 'redirect_uri' must be provided", zapata::HTTP412);

	string _token;

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
			_redirect_uri.insert(_redirect_uri.length(), _token);
		}
		_rep->status(zapata::HTTP303);
		_req << "Location" << _redirect_uri;
	}
	else {
		assertz(_has_token, "Unauthorized code", zapata::HTTP401);

		_token.insert(0, "{ \"access_token\" : \"");
		_token.insert(_token.length(), "\" }");
		_rep->status(zapata::HTTP200);
		_rep << "Content-Length" << (long) _token.length() << "Content-Type" << "application/json";
		_rep->body(_token);
	}

}

