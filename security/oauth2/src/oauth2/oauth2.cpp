/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <ctime>
#include <memory>
#include <zapata/rest.h>
#include <zapata/oauth2/oauth2.h>

auto
zpt::OAUTH2_TOKEN_PROVIDER() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

auto
zpt::OAUTH2_SERVER() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::auth::oauth2::server::server(zpt::auth::oauth2::token_provider_ptr _token_provider,
                                  zpt::json _options)
  : __token_provider{ _token_provider }
  , __options(_options) {
    expect(
      this->__token_provider.get() != nullptr, "a valid token provider must be provided", 500, 0);
}

zpt::auth::oauth2::server::~server() {}

auto
zpt::auth::oauth2::server::options() -> zpt::json {
    return this->__options;
}

auto
zpt::auth::oauth2::server::name() -> std::string {
    return "oauth2.0";
}

auto
zpt::auth::oauth2::server::authorize(zpt::performative _performative,
                                     zpt::json _envelope,
                                     zpt::json _opts) -> zpt::json {
    std::string _param{ "" };
    if (_performative == zpt::Post) { _param.assign("body"); }
    else {
        _param.assign("params");
    }

    expect_mandatory(_envelope[_param], "scope", 412);
    expect_mandatory(_envelope[_param], "response_type", 412);
    auto _response_type = _envelope[_param]["response_type"]->string();

    if (_response_type == "code") {
        return this->authorize_with_code(_performative, _envelope[_param], _envelope, _opts);
    }
    else if (_response_type == "password") {
        return this->authorize_with_password(_performative, _envelope[_param], _envelope, _opts);
    }
    else if (_response_type == "implicit") {
        return zpt::undefined;
    }
    else if (_response_type == "client_credentials") {
        return this->authorize_with_client_credentials(
          _performative, _envelope[_param], _envelope, _opts);
    }
    expect(false, "\"response_type\" not valid", 400, 0);
}

auto
zpt::auth::oauth2::server::authorize(std::string const& _topic,
                                     zpt::json _envelope,
                                     zpt::json _roles_needed) -> zpt::json {
    auto _access_token = zpt::auth::extract(_envelope);
    auto _identity = this->__token_provider->get_token(_access_token);
    expect(_identity["client_id"]->is_string(), "associated token isn't a valid token", 401, 0);
    expect(_identity["permissions"]->is_string(), "associated token isn't a valid token", 401, 0);
    expect(this->__token_provider->validate_roles_permissions(
             _envelope, _topic, _identity["permissions"]),
           "token didn't provide the necessary permissions",
           403,
           1);

    auto _roles_found{ 0ULL };
    auto _roles_granted = _identity["roles"];
    for (auto [_, __, _role_needed] : _roles_needed) {
        for (auto [_, __, _role_granted] : _roles_granted) {
            if (_role_needed == _role_granted) {
                ++_roles_found;
                break;
            }
        }
    }
    expect(
      _roles_found == _roles_needed->size(), "token didn't provide the necessary roles", 401, 0);
    return _identity;
}

auto
zpt::auth::oauth2::server::token(zpt::performative _performative,
                                 zpt::json _envelope,
                                 zpt::json _opts) -> zpt::json {
    std::string _param{ "" };
    if (_performative == zpt::Post) { _param.assign("body"); }
    else {
        _param.assign("params");
    }

    expect_mandatory(_envelope[_param], "client_id", 412);
    expect_mandatory(_envelope[_param], "client_secret", 412);
    expect_mandatory(_envelope[_param], "code", 412);

    auto _token = this->__token_provider->get_code(_envelope[_param]["code"]->string());
    return { "status", 200, "body", _token };
}

auto
zpt::auth::oauth2::server::refresh(zpt::performative _performative,
                                   zpt::json _envelope,
                                   zpt::json _opts) -> zpt::json {
    std::string _param{ "" };
    if (_performative == zpt::Post) { _param.assign("body"); }
    else {
        _param.assign("params");
    }

    expect_mandatory(_envelope[_param], "grant_type", 412);
    expect_mandatory(_envelope[_param], "refresh_token", 412);

    auto _refresh_token =
      this->__token_provider->get_refresh_token(_envelope[_param]["refresh_token"]->string());
    expect_mandatory(_refresh_token, "access_token", 412);
    this->__token_provider->remove_token(_refresh_token);
    auto _token = this->generate_token(_refresh_token);
    this->__token_provider->store_token(_token);
    return { "status", 200, "body", _token };
}

auto
zpt::auth::oauth2::server::validate(std::string const& _access_token, zpt::json _opts)
  -> zpt::json {
    auto _token = this->__token_provider->get_token(_access_token);
    expect(_token->ok(), "token is invalid", 403, 0);
    auto _now = zpt::timestamp();
    auto _expires = _token["expires"]->date();
    if (_expires < _now) { this->__token_provider->remove_token(_token); }
    expect(_expires > _now, "token has expired", 403, 0);
    return _token;
}

auto
zpt::auth::oauth2::server::authorize_with_code(zpt::performative _performative,
                                               zpt::json _request,
                                               zpt::json _envelope,
                                               zpt::json _opts) -> zpt::json {
    expect_mandatory(_request, "client_id", 412);
    expect_mandatory(_request, "redirect_uri", 412);
    expect_mandatory(_opts, "domain", 412);
    expect_mandatory(_opts, "login_url", 412);

    auto _scope = zpt::split(static_cast<std::string>(_request["scope"]), ",");
    auto _redirect_uri = _request["redirect_uri"];
    auto _client_id = _request["client_id"];
    auto _state = _request["state"];

    zpt::json _owner;
    try {
        _owner = this->__token_provider->retrieve_owner(_envelope);
    }
    catch (zpt::failed_expectation const& _e) {
        std::string _l_state(
          std::string("response_type=code") + std::string("&scope=") +
          (_scope->ok() ? static_cast<std::string>(_request["scope"]) : "defaults") +
          std::string("&client_id=") + _client_id->string() + std::string("&redirect_uri=") +
          _redirect_uri->string() + std::string("&state=") + static_cast<std::string>(_state));
        zpt::base64::encode(_l_state);
        zpt::url::encode(_l_state);
        auto _login_url = _opts["login_url"]->string();
        return { "status",
                 (_performative == zpt::Post ? 303 : 307),
                 "headers",
                 { "Set-Cookie",
                   (std::string("deleted; name=oauth_session; domain=") +
                    _opts["domain"]->string() +
                    std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")),
                   "Location",
                   (_login_url + (_login_url.find("?") != std::string::npos ? "&" : "?") +
                    std::string("state=") + _l_state) } };
    }

    zpt::json _client;
    try {
        _client = this->__token_provider->retrieve_client(_envelope);
    }
    catch (zpt::failed_expectation const& _e) {
        return { "status",
                 (_performative == zpt::Post ? 303 : 307),
                 "headers",
                 { "Location",
                   (_redirect_uri->string() +
                    (_redirect_uri->string().find("?") != std::string::npos ? std::string("&")
                                                                            : std::string("?")) +
                    std::string("error=true&reason=no+such+client")) } };
    }

    auto _token = this->generate_token({ "response_type",
                                         "code",
                                         "client_id",
                                         _client_id,
                                         "scope",
                                         _scope,
                                         "owner_id",
                                         _owner["id"] });
    this->__token_provider->store_token(_token);
    return { "status",
             (_performative == zpt::Post ? 303 : 307),
             "headers",
             { "Location",
               ((_redirect_uri->string() +
                 (_redirect_uri->string().find("?") != std::string::npos ? std::string("&")
                                                                         : std::string("?")) +
                 std::string("code=") + static_cast<std::string>(_token["code"]) +
                 std::string("&state=") + static_cast<std::string>(_state))) } };
}

auto
zpt::auth::oauth2::server::authorize_with_password(zpt::performative _performative,
                                                   zpt::json _request,
                                                   zpt::json _envelope,
                                                   zpt::json _opts) -> zpt::json {
    expect_mandatory(_request, "client_id", 412);
    expect_mandatory(_request, "username", 412);
    expect_mandatory(_request, "password", 412);
    expect_mandatory(_opts, "domain", 412);
    expect_mandatory(_opts, "login_url", 412);

    auto _scope = zpt::split(static_cast<std::string>(_request["scope"]), ",");
    auto _redirect_uri = _request["redirect_uri"];
    auto _client_id = _request["client_id"];
    auto _ownername = _request["username"];
    auto _password = _request["password"];
    auto _state = _request["state"];

    zpt::json _owner;
    try {
        _owner = this->__token_provider->retrieve_owner(
          _ownername->string(), _password->string(), _client_id->string());
    }
    catch (zpt::failed_expectation const& _e) {
        if (_redirect_uri->is_string()) {
            std::string _l_state(
              std::string("response_type=code") + std::string("&scope=") +
              (_scope->ok() ? static_cast<std::string>(_request["scope"]) : "defaults") +
              std::string("&client_id=") + _client_id->string() + std::string("&redirect_uri=") +
              static_cast<std::string>(_redirect_uri) + std::string("&state=") +
              static_cast<std::string>(_state));

            zpt::base64::encode(_l_state);
            zpt::url::encode(_l_state);
            auto _login_url = _opts["login_url"]->string();
            return { "status",
                     (_performative == zpt::Post ? 303 : 307),
                     "headers",
                     { "Set-Cookie",
                       (std::string("deleted; name=oauth_session; domain=") +
                        _opts["domain"]->string() +
                        std::string("; path=/; expires=Thu, Jan 01 1970 00:00:00 UTC; HttpOnly")),
                       "Location",
                       (_login_url + (_login_url.find("?") != std::string::npos ? "&" : "?") +
                        std::string("state=") + _l_state) } };
        }
        throw;
    }

    zpt::json _client;
    try {
        _client = this->__token_provider->retrieve_client(_envelope);
    }
    catch (zpt::failed_expectation const& _e) {
        if (_redirect_uri->is_string()) {
            return { "status",
                     (_performative == zpt::Post ? 303 : 307),
                     "headers",
                     { "Location",
                       (_redirect_uri->string() +
                        (_redirect_uri->string().find("?") != std::string::npos
                           ? std::string("&")
                           : std::string("?")) +
                        std::string("error=true&reason=no+such+client")) } };
        }
        throw;
    }

    auto _token = this->generate_token({ "response_type",
                                         "password",
                                         "client_id",
                                         _client_id,
                                         "scope",
                                         _scope,
                                         "owner_id",
                                         _owner["id"] });
    this->__token_provider->store_token(_token);
    if (_redirect_uri->is_string()) {
        return { "status",
                 (_performative == zpt::Post ? 303 : 307),
                 "headers",
                 { "Set-Cookie",
                   (_token["access_token"]->string() + std::string("; owner=") +
                    _owner["id"]->string() + std::string("; name=oauth_session; domain=") +
                    _opts["domain"]->string() + std::string("; path=/; HttpOnly")),
                   "Location",
                   ((_redirect_uri->string() +
                     (_redirect_uri->string().find("?") != std::string::npos ? "&" : "?") +
                     std::string("access_token=") + _token["access_token"]->string() +
                     std::string("&refresh_token=") + _token["refresh_token"]->string() +
                     std::string("&expires=") + _token["expires"]->string() +
                     std::string("&state=") + static_cast<std::string>(_state))) } };
    }
    else {
        _token //
          ->object()
          ->pop("roles")
          .pop("permissions");
        return { "status", 200, "body", _token };
    }
}

auto
zpt::auth::oauth2::server::authorize_with_client_credentials(zpt::performative _performative,
                                                             zpt::json _request,
                                                             zpt::json _envelope,
                                                             zpt::json _opts) -> zpt::json {
    expect_mandatory(_request, "client_id", 412);
    expect_mandatory(_request, "client_secret", 412);
    expect_mandatory(_opts, "domain", 412);
    expect_mandatory(_opts, "login_url", 412);

    auto _scope = zpt::split(static_cast<std::string>(_request["scope"]), ",");
    auto _redirect_uri = _request["redirect_uri"];
    auto _client_id = _request["client_id"];
    auto _client_secret = _request["client_secret"];
    auto _state = _request["state"];

    zpt::json _client;
    try {
        _client =
          this->__token_provider->retrieve_client(_client_id->string(), _client_secret->string());
    }
    catch (zpt::failed_expectation const& _e) {
        if (_redirect_uri->is_string()) {
            return { "status",
                     (_performative == zpt::Post ? 303 : 307),
                     "headers",
                     { "Location",
                       (_redirect_uri->string() +
                        (_redirect_uri->string().find("?") != std::string::npos
                           ? std::string("&")
                           : std::string("?")) +
                        std::string("error=true&reason=no+such+client")) } };
        }
        throw;
    }

    auto _token = this->generate_token({ "response_type",
                                         "client_credentials",
                                         "client_id",
                                         _client_id,
                                         "client_secret",
                                         _client_secret,
                                         "scope",
                                         _scope });
    this->__token_provider->store_token(_token);
    if (_redirect_uri->is_string()) {
        return { "status",
                 (_performative == zpt::Post ? 303 : 307),
                 "headers",
                 { "Location",
                   ((_redirect_uri->string() +
                     (_redirect_uri->string().find("?") != std::string::npos ? "&" : "?") +
                     std::string("access_token=") + _token["access_token"]->string() +
                     std::string("&refresh_token=") + _token["refresh_token"]->string() +
                     std::string("&expires=") + _token["expires"]->string() +
                     std::string("&state=") + static_cast<std::string>(_state))) } };
    }
    else {
        _token
          ->object() //
          ->pop("roles")
          .pop("permissions");
        return { "status", 200, "body", _token };
    }
}

auto
zpt::auth::oauth2::server::generate_token(zpt::json _data) -> zpt::json {
    auto _client_id = _data["client_id"]->string();
    auto _scope = _data["scope"];
    auto _grant_type = _data["grant_type"]->ok() ? static_cast<std::string>(_data["grant_type"])
                                                 : static_cast<std::string>(_data["response_type"]);
    auto _owner_id = static_cast<std::string>(_data["owner_id"]);
    auto _client_secret = static_cast<std::string>(_data["client_secret"]);
    auto _access_token = zpt::generate::r_key(128);

    zpt::json _token{ "id",
                      _access_token,
                      "access_token",
                      _access_token,
                      "refresh_token",
                      zpt::generate::r_key(64),
                      "code",
                      zpt::generate::r_key(64),
                      "expires",
                      (zpt::timestamp_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                          .count() +
                        90L * 24L * 3600L * 1000L,
                      "scope",
                      _scope->is_string() ? zpt::split(_scope, ",", true) : _scope,
                      "grant_type",
                      _grant_type,
                      "client_id",
                      _client_id,
                      "owner_id",
                      (_owner_id.length() != 0 ? zpt::json::string(_owner_id) : zpt::undefined) };

    return _token + this->__token_provider->get_roles_permissions(_token);
}

auto
zpt::auth::extract(zpt::json _envelope) -> std::string {
    if (_envelope["headers"]["Authorization"]->ok()) {
        return static_cast<std::string>(
          zpt::split(_envelope["headers"]["Authorization"]->string(), " ")[1]);
    }
    if (_envelope["body"]["access_token"]->ok()) {
        auto _param = _envelope["body"]["access_token"]->string();
        zpt::url::decode(_param);
        return _param;
    }
    if (_envelope["params"]["access_token"]->ok()) {
        auto _param = _envelope["params"]["access_token"]->string();
        zpt::url::decode(_param);
        return _param;
    }
    if (_envelope["headers"]["Cookie"]->ok()) {
        return static_cast<std::string>(
          zpt::split(_envelope["headers"]["Cookie"]->string(), ";")[0]);
    }
    return "";
}
