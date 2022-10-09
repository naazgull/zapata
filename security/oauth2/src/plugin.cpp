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

#include <iostream>
#include <zapata/rest.h>
#include <zapata/oauth2/oauth2.h>
#include <zapata/oauth2/listeners.h>

std::atomic<bool> _shutdown{ false };

extern "C" auto
_zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _rest = zpt::globals::get<zpt::rest::engine>(zpt::REST_ENGINE());
    auto _config = _plugin->config();
    auto& _token_provider = zpt::globals::get<zpt::auth::oauth2::token_provider_ptr>(zpt::OAUTH2_TOKEN_PROVIDER());
    zpt::globals::alloc<zpt::auth::oauth2::server>(zpt::OAUTH2_SERVER(), _token_provider, _config);

    size_t _step = _plugin->config()["add_to_step"]->integer();

    _rest.add_listener(_step,
                       std::string{ "/{:(GET|POST):}" } + _config["authorize_url"]->string(),
                       zpt::auth::oauth2::authorize_listener);
    _rest.add_listener(
      _step, std::string{ "/{:(GET|POST):}" } + _config["token_url"]->string(), zpt::auth::oauth2::token_listener);
    _rest.add_listener(
      _step, std::string{ "/{:(GET|POST):}" } + _config["refresh_url"]->string(), zpt::auth::oauth2::refresh_listener);
    _rest.add_listener(_step,
                       std::string{ "/{:(GET|POST):}" } + _config["validate_url"]->string(),
                       zpt::auth::oauth2::validate_listener);

    zlog("Registering listeners for oauth2.0", zpt::info);
}

extern "C" auto
_zpt_unload_(zpt::plugin& _plugin) -> void {
    zlog("Unregistering listeners for oauth2.0", zpt::info);
    zpt::globals::dealloc<zpt::auth::oauth2::server>(zpt::OAUTH2_SERVER());
}
