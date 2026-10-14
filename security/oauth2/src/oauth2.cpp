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

#include <vector>
#include <zapata/config.h>
#include <zapata/http.h>
#include <zapata/oauth2/oauth2.h>
#include <zapata/uri.h>

namespace {
/** @brief Build a reply message from a request message.
 * @param _request The incoming request message the reply is based on.
 * @param _status The HTTP status to set on the reply.
 * @param _location The Location header value (empty for non-redirect replies).
 * @param _body The JSON body (used when _location is empty).
 * @return The reply message. */
auto make_reply(zpt::message _request,
                zpt::status _status,
                std::string _location = "",
                zpt::json _body = json_null) -> zpt::message;
/** @brief Compute a PKCE S256 code challenge: the base64url-encoded (no padding) SHA-256 digest
 * of the verifier (RFC 7636). @param _verifier The PKCE code verifier. @return The S256 challenge
 * string. */
auto pkce_s256(std::string const& _verifier) -> std::string;
} // namespace

zpt::auth::oauth2::server::server(zpt::auth::oauth2::token_provider::ptr _token_provider)
  : __token_provider{ _token_provider } {
    expect(this->__token_provider != nullptr, "a valid token provider must be provided");
}

zpt::auth::oauth2::server::~server() {}

auto zpt::auth::oauth2::server::clear() -> server& {
    this->__token_provider.reset();
    return (*this);
}

auto zpt::auth::oauth2::server::name() -> std::string { return "oauth2.0"; }

auto zpt::auth::oauth2::server::authorize(zpt::message _request) -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();

    expect_c(_param("response_type")->ok(), "Required parameter 'response_type'", 412);
    auto _response_type = _param("response_type")->string();

    if (_response_type == "code") { return this->authorize_with_code(_request); }
    else if (_response_type == "password") { return this->authorize_with_password(_request); }
    else if (_response_type == "implicit") { return ::make_reply(_request, 400); }
    else if (_response_type == "client_credentials") {
        return this->authorize_with_client_credentials(_request);
    }
    expect_c(false, "\"response_type\" not valid", 400);
}

auto zpt::auth::oauth2::server::authorize_device(zpt::message _request) -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();

    expect_c(_param("client_id")->ok(), "Required parameter 'client_id'", 412);
    auto _client_id = _param("client_id");
    auto _scope = _param("scope")->ok() ? zpt::split(_param("scope")->string(), ",") : zpt::json{};

    zpt::json _client;
    try {
        _client = this->__token_provider->retrieve_client(_request);
    }
    catch (zpt::failed_expectation const& _e) {
        return ::make_reply(
          _request, 400, "", { "error", "unauthorized_client", "error_description", _e.what() });
    }

    auto _context = zpt::json{ "grant_type", "device_code", "client", _client, "scope", _scope };

    auto _token = this->generate_token(_context);

    return ::make_reply(_request,
                        200,
                        "",
                        { "device_code",
                          _token("device_code"),
                          "user_code",
                          _token("user_code"),
                          "verification_uri",
                          zpt::GLOBAL_CONFIG()("oauth2")("url")("verify_device")->string(),
                          "expires_in",
                          _token("expires_in"),
                          "valid_until",
                          _token("valid_until"),
                          "interval",
                          5 });
}

auto zpt::auth::oauth2::server::token(zpt::message _request) -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();

    expect_c(_param("grant_type")->ok(), "Required parameter 'grant_type'", 412);
    expect_c(_param("client_id")->ok(), "Required parameter 'client_id'", 412);

    auto _grant_type = _param("grant_type");
    if (_grant_type == "device_code") {
        expect_c(_param("device_code")->ok(), "Required parameter 'device_code'", 412);
        return this->token_device(_request);
    }

    expect_c(_param("code")->ok(), "Required parameter 'code'", 412);
    expect_c(_param("client_secret")->ok(), "Required parameter 'client_secret'", 412);

    auto _redirect_uri = _param("redirect_uri");
    auto _token = this->__token_provider->exchange_code(_param);

    if (_token("code_challenge")->ok()) {
        expect_c(_param("code_verifier")->ok(), "Required parameter 'code_verifier'", 400);
        auto _challenge = _token("code_challenge")->string();
        auto _method = _token("code_challenge_method")->ok()
                         ? _token("code_challenge_method")->string()
                         : std::string("S256");
        auto _verifier = _param("code_verifier")->string();
        auto _valid =
          (_method == "plain") ? (_challenge == _verifier) : (::pkce_s256(_verifier) == _challenge);
        expect_c(_valid, "invalid code_verifier", 400);
    }

    if (_redirect_uri->is_string()) {
        auto _status = _request->performative() == zpt::Post ? 303 : 307;
        return ::make_reply(_request,
                            _status,
                            std::format("{}{}",
                                        _redirect_uri->string(),
                                        zpt::uri::params::to_string(
                                          { "params",
                                            { "access_token",
                                              _token("access_token"),
                                              "token_type",
                                              "Bearer",
                                              "refresh_token",
                                              _token("refresh_token"),
                                              "valid_until",
                                              _token("valid_until"),
                                              "expires_in",
                                              _token("expires_in") } },
                                          _redirect_uri->string().find("?") != std::string::npos)));
    }
    else {
        return ::make_reply(_request,
                            200,
                            "",
                            { "access_token",
                              _token("access_token"),
                              "token_type",
                              "Bearer",
                              "refresh_token",
                              _token("refresh_token"),
                              "valid_until",
                              _token("valid_until"),
                              "expires_in",
                              _token("expires_in") });
    }
}

auto zpt::auth::oauth2::server::refresh(zpt::message _request) -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();

    expect_c(_param("grant_type")->ok(), "Required parameter 'grant_type'", 412);
    expect_c(_param("refresh_token")->ok(), "Required parameter 'refresh_token'", 412);

    auto _redirect_uri = _param("redirect_uri");
    auto _refresh_token =
      this->__token_provider->get_data_from_refresh_token(_param("refresh_token")->string());
    expect_c(_refresh_token("client_id")->ok() && _refresh_token("owner_id")->ok(),
             "Invalid refresh_token",
             412);

    zpt::json _owner;
    try {
        _owner = this->__token_provider->retrieve_owner(
          _refresh_token("owner_id")->string(), "", _refresh_token("client_id")->string());
    }
    catch (zpt::failed_expectation const& _e) {
        return ::make_reply(
          _request, 400, "", { "error", "invalid_request", "error_description", _e.what() });
    }

    zpt::json _client;
    try {
        _client =
          this->__token_provider->retrieve_client(_refresh_token("client_id")->string(), "");
    }
    catch (zpt::failed_expectation const& _e) {
        return ::make_reply(
          _request, 400, "", { "error", "unauthorized_client", "error_description", _e.what() });
    }

    auto _context = zpt::json{ "grant_type",
                               "refresh_token",
                               "client",
                               _client,
                               "scope",
                               _refresh_token("scope"),
                               "roles",
                               _refresh_token("roles"),
                               "owner",
                               _owner,
                               "original_grant_type",
                               _refresh_token("grant_type") };

    auto _token = this->generate_token(_context);

    if (_redirect_uri->is_string()) {
        auto _status = _request->performative() == zpt::Post ? 303 : 307;
        return ::make_reply(_request,
                            _status,
                            std::format("{}{}",
                                        _redirect_uri->string(),
                                        zpt::uri::params::to_string(
                                          { "params",
                                            { "access_token",
                                              _token("access_token"),
                                              "token_type",
                                              "Bearer",
                                              "refresh_token",
                                              _token("refresh_token"),
                                              "valid_until",
                                              _token("valid_until"),
                                              "expires_in",
                                              _token("expires_in") } },
                                          _redirect_uri->string().find("?") != std::string::npos)));
    }
    else {
        return ::make_reply(_request,
                            200,
                            "",
                            { "access_token",
                              _token("access_token"),
                              "token_type",
                              "Bearer",
                              "refresh_token",
                              _token("refresh_token"),
                              "valid_until",
                              _token("valid_until"),
                              "expires_in",
                              _token("expires_in") });
    }
}

auto zpt::auth::oauth2::server::validate(zpt::message _request) -> zpt::message {
    auto _access_token = zpt::auth::extract_token(_request);
    auto _token = this->__token_provider->get_data_from_token(_access_token);
    expect_c(_token->ok(), "token is invalid", 403);
    auto _now = zpt::timestamp();
    auto _expires = zpt::timestamp(_token("valid_until")->string());
    expect_c(_expires > _now, "token has expired", 403);
    return ::make_reply(
      _request, 200, "", { "token_type", "Bearer", "valid_until", _token("valid_until") });
}

auto zpt::auth::oauth2::server::authorize_with_code(zpt::message _request) -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();
    auto _status = _request->performative() == zpt::Post ? 303 : 307;

    expect_c(_param("client_id")->ok(), "Required parameter 'client_id'", 412);
    expect_c(_param("redirect_uri")->ok(), "Required parameter 'redirect_uri'", 412);

    auto _scope = zpt::split(static_cast<std::string>(_param("scope")), ",");
    auto _redirect_uri = _param("redirect_uri");
    auto _client_id = _param("client_id");
    auto _state = _param("state");

    zpt::json _owner;
    try {
        _owner = this->__token_provider->retrieve_owner(_request);
    }
    catch (zpt::failed_expectation const& _e) {
        std::string _l_state =
          std::format("response_type=code&scope={}&client_id={}&redirect_uri={}&state={}",
                      (_scope->ok() ? static_cast<std::string>(_param("scope")) : "defaults"),
                      _client_id->string(),
                      _redirect_uri->string(),
                      static_cast<std::string>(_state));
        zpt::base64::encode(_l_state);
        zpt::url::encode(_l_state);
        auto _login_url = zpt::GLOBAL_CONFIG()("oauth2")("url")("login")->string();
        auto _reply = ::make_reply(
          _request,
          _status,
          std::format("{}{}",
                      _login_url,
                      zpt::uri::params::to_string({ "params", { "state", _l_state } },
                                                  _login_url.find("?") != std::string::npos)));
        return _reply;
    }

    zpt::json _client;
    try {
        _client = this->__token_provider->retrieve_client(_request);
    }
    catch (zpt::failed_expectation const& _e) {
        return ::make_reply(_request,
                            _status,
                            std::format("{}{}",
                                        _redirect_uri->string(),
                                        zpt::uri::params::to_string(
                                          { "params",
                                            { "error",
                                              "unauthorized_client",
                                              "error_description",
                                              zpt::url::r_encode(_e.what()) } },
                                          _redirect_uri->string().find("?") != std::string::npos)));
    }

    auto _context =
      zpt::json{ "grant_type", "code", "client", _client, "scope", _scope, "owner", _owner };

    if (_param("code_challenge")->ok()) {
        _context["code_challenge"] = _param("code_challenge");
        _context["code_challenge_method"] = _param("code_challenge_method")->ok()
                                              ? _param("code_challenge_method")
                                              : zpt::json::string("S256");
    }

    auto _token = this->generate_token(_context);
    return ::make_reply(_request,
                        _status,
                        std::format("{}{}",
                                    _redirect_uri->string(),
                                    zpt::uri::params::to_string(
                                      { "params", { "code", _token("code"), "state", _state } },
                                      _redirect_uri->string().find("?") != std::string::npos)));
}

auto zpt::auth::oauth2::server::authorize_with_password(zpt::message _request) -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();
    auto _status = _request->performative() == zpt::Post ? 303 : 307;

    expect_c(_param("client_id")->ok(), "Required parameter 'client_id'", 412);
    expect_c(_param("username")->ok(), "Required parameter 'username'", 412);
    expect_c(_param("password")->ok(), "Required parameter 'password'", 412);

    auto _scope = zpt::split(static_cast<std::string>(_param("scope")), ",");
    auto _redirect_uri = _param("redirect_uri");
    auto _client_id = _param("client_id");
    auto _ownername = _param("username");
    auto _password = _param("password");
    auto _state = _param("state");

    zpt::json _owner;
    try {
        _owner = this->__token_provider->retrieve_owner(
          _ownername->string(), _password->string(), _client_id->string());
    }
    catch (zpt::failed_expectation const& _e) {
        if (_redirect_uri->is_string()) {
            std::string _l_state =
              std::format("response_type=code&scope={}&client_id={}&redirect_uri={}&state={}",
                          (_scope->ok() ? static_cast<std::string>(_param("scope")) : "defaults"),
                          _client_id->string(),
                          _redirect_uri->string(),
                          static_cast<std::string>(_state));
            zpt::base64::encode(_l_state);
            zpt::url::encode(_l_state);
            auto _login_url = zpt::GLOBAL_CONFIG()("oauth2")("url")("login")->string();
            return ::make_reply(
              _request,
              _status,
              std::format("{}{}",
                          _login_url,
                          zpt::uri::params::to_string({ "params", { "state", _l_state } },
                                                      _login_url.find("?") != std::string::npos)));
        }
        throw;
    }

    zpt::json _client;
    try {
        _client = this->__token_provider->retrieve_client(_request);
    }
    catch (zpt::failed_expectation const& _e) {
        if (_redirect_uri->is_string()) {
            return ::make_reply(
              _request,
              _status,
              std::format("{}{}",
                          _redirect_uri->string(),
                          zpt::uri::params::to_string({ "params",
                                                        { "error",
                                                          "unauthorized_client",
                                                          "error_description",
                                                          zpt::url::r_encode(_e.what()) } },
                                                      _redirect_uri->string().find("?") !=
                                                        std::string::npos)));
        }
        throw;
    }

    auto _token = this->generate_token(
      { "grant_type", "password", "client", _client, "scope", _scope, "owner", _owner });

    if (_redirect_uri->is_string()) {
        return ::make_reply(_request,
                            _status,
                            std::format("{}{}",
                                        _redirect_uri->string(),
                                        zpt::uri::params::to_string(
                                          { "params",
                                            { "access_token",
                                              _token("access_token"),
                                              "token_type",
                                              "Bearer",
                                              "refresh_token",
                                              _token("refresh_token"),
                                              "valid_until",
                                              _token("valid_until"),
                                              "expires_in",
                                              _token("expires_in"),
                                              "state",
                                              _state } },
                                          _redirect_uri->string().find("?") != std::string::npos)));
    }
    else {
        return ::make_reply(_request,
                            200,
                            "",
                            { "access_token",
                              _token("access_token"),
                              "token_type",
                              "Bearer",
                              "refresh_token",
                              _token("refresh_token"),
                              "valid_until",
                              _token("valid_until"),
                              "expires_in",
                              _token("expires_in") });
    }
}

auto zpt::auth::oauth2::server::authorize_with_client_credentials(zpt::message _request)
  -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();
    auto _status = _request->performative() == zpt::Post ? 303 : 307;

    expect_c(_param("client_id")->ok(), "Required parameter 'client_id'", 412);
    expect_c(_param("client_secret")->ok(), "Required parameter 'client_secret'", 412);

    auto _scope = zpt::split(static_cast<std::string>(_param("scope")), ",");
    auto _redirect_uri = _param("redirect_uri");
    auto _client_id = _param("client_id");
    auto _client_secret = _param("client_secret");
    auto _state = _param("state");

    zpt::json _client;
    try {
        _client =
          this->__token_provider->retrieve_client(_client_id->string(), _client_secret->string());
    }
    catch (zpt::failed_expectation const& _e) {
        if (_redirect_uri->is_string()) {
            return ::make_reply(
              _request,
              _status,
              std::format("{}{}",
                          _redirect_uri->string(),
                          zpt::uri::params::to_string({ "params",
                                                        { "error",
                                                          "unauthorized_client",
                                                          "error_description",
                                                          zpt::url::r_encode(_e.what()) } },
                                                      _redirect_uri->string().find("?") !=
                                                        std::string::npos)));
        }
        throw;
    }

    auto _token = this->generate_token(
      { "grant_type", "client_credentials", "client", _client, "scope", _scope });

    if (_redirect_uri->is_string()) {
        return ::make_reply(_request,
                            _status,
                            std::format("{}{}",
                                        _redirect_uri->string(),
                                        zpt::uri::params::to_string(
                                          { "params",
                                            { "access_token",
                                              _token("access_token"),
                                              "token_type",
                                              "Bearer",
                                              "refresh_token",
                                              _token("refresh_token"),
                                              "valid_until",
                                              _token("valid_until"),
                                              "expires_in",
                                              _token("expires_in"),
                                              "state",
                                              _state } },
                                          _redirect_uri->string().find("?") != std::string::npos)));
    }
    else {
        return ::make_reply(_request,
                            200,
                            "",
                            { "access_token",
                              _token("access_token"),
                              "token_type",
                              "Bearer",
                              "refresh_token",
                              _token("refresh_token"),
                              "valid_until",
                              _token("valid_until"),
                              "expires_in",
                              _token("expires_in") });
    }
}

auto zpt::auth::oauth2::server::token_device(zpt::message _request) -> zpt::message {
    auto _param = _request->performative() == zpt::Post ? _request->body() : _request->parameters();

    expect_c(_param("device_code")->ok(), "Required parameter 'device_code'", 412);
    expect_c(_param("client_id")->ok(), "Required parameter 'client_id'", 412);
    auto _token = this->__token_provider->exchange_code(_param);

    auto _status = _token("status")->string();
    if (_status == "pending") {
        return ::make_reply(_request, 400, "", { "error", "authorization_pending" });
    }
    if (_status == "denied") {
        return ::make_reply(_request, 400, "", { "error", "access_denied" });
    }
    auto _expires = zpt::timestamp(_token("valid_until")->string());
    if (_expires <= zpt::timestamp()) {
        return ::make_reply(_request, 400, "", { "error", "expired_token" });
    }
    if (_status != "approved") {
        return ::make_reply(_request, 400, "", { "error", "invalid_grant" });
    }

    return ::make_reply(_request,
                        200,
                        "",
                        { "access_token",
                          _token("access_token"),
                          "token_type",
                          "Bearer",
                          "refresh_token",
                          _token("refresh_token"),
                          "valid_until",
                          _token("valid_until"),
                          "expires_in",
                          _token("expires_in") });
}

auto zpt::auth::oauth2::server::generate_token(zpt::json _data) -> zpt::json {
    expect(_data("grant_type")->ok(),
           "A `grant_type` must be provided in order to generate a token");

    auto _scope = _data("scope");
    if (_scope->is_string()) { _data["scope"] = zpt::split(_scope, ",", true); }

    this->__token_provider->generate_secrets(_data);
    return _data;
}

auto zpt::auth::extract_token(zpt::message _message) -> std::string {
    expect(_message->headers()("Authorization")->ok(), "No provided access token");
    return zpt::split(_message->headers()("Authorization")->string(), " ")(1)->string();
}

auto zpt::OAUTH2_SERVER(zpt::auth::oauth2::token_provider::ptr _token_provider)
  -> zpt::auth::oauth2::server& {
    static zpt::auth::oauth2::server _global{ _token_provider };
    return _global;
}

namespace {
auto make_reply(zpt::message _request, zpt::status _status, std::string _location, zpt::json _body)
  -> zpt::message {
    auto _reply = zpt::TRANSPORT_LAYER().get("http")->make_reply(_request);
    _reply //
      ->status(_status)
      .body() = _body;
    if (!_location.empty()) { _reply->header("Location", _location); }
    return _reply;
}

auto pkce_s256(std::string const& _verifier) -> std::string {
    unsigned char _digest[zpt::crypto::SHA256::DIGEST_SIZE];
    zpt::crypto::SHA256 _hash;
    _hash.init();
    _hash.update(reinterpret_cast<const unsigned char*>(_verifier.data()),
                 static_cast<unsigned int>(_verifier.size()));
    _hash.finalize(_digest);
    std::vector<unsigned char> _bytes(_digest, _digest + zpt::crypto::SHA256::DIGEST_SIZE);
    std::string _challenge;
    zpt::base64::url_encode(_bytes, _challenge, false);
    return _challenge;
}
} // namespace
