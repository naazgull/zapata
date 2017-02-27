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
	assertz_mandatory(_envelope["payload"], "scope", 412);
	assertz_mandatory(_envelope["payload"], "response_type", 412);

	std::string _response_type((std::string) _envelope["payload"]["response_type"]);
	
	if (_response_type == "code") {
		assertz_mandatory(_envelope["payload"], "client_id", 412);
		assertz_mandatory(_envelope["payload"], "redirect_uri", 412);
		zpt::json _redirect_uri = _envelope["payload"]["redirect_uri"];

		zpt::json _user;
		try {
			_user = this->retrieve_user(_envelope["headers"]);
		}
		catch(zpt::assertion& _e) {
			std::string _state(std::string("response_type=code") + std::string("&scope=") + (_envelope["payload"]["scope"]->ok() ? _envelope["payload"]["scope"]->str() : "defaults") + std::string("&client_id=") + _envelope["payload"]["client_id"]->str() + std::string("&redirect_uri=") + _envelope["payload"]["redirect_uri"]->str() + std::string("&state=") + ((std::string) _envelope["payload"]["state"]));
			zpt::base64::encode(_state);
			zpt::url::encode(_state);
			std::string _login_url = std::string(_opts["url"]["login"]);	
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Set-Cookie", (std::string("deleted; name=oauth_session; domain=") + std::string(_opts["domain"]) + std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")), 
					"Location", (_login_url + (_login_url.find("?") != std::string::npos ? "&" : "?") + std::string("state=") + _state)
				}
			};
		}
		
		zpt::json _application;
		try {
			_application = this->retrieve_application(_envelope);
		}
		catch(zpt::assertion& _e) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Location", (std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) + std::string("error=true&reason=no+such+application"))
				}
			};
		}
		
		std::string _code = this->store_code({
				"response_type", _response_type,
				"client_id", _envelope["payload"]["client_id"],
				"scope", _envelope["payload"]["scope"],
				"owner_id", _user["id"]
			}
		);
		return {
			"status", (_performative == zpt::ev::Post ? 303 : 307),
			"headers", {
				"Location", (
					(std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) +
						std::string("code=") + _code +
						(_envelope["payload"]["state"]->ok() ? std::string("&state=") + _envelope["payload"]["state"]->str() : std::string("")))
				)
			}
		};
	}
	else if (_response_type == "password") {
		assertz_mandatory(_envelope["payload"], "client_id", 412);
		assertz_mandatory(_envelope["payload"], "username", 412);
		assertz_mandatory(_envelope["payload"], "password", 412);
		zpt::json _redirect_uri = _envelope["payload"]["redirect_uri"];

		zpt::json _user;
		try {
			_user = this->retrieve_user(std::string(_envelope["payload"]["username"]), std::string(_envelope["payload"]["password"]));;
		}
		catch(zpt::assertion& _e) {
			if (_redirect_uri->is_string()) {
				std::string _state(std::string("response_type=code") + std::string("&scope=") + (_envelope["payload"]["scope"]->ok() ? _envelope["payload"]["scope"]->str() : "defaults") + std::string("&client_id=") + _envelope["payload"]["client_id"]->str() + std::string("&redirect_uri=") + _envelope["payload"]["redirect_uri"]->str() + std::string("&state=") + ((std::string) _envelope["payload"]["state"]));
				zpt::base64::encode(_state);
				zpt::url::encode(_state);
				std::string _login_url = std::string(_opts["url"]["login"]);	
				return {
					"status", (_performative == zpt::ev::Post ? 303 : 307),
					"headers", {
						"Set-Cookie", (std::string("deleted; name=oauth_session; domain=") + std::string(_opts["domain"]) + std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")), 
						"Location", (_login_url + (_login_url.find("?") != std::string::npos ? "&" : "?") + std::string("state=") + _state)
					}
				};
			}
			throw;
		}
		
		zpt::json _application;
		try {
			_application = this->retrieve_application(_envelope);
		}
		catch(zpt::assertion& _e) {
			if (_redirect_uri->is_string()) {
				return {
					"status", (_performative == zpt::ev::Post ? 303 : 307),
					"headers", {
						"Location", (std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) + std::string("error=true&reason=no+such+application"))
					}
				};
			}
			throw;
		}
		
		zpt::json _token = this->generate_token(
			{
				"response_type", _response_type,
				"client_id", _envelope["payload"]["client_id"],
				"scope", _envelope["payload"]["scope"],
				"owner_id", _user["id"]
			}
		);
		this->store_token(_token);
		if (_redirect_uri->is_string()) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Set-Cookie", (_token["access_token"]->str() + std::string("; name=oauth_session; domain=") + std::string(_opts["domain"]) + std::string("; path=/; HttpOnly")), 
					"Location", (
						(std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? "&" : "?") +
							std::string("access_token=") + _token["access_token"]->str() +
							std::string("&refresh_token=") + _token["refresh_token"]->str() +
							std::string("&expires=") + ((std::string) _token["expires"]) +
							std::string("&state=") + ((std::string) _envelope["payload"]["state"]))
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
		assertz_mandatory(_envelope["payload"], "client_id", 412);
		assertz_mandatory(_envelope["payload"], "client_secret", 412);
		zpt::json _redirect_uri = _envelope["payload"]["redirect_uri"];

		zpt::json _application;
		try {
			_application = this->retrieve_application(_envelope);
		}
		catch(zpt::assertion& _e) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Location", (std::string(_redirect_uri) + (std::string(_redirect_uri).find("?") != std::string::npos ? std::string("&") : std::string("?")) + std::string("error=true&reason=no+such+application"))
				}
			};
		}
		
		zpt::json _token = this->generate_token(
			{
				"response_type", _response_type,
				"client_id", _envelope["payload"]["client_id"],
				"client_secret", _envelope["payload"]["client_secret"],
				"scope", _envelope["payload"]["scope"]
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
							std::string("&state=") + ((std::string) _envelope["payload"]["state"]))
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

	zpt::json _code = this->get_code(std::string(_envelope["payload"]["code"]));
	zpt::json _token = this->generate_token(_code);
	this->store_token(_token);
	this->remove_code({ "code", std::string(_envelope["payload"]["code"]) });
	return {
		"status", 200,
		"payload", _token
	};
}

auto zpt::authenticator::OAuth2::refresh(zpt::ev::performative _performative, zpt::json _envelope, zpt::json _opts) -> zpt::json {
	assertz_mandatory(_envelope["payload"], "grant_type", 412);
	assertz_mandatory(_envelope["payload"], "refresh_token", 412);

	zpt::json _refresh_token = this->get_refresh_token(std::string(_envelope["payload"]["refresh_token"]));
	zpt::json _token = this->generate_token(_refresh_token);
	this->store_token(_token);
	this->remove_token(_refresh_token);
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
	std::string _grant_type = std::string(_data["repsonse_type"]);
	std::string _owner_id = std::string(_data["owner"]);
	std::string _client_secret = std::string(_data["client_secret"]);
	
	std::string _access_token(zpt::generate::r_key(128));
	std::string _refresh_token(zpt::generate::r_key(64));
	zpt::timestamp_t _expires = (zpt::timestamp_t) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + 90L * 24L * 3600L * 1000L;
	zpt::json _token = {
		"id", _access_token,
		"access_token", _access_token,
		"refresh_token", _refresh_token,
		"expires", _expires,
		"scope", zpt::split(_scope, ",", true),
		"grant_type", _grant_type,
		"client_id", _client_id,
		"owner_id", _owner_id
	};
	
	return _token;
}
