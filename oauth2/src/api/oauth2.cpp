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
#include <zapata/rest.h>
#include <zapata/redis.h>
#include <zapata/auth.h>
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
					"Set-Cookie", (std::string("deleted; name=oauth_session; domain=") + _emitter->options()["domain"]->str() + std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")), 
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
				"client_id", _envelope["payload"]["client_id"],
				"scope", _envelope["payload"]["scope"],
				"application", _application, 
				"user", _user
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
						"Set-Cookie", (std::string("deleted; name=oauth_session; domain=") + _emitter->options()["domain"]->str() + std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")), 
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
				"client", _application, 
				"owner", _user
			}
		);
		this->store_token(_token);
		if (_redirect_uri->is_string()) {
			return {
				"status", (_performative == zpt::ev::Post ? 303 : 307),
				"headers", {
					"Set-Cookie", (_token["access_token"]->str() + std::string("; name=oauth_session; domain=") + _emitter->options()["domain"]->str() + std::string("; path=/; HttpOnly")), 
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
				"scope", _envelope["payload"]["scope"],
				"client", _application
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

auto zpt::authenticator::OAuth2::token(zpt::ev::performative _performative, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
	assertz(
		_envelope["payload"]->ok() &&
		_envelope["payload"]["client_id"]->ok() &&
		_envelope["payload"]["client_secret"]->ok() &&
		_envelope["payload"]["code"]->ok(),
		"required fields: 'client_id', 'client_secret' & 'code'", 412, 0);

	zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.oauth").get();
	std::string _client_id((std::string) _envelope["payload"]["client_id"]);
	zpt::json _code = _db->get("codes", zpt::path::join({ _emitter->version(), "apps", _client_id, "codes", _envelope["payload"]["code"] }));
	assertz(_code->ok(), "invalid parameter: no such code", 404, 0);
	std::string _client_secret((std::string) _envelope["payload"]["client_secret"]);	
	zpt::json _token = this->generate_token(_code["user"], _code["application"], _client_id, _client_secret, std::string(_code["scope"]), "code",  _emitter);
	_db->remove("codes", zpt::path::join({ _emitter->version(), "apps", _client_id, "codes", _envelope["payload"]["code"] }));

	return {
		"status", 200,
		"payload", _token
	};
}

auto zpt::authenticator::OAuth2::refresh(zpt::ev::performative _performative, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
	assertz(
		_envelope["payload"]->ok() &&
		_envelope["payload"]["grant_type"]->ok() &&
		_envelope["payload"]["refresh_token"]->ok(),
		"required fields: 'grant_type' & 'refresh_token'", 412, 0);

	zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.oauth").get();
	std::string _client_id((std::string) _envelope["payload"]["client_id"]);
	zpt::json _refresh = _db->get("tokens/refresh", zpt::path::join({ _emitter->version(), "oauth2.0", "tokens", "refresh", _envelope["payload"]["refresh_token"] }));
	assertz(_refresh->ok(), "invalid parameter: no such refresh token", 404, 0);
	zpt::json _token = this->generate_token(_refresh["owner"], _refresh["application"], _refresh["application"]["client_id"]->str(), _refresh["application"]["client_secret"]->str(),  std::string(_refresh["scope"]), "code", _emitter);

	return {
		"status", 200,
		"payload", _token
	};
}

auto zpt::authenticator::OAuth2::generate_token(std::string _owner_url, std::string _application_url, std::string _client_id, std::string _client_secret, std::string _scope, std::string _grant_type, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::json _owner = _emitter->route(zpt::ev::Get, _owner_url, zpt::undefined);
	assertz(_owner["status"] == 200, "invalid resource: no such resource owner", 404, 0);
	return this->generate_token(_owner["payload"], _application_url, _client_id, _client_secret, _scope, _grant_type, _emitter);
}

auto zpt::authenticator::OAuth2::generate_token(zpt::json _owner, std::string _application_url, std::string _client_id, std::string _client_secret, std::string _scope, std::string _grant_type, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::json _application = _emitter->route(zpt::ev::Get, _application_url, zpt::undefined);
	assertz(_application["status"] == 200, "invalid resource: no such application", 404, 0);
	return this->generate_token(_owner, _application["payload"], _client_id, _client_secret, _scope, _grant_type, _emitter);
}

auto zpt::authenticator::OAuth2::generate_token(zpt::json _owner, zpt::json _application, std::string _client_id, std::string _client_secret, std::string _scope, std::string _grant_type, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.oauth").get();
	assertz(_client_secret.length() == 0 || _application["client_secret"]->str() == _client_secret, "invalid parameter: no such client secret", 401, 0);
	std::string _access_token = zpt::rest::authorization::serialize({ "owner", _owner["id"]->str(), "application", _application["id"]->str(), "grant_type", _grant_type });
	std::string _refresh_token(zpt::generate_key(64));
	zpt::timestamp_t _expires = (zpt::timestamp_t) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + ((_owner["username"]->str() == "root" && _client_id == "00000000-0000-0000-0000-000000000000" ? 400 : 1) * 90L * 24L * 3600L * 1000L);
	zpt::json _token = {
		"id", _access_token,
		"access_token", _access_token,
		"refresh_token", _refresh_token,
		"expires", _expires,
		"scope", zpt::rest::scopes::deserialize(_scope),
		"grant_type", _grant_type,
		"application", _application,
		"owner", _owner
	};
	_db->insert("tokens", std::string("/") + _emitter->version() + std::string("/oauth2.0/tokens"), _token);

	zpt::json _refresh = {
		"id", _refresh_token,
		"access_token", _access_token,
		"refresh_token", _refresh_token,
		"expires", _expires,
		"scope", zpt::rest::scopes::deserialize(_scope),
		"grant_type", _grant_type,
		"application", _application,
		"owner", _owner
	};
	_db->insert("tokens/refresh", std::string("/") + _emitter->version() + std::string("/oauth2.0/tokens/refresh"), _refresh);
	
	return {
		"access_token", _access_token,
		"refresh_token", _refresh_token,
		"expires", _expires
	};
}

auto zpt::authenticator::OAuth2::validate(std::string _access_token, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::redis::Client* _db = (zpt::redis::Client*) _emitter->get_kb("redis.oauth").get();
	zpt::json _token = _db->get("tokens", std::string("/") + _emitter->version() + std::string("/oauth2.0/tokens/") + _access_token);
	assertz(_token->ok(), "token is invalid", 403, 0);
	zpt::timestamp_t _now = zpt::timestamp();
	zpt::timestamp_t _expires = _token["expires"]->date();
	if (_expires < _now) {
		_db->remove("tokens", std::string("/") + _emitter->version() + std::string("/oauth2.0/tokens/") + _access_token);
		_db->remove("tokens/refresh", std::string("/") + _emitter->version() + std::string("/oauth2.0/tokens/refresh/") + _token["refresh_token"]->str());
	}
	assertz(_expires > _now, "token has expired", 403, 0);
	return _token;
}
