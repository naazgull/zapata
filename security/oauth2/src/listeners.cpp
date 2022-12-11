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
#include <zapata/http.h>
#include <zapata/oauth2/oauth2.h>
#include <zapata/oauth2/listeners.h>

auto zpt::auth::oauth2::authorize_listener(zpt::pipeline::event<zpt::exchange>& _event) -> void {
    auto& _channel = _event->content();
    auto _message = _channel->received();
    auto _body = _message["body"];
    auto _config = zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG());
    auto& _server = zpt::global_cast<zpt::auth::oauth2::server>(zpt::OAUTH2_SERVER());
    auto _authorization =
      _server.authorize(_message["performative"]->integer(), _message, _config["oauth2"]);
    _channel->to_send() = _authorization;
}

auto zpt::auth::oauth2::token_listener(zpt::pipeline::event<zpt::exchange>& _event) -> void {
    auto& _channel = _event->content();
    auto _message = _channel->received();
    auto _body = _message["body"];
    auto _config = zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG());
    auto& _server = zpt::global_cast<zpt::auth::oauth2::server>(zpt::OAUTH2_SERVER());
    auto _token = _server.token(_message["performative"]->integer(), _message, _config["oauth2"]);
    _channel->to_send() = _token;
}

auto zpt::auth::oauth2::refresh_listener(zpt::pipeline::event<zpt::exchange>& _event) -> void {
    auto& _channel = _event->content();
    auto _message = _channel->received();
    auto _body = _message["body"];
    auto _config = zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG());
    auto& _server = zpt::global_cast<zpt::auth::oauth2::server>(zpt::OAUTH2_SERVER());
    auto _refresh =
      _server.refresh(_message["performative"]->integer(), _message, _config["oauth2"]);
    _channel->to_send() = _refresh;
}

auto zpt::auth::oauth2::validate_listener(zpt::pipeline::event<zpt::exchange>& _event) -> void {
    auto& _channel = _event->content();
    auto _message = _channel->received();
    auto _body = _message["body"];
    auto _config = zpt::global_cast<zpt::json>(zpt::GLOBAL_CONFIG());
    auto& _server = zpt::global_cast<zpt::auth::oauth2::server>(zpt::OAUTH2_SERVER());
    auto _validate =
      _server.validate(_message["performative"]->integer(), _message, _config["oauth2"]);
    _channel->to_send() = _validate;
}
