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

#include <zapata/net/transport/http.h>
#include <zapata/base.h>
#include <zapata/globals/globals.h>
#include <zapata/uri/uri.h>
#include <zapata/net/socket/socket_stream.h>

auto
zpt::HTTP_SERVER_SOCKET() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

auto
zpt::net::transport::http::set_headers(zpt::exchange& _channel, zpt::HTTPObj& _http_message)
  -> zpt::net::transport::http& {
    if (!_channel->received()->ok()) {
        _channel->received() = zpt::json::object();
    }

    zpt::json _headers = _channel->received()["headers"];
    if (!_headers->ok()) {
        _headers = zpt::json::object();
        _channel->received() << "headers" << _headers;
    }
    for (auto [_key, _value] : _http_message.headers()) {
        _headers << _key << _value;
    }
    if (!_headers["Content-Type"]->ok()) {
        _headers << "Content-Type"
                 << "*/*";
    }
    return (*this);
}

auto
zpt::net::transport::http::set_body(zpt::exchange& _channel, zpt::HTTPObj& _http_message)
  -> zpt::net::transport::http& {
    if (_http_message.body().length() != 0) {
        if (!_channel->received()->ok()) {
            _channel->received() = zpt::json::object();
        }

        auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        std::istringstream _is;
        _is.str(_http_message.body());

        std::string _content_type = _channel->received()["headers"]["Content-Type"];
        zpt::json _content = _layer.translate(_is, _content_type);

        _channel->received() << "body" << _content;
    }
    return (*this);
}

auto
zpt::net::transport::http::set_params(zpt::exchange& _channel, zpt::http::req& _request)
  -> zpt::net::transport::http& {
    auto& _query = _request->query();
    if (_query.length() != 0) {
        zpt::json _params = zpt::uri::parse(std::string{ "?" } + _query);
        if (!_channel->received()->ok()) {
            _channel->received() = zpt::json::object();
        }
        _channel->received() << "params" << _params["params"];
    }
    return (*this);
}

auto
zpt::net::transport::http::set_method(zpt::exchange& _channel, zpt::http::req& _request)
  -> zpt::net::transport::http& {
    if (!_channel->received()->ok()) {
        _channel->received() = zpt::json::object();
    }
    _channel->received() << "method" << _request->method();
    return (*this);
}

auto
zpt::net::transport::http::set_status(zpt::exchange& _channel, zpt::http::rep& _response)
  -> zpt::net::transport::http& {
    if (!_channel->received()->ok()) {
        _channel->received() = zpt::json::object();
    }
    _channel->received() << "status" << _response->status();
    return (*this);
}

auto
zpt::net::transport::http::set_headers(zpt::http::req& _request, zpt::exchange& _channel)
  -> zpt::net::transport::http& {
    for (auto [_idx, _key, _value] : _channel->to_send()["headers"]) {
        _request->header(_key, _value);
    }
    return (*this);
}

auto
zpt::net::transport::http::set_body(zpt::http::req& _request, zpt::exchange& _channel)
  -> zpt::net::transport::http& {
    if (_channel->to_send()["body"]->ok()) {
        _request->body(static_cast<std::string>(_channel->to_send()["body"]));
    }
    return (*this);
}

auto
zpt::net::transport::http::set_method(zpt::http::req& _request, zpt::exchange& _channel)
  -> zpt::net::transport::http& {
    if (_channel->to_send()["method"]->ok()) {
        _request->method(static_cast<unsigned int>(_channel->to_send()["method"]));
    }
    return (*this);
}

auto
zpt::net::transport::http::receive_request(zpt::exchange& _channel) -> void {
    zpt::http::req _request;
    _channel->stream() >> _request;
    zlog("Received HTTP message:\n" << _request, zpt::trace);

    this //
      ->set_headers(_channel, *_request)
      .set_body(_channel, *_request)
      .set_method(_channel, _request)
      .set_params(_channel, _request);
    _channel //
      ->version()
      .assign(_request->version());
    _channel //
      ->scheme()
      .assign("http");
    _channel //
      ->uri()
      .assign(_request->url());
    _channel->keep_alive() = (_request->header("Connection") == "keep-alive");
}

auto
zpt::net::transport::http::send_reply(zpt::exchange& _channel) -> void {
    zpt::http::rep _response;
    zpt::init(_response);

    zlog(zpt::pretty(_channel->to_send()), zpt::trace);
    zpt::http::status _status =
      static_cast<zpt::http::status>(_channel->to_send()["status"]->intr());
    _response->status(_status);
    _response->version(_channel->version());
    if (_channel->to_send()["body"]->ok()) {
        _response->body(_channel->to_send()["body"]);
        _response->header("Content-Type", _channel->to_send()["headers"]["Content-Type"]);
    }
    zlog("Sending HTTP message:\n" << _response, zpt::trace);
    _channel->stream() << _response << std::flush;
}

auto
zpt::net::transport::http::send_request(zpt::exchange& _channel) -> void {
    zpt::http::req _request;
    zpt::init(_request);

    this //
      ->set_method(_request, _channel)
      .set_headers(_request, _channel)
      .set_body(_request, _channel);
    _request->method(static_cast<unsigned int>(_channel->to_send()["method"]));
    _request->url(_channel->uri());
    _request->version(_channel->version());
    _channel->stream() << _request << std::flush;
    zlog("Sending HTTP message:\n" << _request, zpt::trace);
}

auto
zpt::net::transport::http::receive_reply(zpt::exchange& _channel) -> void {
    zpt::http::rep _response;
    _channel->stream() >> _response;
    zlog("Received HTTP message:\n" << _response, zpt::trace);

    this //
      ->set_headers(_channel, *_response)
      .set_body(_channel, *_response)
      .set_status(_channel, _response);
    _channel //
      ->version()
      .assign(_response->version());
    _channel //
      ->scheme()
      .assign("http");
    _channel->keep_alive() = false;
}

auto
zpt::net::transport::http::resolve(zpt::json _uri) -> zpt::exchange {
    expect(_uri["scheme"]->ok() && _uri["domain"]->ok(),
           "URI parameter must contain 'scheme', 'domain' and 'port'",
           500,
           0);
    expect(_uri["scheme"]->str().find("http") == 0, "scheme must be 'http(s)'", 500, 0);
    auto _stream = zpt::stream::alloc<zpt::basic_socketstream<char>>(
      _uri["domain"],
      _uri["port"]->ok() ? _uri["port"]->intr() : 80,
      _uri["scheme"]->str() == "https");
    zpt::exchange _to_return{ _stream.release() };
    _to_return->to_send() = {
        "headers",
        { "Host",
          (_uri["domain"]->str() + std::string{ ":" } +
           (_uri["port"]->ok() ? static_cast<std::string>(_uri["port"]) : "80")) }
    };
    _to_return //
      ->scheme()
      .assign(_uri["scheme"]->str());
    _to_return //
      ->uri()
      .assign(zpt::path::join(_uri["path"]));
    _to_return //
      ->version()
      .assign("1.1");
    return _to_return;
}
