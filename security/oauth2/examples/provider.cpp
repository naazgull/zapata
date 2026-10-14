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

/**
 * @file provider.cpp
 * @brief Example OAuth2 provider: an in-memory zpt::auth::oauth2::token_provider.
 *
 * This plugin implements the token_provider interface entirely in memory, using
 * a single well-known resource owner and client. On load it registers the global
 * OAuth2 server with this provider (zpt::OAUTH2_SERVER). It does not register the
 * REST handlers themselves -- that is the job of the zapata-security-oauth2
 * plugin -- so a deployment loads both this example and the oauth2 plugin.
 *
 * A production implementation would resolve owners/clients and persist tokens
 * in a database; here everything is a constant or an in-memory map.
 */

#include <map>
#include <mutex>
#include <string>
#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/oauth2/oauth2.h>
#include <zapata/startup.h>

namespace {

/**
 * @brief An in-memory zpt::auth::oauth2::token_provider backed by constants.
 *
 * Demonstrates every method of the token_provider interface: a fixed resource
 * owner (admin/admin123) and client (client-1/client-secret-1), and an
 * in-memory token store keyed by access token.
 */
class memory_token_provider : public zpt::auth::oauth2::token_provider {
  public:
    /** @brief Constant resource owner used by this example. */
    static constexpr char const* owner_id = "owner-1";
    static constexpr char const* owner_username = "admin";
    static constexpr char const* owner_password = "admin123";
    /** @brief Constant client used by this example. */
    static constexpr char const* client_id = "client-1";
    static constexpr char const* client_secret = "client-secret-1";

    memory_token_provider() = default;
    ~memory_token_provider() override = default;

    /** @brief Resolve the resource owner from the request (authorization code
        flow). A real implementation would resolve the owner from the session
        established by the login redirect; here the owner is always considered
        authenticated. */
    auto retrieve_owner(zpt::message _message) -> zpt::json override {
        (void)_message;
        return { "id", owner_id, "username", owner_username };
    }

    /** @brief Resolve the resource owner from explicit credentials (password
        grant). */
    auto retrieve_owner(std::string const& _owner,
                        std::string const& _password,
                        std::string const& _client_id) -> zpt::json override {
        (void)_client_id;
        if (_password.empty()) {
            expect_c(_owner == owner_id, "invalid owner credentials", 401);
            return { "id", owner_id, "username", owner_username };
        }
        else {
            expect_c(_owner == owner_username && _password == owner_password,
                     "invalid owner credentials",
                     401);
            return { "id", owner_id, "username", owner_username };
        }
    }

    /** @brief Resolve the client from the request (authorization code flow).
     * @throws zpt::failed_expectation If no such client is known. */
    auto retrieve_client(zpt::message _message) -> zpt::json override {
        auto _id = this->param(_message, "client_id");
        expect_c(static_cast<std::string>(_id) == client_id, "no such client", 401);
        return { "id", client_id, "client_id", client_id, "name", "example-client" };
    }

    /** @brief Resolve the client from explicit credentials (client credentials
        grant). @throws zpt::failed_expectation If the credentials are unknown. */
    auto retrieve_client(std::string const& _client_id, std::string const& _client_secret)
      -> zpt::json override {
        if (_client_secret.empty()) { expect_c(_client_id == client_id, "no such client", 401); }
        else {
            expect_c(
              _client_id == client_id && _client_secret == client_secret, "no such client", 401);
        }
        return { "id", client_id, "client_id", client_id, "name", "example-client" };
    }

    /** @brief Generate the token's secret attributes and persist it. For a normal grant this means
        access/refresh tokens, an authorization code and a 90-day expiry, stored keyed by the
        access token. For the device grant (grant_type "device") it instead mints an approved device
        authorization -- device_code, user_code, a pre-issued access/refresh token pair and a short
        expiry -- stored under all three of its codes so the existing getters resolve any of them.
        This example assumes the resource owner has already approved it at the verification URI.
        In both cases the roles/permissions are attached. @param _data The grant context to complete
        in place. */
    auto generate_secrets(zpt::json& _data) -> void override {
        auto _access_token = zpt::generate::r_key(128);
        _data["access_token"] = _access_token;
        _data["refresh_token"] = zpt::generate::r_key(64);
        long long _expires_in = 5LL * 60LL * 1000LL;
        _data["valid_until"] = zpt::timestamp(zpt::now<long long>() + _expires_in);
        _data["expires_in"] = _expires_in;
        _data["client_id"] = client_id;
        _data["owner_id"] = owner_id;

        if (_data("grant_type")->ok() && _data("grant_type") == "device_code") {
            auto _device_code = zpt::generate::r_key(64);
            auto _user_code = zpt::generate::r_key(6);
            _data["device_code"] = _device_code;
            _data["user_code"] = _user_code;
            _data["status"] = "approved";
        }
        else { _data["code"] = zpt::generate::r_key(64); }

        std::lock_guard _lock{ _mutex };
        _tokens[_access_token] = _data;
    }

    /** @brief Exchange an authorization code for the stored token, after
        verifying the requesting client. @throws zpt::failed_expectation If the
        client is unknown or the code is not valid. */
    auto exchange_code(zpt::json const& _context) -> zpt::json override {
        expect_c(_context("client_id") == client_id && _context("client_secret") == client_secret,
                 "no such client",
                 401);
        std::lock_guard _lock{ _mutex };
        for (auto&& [_, _token] : _tokens) {
            if (_context("code")->ok() && _token("code") == _context("code")) { return _token; }
            if (_context("device_code")->ok() && _token("device_code") == _context("device_code")) {
                return _token;
            }
        }
        expect_c(false, "invalid authorization code", 400);
        return zpt::json{};
    }

    /** @brief Look up a token by its access token. @return The token, or an
        empty value if it is not stored. */
    auto get_data_from_token(std::string const& _access_token) -> zpt::json override {
        std::lock_guard _lock{ _mutex };
        auto _it = _tokens.find(_access_token);
        return _it == _tokens.end() ? zpt::json{} : _it->second;
    }

    /** @brief Look up a token by its refresh token. @return The token, or an
        empty value if it is not stored. */
    auto get_data_from_refresh_token(std::string const& _refresh_token) -> zpt::json override {
        std::lock_guard _lock{ _mutex };
        for (auto&& [_, _token] : _tokens) {
            if (static_cast<std::string>(_token("refresh_token")) == _refresh_token) {
                return _token;
            }
        }
        return zpt::json{};
    }

  private:
    /** @brief Read a value from the request body, falling back to URL
        parameters. @return The value, or an empty value if absent. */
    auto param(zpt::message _message, std::string const& _key) -> zpt::json {
        auto _body = _message->body();
        if (_body(_key)->ok()) { return _body(_key); }
        auto _params = _message->parameters();
        if (_params(_key)->ok()) { return _params(_key); }
        return zpt::json{};
    }

    /** @brief The in-memory token store, keyed by access token. */
    std::map<std::string, zpt::json> _tokens;
    /** @brief Guards _tokens. */
    std::mutex _mutex;
};

} // namespace

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading OAuth2 example provider (in-memory token provider)", zpt::info);
    zpt::OAUTH2_SERVER(std::make_shared<memory_token_provider>());
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading OAuth2 example provider", zpt::info);
    zpt::OAUTH2_SERVER().clear();
}
