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
#include <zapata/oauth2/config.h>

namespace zpt {
namespace authenticator {

auto
extract(zpt::json _envelope) -> std::string;

class OAuth2 {
  public:
    OAuth2(zpt::json _options);
    virtual ~OAuth2();

    virtual auto options() -> zpt::json;
    virtual auto name() -> std::string;

    virtual auto authorize(zpt::performative _performative, zpt::json _envelope, zpt::json _opts)
      -> zpt::json;
    virtual auto authorize(std::string const& _topic, zpt::json _envelope, zpt::json _roles_needed)
      -> zpt::json;
    virtual auto token(zpt::performative _performative, zpt::json _envelope, zpt::json _opts)
      -> zpt::json;
    virtual auto refresh(zpt::performative _performative, zpt::json _envelope, zpt::json _opts)
      -> zpt::json;
    virtual auto validate(std::string const& _access_token, zpt::json _opts) -> zpt::json;

    virtual auto retrieve_owner(zpt::json _envelope) -> zpt::json = 0;
    virtual auto retrieve_owner(std::string const& _owner,
                                std::string const& _password,
                                std::string const& _client_id) -> zpt::json = 0;
    virtual auto retrieve_client(zpt::json _envelope) -> zpt::json = 0;
    virtual auto retrieve_client(std::string const& _client_id, std::string const& _client_secret)
      -> zpt::json = 0;
    virtual auto store_token(zpt::json _token) -> std::string = 0;
    virtual auto get_code(std::string const& _code) -> zpt::json = 0;
    virtual auto get_token(std::string const& _access_token) -> zpt::json = 0;
    virtual auto get_refresh_token(std::string const& _refresh_token) -> zpt::json = 0;
    virtual auto get_roles_permissions(zpt::json _token) -> zpt::json = 0;
    virtual auto validate_roles_permissions(zpt::json _envelope,
                                            std::string _topic,
                                            zpt::json _permissions) -> bool = 0;
    virtual auto remove_token(zpt::json _token) -> void = 0;

  private:
    zpt::json __options;

    auto generate_token(zpt::json _data) -> zpt::json;
};
} // namespace authenticator
} // namespace zpt
