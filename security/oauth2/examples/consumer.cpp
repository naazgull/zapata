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
 * @file consumer.cpp
 * @brief Example consumer of the OAuth2.0 API.
 *
 * This plugin walks through every OAuth2 grant flow in turn against a running
 * OAuth2 server (see provider.cpp + the zapata-security-oauth2 plugin): the
 * authorization code flow (authorize -> code -> token), the resource owner
 * password grant, the client credentials grant, then token validation and
 * refresh, and finally the device-code flow.
 *
 * Once the OAuth2 server is discovered (a REGISTERED_REMOTE_SERVICE system
 * event for the authorize endpoint), oauth2_client_boot fires the first
 * request (the authorization code /authorize) and names oauth2_client as its
 * reply listener via make_call<oauth2_client>. oauth2_client then drives the
 * rest of the walk on its own: it is a single zpt::events::process that keeps
 * its position in a per-instance step counter and chains every subsequent
 * request through its call context (context()), re-triggering on each reply
 * until the flow is complete.
 */

#include <format>
#include <string>
#include <zapata/base.h>
#include <zapata/config.h>
#include <zapata/events.h>
#include <zapata/json.h>
#include <zapata/ontology.h>
#include <zapata/rest.h>
#include <zapata/startup.h>
#include <zapata/transport.h>
#include <zapata/uri.h>

namespace {

/** @brief Resolve an OAuth2 endpoint URL from configuration, falling back to
    the conventional /oauth2/<key> path. */
auto oauth2_url(std::string const& _key) -> std::string {
    auto _config = zpt::GLOBAL_CONFIG();
    return _config("oauth2")("url")(_key)->ok() ? _config("oauth2")("url")(_key)->string()
                                                : std::format("/oauth2/{}", _key);
}

/** @brief Build a POST request to _url with a JSON body. */
auto make_request(std::string const& _url, zpt::json const& _body) -> zpt::message {
    auto _msg = zpt::TRANSPORT_LAYER().get("http")->make_request();
    _msg //
      ->performative(zpt::Post)
      .uri(_url)
      .body() = _body;
    return _msg;
}

} // namespace

/**
 * @brief OAuth2 API consumer. A single process that walks every grant flow in
 * order, chaining each request through its call context.
 *
 * The first request (the authorization code /authorize) is fired by
 * oauth2_client_boot and arrives here as the step-0 reply. Each following
 * step is fired by this object: when the current step's reply is pending the
 * process stays blocked; when the reply is in the context it is processed and
 * the next step is fired.
 *
 * Steps:
 *   0 - authorization code flow: /authorize replied with a redirect carrying
 *       the code -> exchange it at /token.
 *   1 - /token returned the code-flow token -> start the password grant.
 *   2 - password grant returned a token -> start the client credentials grant.
 *   3 - client credentials grant returned a token -> validate it.
 *   4 - token validated -> refresh the code-flow token.
 *   5 - refresh returned a fresh token -> request a device authorization.
 *   6 - device authorization issued (device_code + user_code) -> approve it.
 *   7 - device authorization approved -> poll /token with the device_code.
 *   8 - /token returned the device-grant token -> done.
 */
class oauth2_client : public zpt::events::process {
  public:
    using zpt::events::process::process;
    ~oauth2_client() = default;

    auto blocked() const -> bool {
        return this->__step != 0 && this->context() != nullptr && !this->context()->is_replied();
    }

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
        try {
            if (this->__step == 0) { this->context()->reply(this->received()); }

            if (this->context() != nullptr && this->context()->is_replied()) {
                auto _reply = this->context()->reply();
                switch (this->__step) {
                    case 0: {
                        auto _location = _reply->headers()("Location")->string();
                        zlog("Authorization code redirect: " << _location, zpt::info);
                        this->__code = zpt::uri::parse(_location)("params")("code")->string();
                        break;
                    }
                    case 1: {
                        this->__refresh_token = _reply->body()("refresh_token")->string();
                        zlog("Authorization code token:\n"
                               << zpt::pretty{ _reply->body() },
                             zpt::info);
                        break;
                    }
                    case 2: {
                        zlog("Password grant token:\n" << zpt::pretty{ _reply->body() }, zpt::info);
                        break;
                    }
                    case 3: {
                        this->__token = _reply->body()("access_token")->string();
                        zlog("Client credentials token:\n"
                               << zpt::pretty{ _reply->body() },
                             zpt::info);
                        break;
                    }
                    case 4: {
                        zlog("Token validated:\n" << zpt::pretty{ _reply->body() }, zpt::info);
                        break;
                    }
                    case 5: {
                        zlog("Refreshed token:\n" << zpt::pretty{ _reply->body() }, zpt::info);
                        break;
                    }
                    case 6: {
                        this->__device_code = _reply->body()("device_code")->string();
                        this->__user_code = _reply->body()("user_code")->string();
                        zlog("Device authorization:\n" << zpt::pretty{ _reply->body() }, zpt::info);
                        break;
                    }
                    case 7: {
                        zlog("Device authorization approved:\n"
                               << zpt::pretty{ _reply->body() },
                             zpt::info);
                        break;
                    }
                    case 8: {
                        zlog("Device grant token:\n" << zpt::pretty{ _reply->body() }, zpt::info);
                        this->context(nullptr);
                        return zpt::events::finish;
                    }
                    default: break;
                }
                this->context(nullptr);
                ++this->__step;
                return zpt::events::retrigger;
            }
            if (this->context() == nullptr) {
                zlog("OAuth2 flow walk step: >> " << this->__step << " <<", zpt::debug);

                switch (this->__step) {
                    case 1: {
                        this->exchange_code();
                        break;
                    }
                    case 2: {
                        this->password_grant();
                        break;
                    }
                    case 3: {
                        this->client_credentials();
                        break;
                    }
                    case 4: {
                        this->validate_token();
                        break;
                    }
                    case 5: {
                        this->refresh_token();
                        break;
                    }
                    case 6: {
                        this->device_authorization();
                        break;
                    }
                    case 7: {
                        this->approve_device();
                        break;
                    }
                    case 8: {
                        this->device_token();
                        break;
                    }
                    default: {
                        return zpt::events::abort;
                    }
                }
                return zpt::events::retrigger;
            }
        }
        catch (std::exception const& _e) {
            zlog("OAuth2 flow walk aborted: " << zpt::exception::get_message(_e), zpt::info);
            return zpt::events::abort;
        }
        return zpt::events::finish;
    }

  private:
    /** @brief Exchange the authorization code for a token. */
    auto exchange_code() -> oauth2_client& {
        this->context(zpt::make_call(zpt::REST_RESOLVER(),
                                     make_request(oauth2_url("token"),
                                                  { "client_id",
                                                    "client-1",
                                                    "client_secret",
                                                    "client-secret-1",
                                                    "code",
                                                    this->__code })));
        return (*this);
    }

    /** @brief Request a token via the resource owner password grant. */
    auto password_grant() -> oauth2_client& {
        this->context(zpt::make_call(zpt::REST_RESOLVER(),
                                     make_request(oauth2_url("authorize"),
                                                  { "response_type",
                                                    "password",
                                                    "client_id",
                                                    "client-1",
                                                    "username",
                                                    "admin",
                                                    "password",
                                                    "admin123",
                                                    "scope",
                                                    "read" })));
        return (*this);
    }

    /** @brief Request a token via the client credentials grant. */
    auto client_credentials() -> oauth2_client& {
        this->context(zpt::make_call(zpt::REST_RESOLVER(),
                                     make_request(oauth2_url("authorize"),
                                                  { "response_type",
                                                    "client_credentials",
                                                    "client_id",
                                                    "client-1",
                                                    "client_secret",
                                                    "client-secret-1" })));
        return (*this);
    }

    /** @brief Validate the client credentials access token. */
    auto validate_token() -> oauth2_client& {
        this->context(
          zpt::make_call(zpt::REST_RESOLVER(),
                         make_request(oauth2_url("validate"), { "access_token", this->__token })));
        return (*this);
    }

    /** @brief Refresh the code-flow token. */
    auto refresh_token() -> oauth2_client& {
        this->context(zpt::make_call(
          zpt::REST_RESOLVER(),
          make_request(oauth2_url("refresh"),
                       { "grant_type", "refresh_token", "refresh_token", this->__refresh_token })));
        return (*this);
    }

    /** @brief Request a device authorization. */
    auto device_authorization() -> oauth2_client& {
        this->context(zpt::make_call(zpt::REST_RESOLVER(),
                                     make_request(oauth2_url("device_authorization"),
                                                  { "client_id", "client-1", "scope", "read" })));
        return (*this);
    }

    /** @brief Approve the pending device authorization. */
    auto approve_device() -> oauth2_client& {
        this->context(
          zpt::make_call(zpt::REST_RESOLVER(),
                         make_request(oauth2_url("approve"),
                                      { "user_code", this->__user_code, "approved", "true" })));
        return (*this);
    }

    /** @brief Poll /token with the device code to retrieve the device grant token. */
    auto device_token() -> oauth2_client& {
        this->context(zpt::make_call(zpt::REST_RESOLVER(),
                                     make_request(oauth2_url("token"),
                                                  { "grant_type",
                                                    "device_code",
                                                    "device_code",
                                                    this->__device_code,
                                                    "client_id",
                                                    "client-1",
                                                    "client_secret",
                                                    "client-secret-1" })));
        return (*this);
    }

    /** @brief Current step of the flow walk (per instance). */
    size_t __step{ 0 };
    /** @brief Authorization code extracted from the code-flow redirect. */
    std::string __code{ "" };
    /** @brief Refresh token from the code flow, reused by the refresh step. */
    std::string __refresh_token{ "" };
    /** @brief Access token from the client credentials grant, validated next. */
    std::string __token{ "" };
    /** @brief Device code from the device flow, used to poll /token. */
    std::string __device_code{ "" };
    /** @brief User code from the device flow, used to approve the authorization. */
    std::string __user_code{ "" };
};

/** @brief Kick off the OAuth2 flow walk once the OAuth2 server is reachable. */
class oauth2_client_boot : public zpt::system_event {
  public:
    using zpt::system_event::system_event;
    ~oauth2_client_boot() = default;

    auto operator()(zpt::events::dispatcher::ptr) -> zpt::events::state override {
        auto _id = this->__received->body()("_id")->string();
        if (_id.find("authorize") != std::string::npos) {
            zlog("OAuth2 server discovered; starting flow walk", zpt::info);
            zpt::make_call<oauth2_client>(zpt::REST_RESOLVER(),
                                          make_request(oauth2_url("authorize"),
                                                       { "response_type",
                                                         "code",
                                                         "client_id",
                                                         "client-1",
                                                         "redirect_uri",
                                                         "http://localhost:8080/callback",
                                                         "scope",
                                                         "read" }));
            zpt::SYSTEM_EVENTS_RESOLVER() //
              ->remove<oauth2_client_boot>(zpt::system_event_type::REGISTERED_REMOTE_SERVICE);
        }
        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading OAuth2 example consumer", zpt::info);
    zpt::SYSTEM_EVENTS_RESOLVER() //
      ->add<oauth2_client_boot>(zpt::system_event_type::REGISTERED_REMOTE_SERVICE);
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloading OAuth2 example consumer", zpt::info);
    zpt::SYSTEM_EVENTS_RESOLVER() //
      ->remove<oauth2_client_boot>(zpt::system_event_type::REGISTERED_REMOTE_SERVICE);
}
