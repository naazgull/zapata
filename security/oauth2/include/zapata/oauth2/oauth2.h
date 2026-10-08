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

#pragma once

#include <memory>
#include <string>
#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/ontology/message.h>

namespace zpt {
namespace auth {
/** @brief Extract an access token from a request message. Checks the Authorization header
    first, then the request body, then URL params. Returns an empty string if no token is
    found. */
auto extract(zpt::message _message) -> std::string;

namespace oauth2 {
/**
    @name Token Provider Interface
    @brief Abstract interface for OAuth2 token storage and validation. Implement this class
    to provide custom token persistence backed by a database, cache, or other store.
   @{
*/
class token_provider {
  public:
    using ptr = std::shared_ptr<token_provider>;

    /** @brief Default constructor. */
    token_provider() = default;
    /** @brief Virtual destructor. */
    virtual ~token_provider() = default;

    /** @brief Retrieve owner information from the request message. Used during the authorization
        code flow to authenticate the resource owner. @param _message The request message
        containing authentication data. @return JSON containing the owner's identity. */
    virtual auto retrieve_owner(zpt::message _message) -> zpt::json = 0;
    /** @brief Retrieve owner information using explicit credentials. Used during the password
        grant flow. @param _owner The username or owner identifier. @param _password The owner's
        password. @param _client_id The requesting client's identifier. @return JSON containing
        the owner's identity. */
    virtual auto retrieve_owner(std::string const& _owner,
                                std::string const& _password,
                                std::string const& _client_id) -> zpt::json = 0;
    /** @brief Retrieve client information from the request message. Used to validate the
        requesting application during authorization flows. @param _message The request message
        containing client credentials. @return JSON containing the client's identity. */
    virtual auto retrieve_client(zpt::message _message) -> zpt::json = 0;
    /** @brief Retrieve client information using explicit credentials. Used during the client
        credentials grant flow. @param _client_id The client's identifier. @param _client_secret
        The client's secret. @return JSON containing the client's identity. */
    virtual auto retrieve_client(std::string const& _client_id, std::string const& _client_secret)
      -> zpt::json = 0;
    /** @brief Generate and persist the secret attributes of a token. Invoked from
        server::generate_token once the grant context has been built. This method instantiates,
        within _data, whatever secret attributes (access token, refresh token, code, expiration,
        ...) are required for the grant_type present in _data together with any other additional
        attributes it finds necessary, enriches the token with the appropriate roles and
        permissions, and stores it if that is the provider's logic. @param _data The grant context
        JSON to complete in place. @return void. */
    virtual auto generate_secrets(zpt::json& _data) -> void = 0;
    /** @brief Exchange an authorization code for token data. @param _code The authorization
        code received from the client. @param _id The client identifier. @param _secret The
        client secret. @return JSON containing the access and refresh tokens. */
    virtual auto exchange_code(std::string const& _code,
                               std::string const& _id,
                               std::string const& _secret) -> zpt::json = 0;
    /** @brief Look up token data by access token string. @param _access_token The access token
        value. @return JSON containing the token data. */
    virtual auto get_data_from_token(std::string const& _access_token) -> zpt::json = 0;
    /** @brief Look up token data by refresh token string. @param _refresh_token The refresh
        token value. @return JSON containing the token data. */
    virtual auto get_data_from_refresh_token(std::string const& _refresh_token) -> zpt::json = 0;
    /** @brief Retrieve roles and permissions associated with a token. Called during token
        generation to enrich the token with authorization metadata. @param _token The token JSON.
        @return JSON containing roles and permissions. */
    virtual auto get_roles_permissions(zpt::json _token) -> zpt::json = 0;
    /** @brief Validate that a token's roles/permissions satisfy the requirements for a given
        topic. @param _message The request message. @param _topic The topic being accessed.
        @param _permissions The permissions required. @return true if the token has sufficient
        permissions. */
    virtual auto validate_roles_permissions(zpt::message _message,
                                            std::string _topic,
                                            zpt::json _permissions) -> bool = 0;
    /** @brief Approve or deny a pending device authorization (RFC 8628 device grant). Resolves the
        authorization by its device_code or user_code, flips its status to approved or denied, and
        persists the change. @param _code The device_code or user_code identifying the device
        authorization. @param _approved true to approve, false to deny. @return void. */
    virtual auto approve(std::string const& _code, bool _approved) -> void = 0;
};
/**@}
 */

/** @name OAuth2 Server
    @brief Central OAuth2 authorization server. Handles all OAuth2 grant flows (authorization
    code, password, client credentials) and provides token validation.
   @{
*/
class server {
  public:
    /** @brief Construct an OAuth2 server.
        @param _token_provider The token provider implementation for storage and lookup.
     */
    server(zpt::auth::oauth2::token_provider::ptr _token_provider);
    /** @brief Virtual destructor.
     * @return void (destructors implicitly clean up the object). */
    virtual ~server();

    virtual auto clear() -> server&;
    /** @brief Return the server's name. @return The string "oauth2.0". */
    virtual auto name() -> std::string;
    /** @brief Authorize a request based on the grant type. Dispatches to the appropriate
        grant handler (authorization code, password, client credentials).
        @param _request The incoming request message.
        @return Reply message with a redirect URL or error body. */
    virtual auto authorize(zpt::message _request) -> zpt::message;
    /** @brief Authorize a request by validating the access token against required roles.
        Used for internal authorization checks.
        @param _topic The topic being accessed.
        @param _request The incoming request message.
        @param _roles_needed The roles required to access the topic.
        @return Reply message with the token's identity data if authorized, or an error. */
    virtual auto authorize(std::string const& _topic,
                           zpt::message _request,
                           zpt::json _roles_needed) -> zpt::message;
    /** @brief Exchange an authorization code for access and refresh tokens.
        @param _request The incoming request message.
        @return Reply message with a redirect or the token data. */
    virtual auto token(zpt::message _request) -> zpt::message;
    /** @brief Exchange a refresh token for a new pair of access and refresh tokens.
        The old refresh token is revoked.
        @param _request The incoming request message.
        @return Reply message with a redirect or the new token data. */
    virtual auto refresh(zpt::message _request) -> zpt::message;
    /** @brief Validate an access token. Checks that the token is valid and has not expired.
        @param _request The incoming request message.
        @return Reply message with the token data if valid, or an error. */
    virtual auto validate(zpt::message _request) -> zpt::message;
    /** @brief Request a device authorization (RFC 8628 device grant). Generates a device_code and
        a user_code that the resource owner uses to approve the request offline.
        @param _request The incoming request message (client_id, scope).
        @return Reply message with the device_code, user_code, verification_uri, expiry and
        polling interval. */
    virtual auto device_authorization(zpt::message _request) -> zpt::message;
    /** @brief Approve or deny a device authorization using its user_code.
        @param _request The incoming request message (user_code, approved).
        @return Reply message carrying the resulting status. */
    virtual auto approve(zpt::message _request) -> zpt::message;

    /**@}
     */

  private:
    /** @brief The token provider for storage and lookup operations. */
    zpt::auth::oauth2::token_provider::ptr __token_provider{ nullptr };

    /**
     * @brief Handle the authorization code grant flow. Prompts the owner to authenticate
     * if needed, then generates a code token.
     * @param _request The incoming request message.
     * @return Reply message with a redirect URL or authorization code.
     */
    auto authorize_with_code(zpt::message _request) -> zpt::message;
    /**
     * @brief Handle the password grant flow. Authenticates the owner with username and
     * password, then generates an access token.
     * @param _request The incoming request message.
     * @return Reply message with a redirect URL or access token data.
     */
    auto authorize_with_password(zpt::message _request) -> zpt::message;
    /**
     * @brief Handle the client credentials grant flow. Authenticates the client with
     * its credentials and generates an access token.
     * @param _request The incoming request message.
     * @return Reply message with a redirect URL or access token data.
     */
    auto authorize_with_client_credentials(zpt::message _request) -> zpt::message;
    /**
     * @brief Handle the device code grant at the token endpoint (RFC 8628). Resolves the pending
     * device authorization by device_code and returns the access token once approved.
     * @param _request The incoming request message (device_code).
     * @return Reply message with the token, or a device-flow error (pending/denied/expired).
     */
    auto token_device(zpt::message _request) -> zpt::message;
    /**
     * @brief Generate a new token. Builds the grant context, then delegates to the token
     * provider's generate_secrets to instantiate the secret attributes, roles and permissions,
     * and persist the token.
     * @param _data Grant context payload.
     * @return JSON containing the generated token(s) and metadata.
     */
    auto generate_token(zpt::json _data) -> zpt::json;
};
} // namespace oauth2
} // namespace auth

/** @brief Global accessor for the OAuth2 server instance. Used by all handlers to dispatch
    requests to the server.
 * @return Reference to the OAuth2 server instance variable. */
auto OAUTH2_SERVER(zpt::auth::oauth2::token_provider::ptr _token_provider = nullptr)
  -> zpt::auth::oauth2::server&;
} // namespace zpt
