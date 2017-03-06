/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <zapata/oauth2.h>
#include <ctime>
#include <memory>

zpt::authenticator::OAuth2::OAuth2(zpt::json _options) : __options(_options) {
}

zpt::authenticator::OAuth2::~OAuth2() {
}

auto zpt::authenticator::OAuth2::options() -> zpt::json {
	return this->__options;
}
						  
auto zpt::authenticator::OAuth2::name() -> std::string {
	return "oauth2.0";
}

auto zpt::authenticator::OAuth2::authorize(zpt::ev::performative _performative, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	std::string _param;
	if (_performative == zpt::ev::Post) {
		_param.assign("payload");
	}
	else {
		_param.assign("params");
	}

	assertz_mandatory(_envelope[_param], "scope", 412);
	assertz_mandatory(_envelope[_param], "response_type", 412);
	std::string _response_type = (std::string) _envelope[_param]["response_type"];
	zpt::json _scope = _envelope[_param]["scope"];
	
	if (_response_type == "code") {
		assertz_mandatory(_envelope[_param], "client_id", 412);
		assertz_mandatory(_envelope[_param], "redirect_uri", 412);
		
		zpt::json _redirect_uri = _envelope[_param]["redirect_uri"];
		zpt::json _client_id = _envelope[_param]["client_id"];
		zpt::json _state = _envelope[_param]["state"];

		zpt::json _owner;
		try {
			_owner = this->retrieve_owner(_envelope);
		}
		catch(zpt::assertion& _e) {
			std::string _l_state(std::string("response_type=code") + std::string("&scope=") + (_scope->ok() ? _scope->str() : "defaults") + std::string("&client_id=") + _client_id->str() + std::string("&redirect_uri=") + _redirect_uri->str() + std::string("&state=") + ((std::string) _state));
			zpt::base64::encode(_l_state);
			zpt::url::encode(_l_state);
			std::string _login_url = std::string(_opts["url"]["login"]);	
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Set-Cookie", (std::string("deleted; name=oauth_session; domain=") + std::string(_opts["domain"]) + std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")), 
					"Location", (_login_url + (_login_url.find("?") != std::string::npos ? "&" : "?") + std::string("state=") + _l_state)
				}
			};
		}
		
		zpt::json _client;
		try {
			_client = this->retrieve_client(_envelope);
		}
		catch(zpt::assertion& _e) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Location", (std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) + std::string("error=true&reason=no+such+client"))
				}
			};
		}

		zpt::json _token = this->generate_token(
			{
				"response_type", _response_type,
				"client_id", _client_id,
				"scope", _scope,
				"owner_id", _owner["id"]
			}
		);
		this->store_token(_token);
		return {
			"status", (_performative == zpt::ev::Post ? 303 : 307),
			"headers", {
				"Location", (
					(std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) +
						std::string("code=") + std::string(_token["code"]) +
						(_state->ok() ? std::string("&state=") + _state->str() : std::string("")))
				)
			}
		};
	}
	else if (_response_type == "password") {
		assertz_mandatory(_envelope[_param], "client_id", 412);
		assertz_mandatory(_envelope[_param], "username", 412);
		assertz_mandatory(_envelope[_param], "password", 412);
		
		zpt::json _redirect_uri = _envelope[_param]["redirect_uri"];
		zpt::json _client_id = _envelope[_param]["client_id"];
		zpt::json _ownername = _envelope[_param]["username"];
		zpt::json _password = _envelope[_param]["password"];
		zpt::json _state = _envelope[_param]["state"];

		zpt::json _owner;
		try {
			_owner = this->retrieve_owner(std::string(_ownername), std::string(_password));
		}
		catch(zpt::assertion& _e) {
			if (_redirect_uri->is_string()) {
				std::string _l_state(std::string("response_type=code") + std::string("&scope=") + (_scope->ok() ? _scope->str() : "defaults") + std::string("&client_id=") + _client_id->str() + std::string("&redirect_uri=") + _redirect_uri->str() + std::string("&state=") + ((std::string) _state));
				zpt::base64::encode(_l_state);
				zpt::url::encode(_l_state);
				std::string _login_url = std::string(_opts["url"]["login"]);	
				return {
					"status", (_performative == zpt::ev::Post ? 303 : 307),
					"headers", {
						"Set-Cookie", (std::string("deleted; name=oauth_session; domain=") + std::string(_opts["domain"]) + std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")), 
						"Location", (_login_url + (_login_url.find("?") != std::string::npos ? "&" : "?") + std::string("state=") + _l_state)
					}
				};
			}
			throw;
		}
		
		zpt::json _client;
		try {
			_client = this->retrieve_client(_envelope);
		}
		catch(zpt::assertion& _e) {
			if (_redirect_uri->is_string()) {
				return {
					"status", (_performative == zpt::ev::Post ? 303 : 307),
					"headers", {
						"Location", (std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) + std::string("error=true&reason=no+such+client"))
					}
				};
			}
			throw;
		}
		
		zpt::json _token = this->generate_token(
			{
				"response_type", _response_type,
				"client_id", _client_id,
				"scope", _scope,
				"owner_id", _owner["id"]
			}
		);
		this->store_token(_token);
		if (_redirect_uri->is_string()) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Set-Cookie", (_token["access_token"]->str() + std::string("; owner=") + std::string(_owner["id"]) + std::string("; name=oauth_session; domain=") + std::string(_opts["domain"]) + std::string("; path=/; HttpOnly")), 
					"Location", (
						(std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? "&" : "?") +
							std::string("access_token=") + _token["access_token"]->str() +
							std::string("&refresh_token=") + _token["refresh_token"]->str() +
							std::string("&expires=") + ((std::string) _token["expires"]) +
							std::string("&state=") + ((std::string) _state))
					)
				}
			};
		}
		else {
			return {
				"status", 200,
				"payload", _token
			};
		}
	}					
	else if (_response_type == "implicit") {
		return zpt::undefined;
	}					
	else if (_response_type == "client_credentials") {
		assertz_mandatory(_envelope[_param], "client_id", 412);
		assertz_mandatory(_envelope[_param], "client_secret", 412);

		zpt::json _redirect_uri = _envelope[_param]["redirect_uri"];
		zpt::json _client_id = _envelope[_param]["client_id"];
		zpt::json _client_secret = _envelope[_param]["client_secret"];
		zpt::json _state = _envelope[_param]["state"];

		zpt::json _client;
		try {
			_client = this->retrieve_client(std::string(_client_id), std::string(_client_secret));
		}
		catch(zpt::assertion& _e) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Location", (std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) + std::string("error=true&reason=no+such+client"))
				}
			};
		}
		
		zpt::json _token = this->generate_token(
			{
				"response_type", _response_type,
				"client_id", _client_id,
				"client_secret", _client_secret,
				"scope", _scope
			}
		);
		this->store_token(_token);
		if (_redirect_uri->is_string()) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Location", (
						(std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? "&" : "?") +
							std::string("access_token=") + _token["access_token"]->str() +
							std::string("&refresh_token=") + _token["refresh_token"]->str() +
							std::string("&expires=") + ((std::string) _token["expires"]) +
							std::string("&state=") + ((std::string) _state))
					)
				}
			};
		}
		else {
			return {
				"status", 200,
				"payload", _token
			};
		}
	}					
	assertz(false, "\"response_type\" not valid", 400, 0);
}

auto zpt::authenticator::OAuth2::token(zpt::ev::performative _performative, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	assertz_mandatory(_envelope["payload"], "client_id", 412);
	assertz_mandatory(_envelope["payload"], "client_secret", 412);
	assertz_mandatory(_envelope["payload"], "code", 412);

	zpt::json _token = this->get_code(std::string(_envelope["payload"]["code"]));
	return {
		"status", 200,
		"payload", _token
	};
}

auto zpt::authenticator::OAuth2::refresh(zpt::ev::performative _performative, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	assertz_mandatory(_envelope["payload"], "grant_type", 412);
	assertz_mandatory(_envelope["payload"], "refresh_token", 412);

	zpt::json _refresh_token = this->get_refresh_token(std::string(_envelope["payload"]["refresh_token"]));
	assertz_mandatory(_refresh_token, "access_token", 412);
	this->remove_token(_refresh_token);
	zpt::json _token = this->generate_token(_refresh_token);
	this->store_token(_token);
	return {
		"status", 200,
		"payload", _token
	};
}

auto zpt::authenticator::OAuth2::validate(std::string _access_token, zpt::json _opts) -> zpt::json {
	zpt::json _token = this->get_token(_access_token);
	assertz(_token->ok(), "token is invalid", 403, 0);
	zpt::timestamp_t _now = zpt::timestamp();
	zpt::timestamp_t _expires = _token["expires"]->date();
	if (_expires < _now) {
		this->remove_token(_token);
	}
	assertz(_expires > _now, "token has expired", 403, 0);
	return _token;
}

auto zpt::authenticator::OAuth2::generate_token(zpt::json _data) -> zpt::json {
	std::string _client_id = std::string(_data["client_id"]);
	std::string _scope = std::string(_data["scope"]);
	std::string _grant_type = std::string(_data["response_type"]);
	std::string _owner_id = std::string(_data["owner"]);
	std::string _client_secret = std::string(_data["client_secret"]);
	std::string _access_token = zpt::generate::r_key(128);
	
	zpt::json _token = {
		"id", _access_token,
		"access_token", _access_token,
		"refresh_token", zpt::generate::r_key(64),
		"code", zpt::generate::r_key(64),
		"expires", (zpt::timestamp_t) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + 90L * 24L * 3600L * 1000L,
		"scope", zpt::split(_scope, ",", true),
		"grant_type", _grant_type,
		"client_id", _client_id,
		"owner_id", (_owner_id.length() != 0 ? zpt::json::string(_owner_id) : zpt::undefined)
	};
	
	return _token + this->get_roles_permissions(_token);
}
