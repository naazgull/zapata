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

#include <zapata/config.h>
#include <zapata/oauth2.h>
#include <zapata/rest.h>
#include <zapata/startup.h>

extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Registering handlers for oauth2.0", zpt::info);
    auto const& _config = zpt::GLOBAL_CONFIG()("oauth2")("url");
    auto _resolver = zpt::REST_RESOLVER();
    _resolver //
      ->add<zpt::auth::oauth2::authorize>(_config("authorize")->string())
      .add<zpt::auth::oauth2::token>(_config("token")->string())
      .add<zpt::auth::oauth2::refresh>(_config("refresh")->string())
      .add<zpt::auth::oauth2::validate>(_config("validate")->string())
      .add<zpt::auth::oauth2::device_authorization>(_config("device_authorization")->string())
      .add<zpt::auth::oauth2::approve>(_config("approve")->string());
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unregistering handlers for oauth2.0", zpt::info);
    auto const& _config = zpt::GLOBAL_CONFIG()("oauth2")("url");
    auto _resolver = zpt::REST_RESOLVER();
    _resolver //
      ->remove<zpt::auth::oauth2::authorize>(_config("authorize")->string())
      .remove<zpt::auth::oauth2::token>(_config("token")->string())
      .remove<zpt::auth::oauth2::refresh>(_config("refresh")->string())
      .remove<zpt::auth::oauth2::validate>(_config("validate")->string())
      .remove<zpt::auth::oauth2::device_authorization>(_config("device_authorization")->string())
      .remove<zpt::auth::oauth2::approve>(_config("approve")->string());
}
