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

#include <map>
#include <memory>
#include <string>
#include <zapata/base.h>
#include <zapata/json.h>

namespace zpt {
/** @brief Global accessor for the OAuth2 token provider index. Used internally by the plugin
    to pass a token provider instance to the OAuth2 server.
 * @return Reference to the token provider index variable. */
auto OAUTH2_TOKEN_PROVIDER() -> ssize_t&;
/** @brief Global accessor for the OAuth2 server instance. Used by all listeners to dispatch
    requests to the server.
 * @return Reference to the OAuth2 server instance variable. */
auto OAUTH2_SERVER() -> ssize_t&;

namespace auth {
/** @brief Extract an access token from an envelope. Checks the Authorization header first,
    then the request body, then URL params. Returns an empty string if no token is found. */
auto extract(zpt::json _envelope) -> std::string;

namespace oauth2 {
/**
    @name Token Provider Interface
    @brief Abstract interface for OAuth2 token storage and validation. Implement this class
    to provide custom token persistence backed by a database, cache, or other store.
   @{
*/

class token_provider {
  public:
    /** @brief Default constructor. */
    token_provider() = default;
    /** @brief Virtual destructor. */
    virtual ~token_provider() = default;

    /** @brief Retrieve owner information from the envelope. Used during the authorization code
        flow to authenticate the resource owner. @param _envelope The request envelope containing
        authentication data. @return JSON containing the owner's identity. */
    virtual auto retrieve_owner(zpt::json _envelope) -> zpt::json = 0;
    /** @brief Retrieve owner information using explicit credentials. Used during the password
        grant flow. @param _owner The username or owner identifier. @param _password The owner's
        password. @param _client_id The requesting client's identifier. @return JSON containing
        the owner's identity. */
    virtual auto retrieve_owner(std::string const& _owner,
                                std::string const& _password,
                                std::string const& _client_id) -> zpt::json = 0;
    /** @brief Retrieve client information from the envelope. Used to validate the requesting
        application during authorization flows. @param _envelope The request envelope containing
        client credentials. @return JSON containing the client's identity. */
    virtual auto retrieve_client(zpt::json _envelope) -> zpt::json = 0;
    /** @brief Retrieve client information using explicit credentials. Used during the client
        credentials grant flow. @param _client_id The client's identifier. @param _client_secret
        The client's secret. @return JSON containing the client's identity. */
    virtual auto retrieve_client(std::string const& _client_id, std::string const& _client_secret)
      -> zpt::json = 0;
    /** @brief Store a generated token in the backing store. @param _token The token JSON to
        store. @return A string identifier for the stored token. */
    virtual auto store_token(zpt::json _token) -> std::string = 0;
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
        topic. @param _envelope The request envelope. @param _topic The topic being accessed.
        @param _permissions The permissions required. @return true if the token has sufficient
        permissions. */
    virtual auto validate_roles_permissions(zpt::json _envelope,
                                            std::string _topic,
                                            zpt::json _permissions) -> bool = 0;
    /** @brief Remove (revoke) a token from the backing store. Used when a token has expired
        or a refresh token is being replaced. @param _token The token JSON to revoke. */
    virtual auto remove_token(zpt::json _token) -> void = 0;
};

/**@}
 */
using token_provider_ptr = std::shared_ptr<zpt::auth::oauth2::token_provider>;

/** @name OAuth2 Server
    @brief Central OAuth2 authorization server. Handles all OAuth2 grant flows (authorization
    code, password, client credentials) and provides token validation.
   @{
*/

class server {
  public:
    /** @brief Construct an OAuth2 server. @param _token_provider The token provider implementation
        for storage and lookup. @param _options Server configuration options including domain and
        URL settings. */
    server(zpt::auth::oauth2::token_provider_ptr _token_provider, zpt::json _options);
    /** @brief Virtual destructor.
     * @return void (destructors implicitly clean up the object). */
    virtual ~server();

    /** @brief Return the server's configuration options. @return JSON containing server options. */
    virtual auto options() -> zpt::json;
    /** @brief Return the server's name. @return The string "oauth2.0". */
    virtual auto name() -> std::string;

    /** @brief Authorize a request based on the grant type. Dispatches to the appropriate
        grant handler (authorization code, password, client credentials).
        @param _performative The HTTP method (Get or Post).
        @param _envelope The request envelope containing OAuth2 parameters.
        @param _opts Server configuration options.
        @return JSON response with redirect URL or error. */
    virtual auto authorize(zpt::performative _performative, zpt::json _envelope, zpt::json _opts)
      -> zpt::json;
    /** @brief Authorize a request by validating the access token against required roles.
        Used for internal authorization checks.
        @param _topic The topic being accessed.
        @param _envelope The request envelope containing the access token.
        @param _roles_needed The roles required to access the topic.
        @return JSON containing the token's identity data if authorized. */
    virtual auto authorize(std::string const& _topic, zpt::json _envelope, zpt::json _roles_needed)
      -> zpt::json;
    /** @brief Exchange an authorization code for access and refresh tokens.
        @param _performative The HTTP method (Get or Post).
        @param _envelope The request envelope containing client credentials and the code.
        @param _opts Server configuration options.
        @return JSON response with redirect or token data. */
    virtual auto token(zpt::performative _performative, zpt::json _envelope, zpt::json _opts)
      -> zpt::json;
    /** @brief Exchange a refresh token for a new pair of access and refresh tokens.
        The old refresh token is revoked.
        @param _performative The HTTP method (Get or Post).
        @param _envelope The request envelope containing the refresh token.
        @param _opts Server configuration options.
        @return JSON response with redirect or new token data. */
    virtual auto refresh(zpt::performative _performative, zpt::json _envelope, zpt::json _opts)
      -> zpt::json;
    /** @brief Validate an access token. Checks that the token is valid and has not expired.
        @param _performative The HTTP method (Get or Post).
        @param _envelope The request envelope containing the access token.
        @param _opts Server configuration options.
        @return JSON token data if valid. */
    virtual auto validate(zpt::performative _performative, zpt::json _envelope, zpt::json _opts)
      -> zpt::json;

    /**@}
     */

  private:
    /** @brief The token provider for storage and lookup operations. */
    zpt::auth::oauth2::token_provider_ptr __token_provider{ nullptr };
    /** @brief Server configuration options. */
    zpt::json __options;

    /**
     * @brief Handle the authorization code grant flow. Prompts the owner to authenticate
     * if needed, then generates a code token.
     * @param _performative The HTTP method (Get or Post).
     * @param _request The HTTP request envelope.
     * @param _envelope The request envelope containing OAuth2 parameters.
     * @param _opts Server configuration options.
     * @return JSON response with redirect URL or authorization code.
     */
    auto authorize_with_code(zpt::performative _performative,
                             zpt::json _request,
                             zpt::json _envelope,
                             zpt::json _opts) -> zpt::json;
    /**
     * @brief Handle the password grant flow. Authenticates the owner with username and
     * password, then generates an access token.
     * @param _performative The HTTP method (Get or Post).
     * @param _request The HTTP request envelope.
     * @param _envelope The request envelope containing credentials.
     * @param _opts Server configuration options.
     * @return JSON response with redirect URL or access token data.
     */
    auto authorize_with_password(zpt::performative _performative,
                                 zpt::json _request,
                                 zpt::json _envelope,
                                 zpt::json _opts) -> zpt::json;
    /**
     * @brief Handle the client credentials grant flow. Authenticates the client with
     * its credentials and generates an access token.
     * @param _performative The HTTP method (Get or Post).
     * @param _request The HTTP request envelope.
     * @param _envelope The request envelope containing client credentials.
     * @param _opts Server configuration options.
     * @return JSON response with redirect URL or access token data.
     */
    auto authorize_with_client_credentials(zpt::performative _performative,
                                           zpt::json _request,
                                           zpt::json _envelope,
                                           zpt::json _opts) -> zpt::json;
    /**
     * @brief Generate a new token with access token, refresh token, code, and 90-day
     * expiration. Enriches the token with roles/permissions from the provider.
     * @param _data Token data payload.
     * @return JSON containing the generated token(s) and metadata.
     */
    auto generate_token(zpt::json _data) -> zpt::json;
};
} // namespace oauth2
} // namespace auth
} // namespace zpt
