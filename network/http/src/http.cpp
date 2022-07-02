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
zpt::net::transport::http::set_headers(zpt::exchange& _channel, zpt::HTTPObj& _http_message) const
  -> const zpt::net::transport::http& {
    if (!_channel->received()->ok()) { _channel->received() = zpt::json::object(); }

    zpt::json _headers = _channel->received()["headers"];
    if (!_headers->ok()) {
        _headers = zpt::json::object();
        _channel->received() << "headers" << _headers;
    }
    for (auto [_key, _value] : _http_message.headers()) { _headers << _key << _value; }
    _headers << "Content-Type" << _channel->content_type()[0];
    return (*this);
}

auto
zpt::net::transport::http::set_body(zpt::exchange& _channel, zpt::HTTPObj& _http_message) const
  -> const zpt::net::transport::http& {
    if (_http_message.body().length() != 0) {
        if (!_channel->received()->ok()) { _channel->received() = zpt::json::object(); }
        auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        std::istringstream _is;
        _is.str(_http_message.body());
        zpt::json _content = _layer.translate(_is, _channel->content_type()[0]);
        _channel->received() << "body" << _content;
    }
    return (*this);
}

auto
zpt::net::transport::http::set_params(zpt::exchange& _channel, zpt::http::req& _request) const
  -> const zpt::net::transport::http& {
    auto _params = _request->params();
    if (_params->ok()) {
        if (!_channel->received()->ok()) { _channel->received() = zpt::json::object(); }
        _channel->received() << "params" << _params;
    }
    return (*this);
}

auto
zpt::net::transport::http::set_method(zpt::exchange& _channel, zpt::http::req& _request) const
  -> const zpt::net::transport::http& {
    if (!_channel->received()->ok()) { _channel->received() = zpt::json::object(); }
    _channel->received() << "performative" << _request->method();
    return (*this);
}

auto
zpt::net::transport::http::set_method(zpt::exchange& _channel, zpt::http::rep& _response) const
  -> const zpt::net::transport::http& {
    if (!_channel->received()->ok()) { _channel->received() = zpt::json::object(); }
    _channel->received() << "performative" << zpt::Reply;
    return (*this);
}

auto
zpt::net::transport::http::set_status(zpt::exchange& _channel, zpt::http::rep& _response) const
  -> const zpt::net::transport::http& {
    if (!_channel->received()->ok()) { _channel->received() = zpt::json::object(); }
    _channel->received() << "status" << _response->status();
    return (*this);
}

auto
zpt::net::transport::http::set_headers(zpt::http::req& _request, zpt::exchange& _channel) const
  -> const zpt::net::transport::http& {
    for (auto [_idx, _key, _value] : _channel->to_send()["headers"]) {
        _request->header(_key, _value);
    }
    return (*this);
}

auto
zpt::net::transport::http::set_headers(zpt::http::rep& _reply, zpt::exchange& _channel) const
  -> const zpt::net::transport::http& {
    if (_channel->received()->ok()) {
        auto _req_headers = _channel->received()["headers"];
        if (_req_headers["Cache-Control"]->ok()) {
            _reply->header("Cache-Control", _req_headers["Cache-Control"]->string());
        }
    }
    for (auto [_idx, _key, _value] : _channel->to_send()["headers"]) {
        _reply->header(_key, _value);
    }
    return (*this);
}

auto
zpt::net::transport::http::set_body(zpt::http::req& _request, zpt::exchange& _channel) const
  -> const zpt::net::transport::http& {
    auto _body = _channel->to_send()["body"];
    if (_body->ok()) {
        auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        std::ostringstream _os;
        _request->header("Content-Type", _layer.translate(_os, _channel->content_type()[0], _body));
        _request->body(_os.str());
    }
    return (*this);
}

auto
zpt::net::transport::http::set_body(zpt::http::rep& _reply, zpt::exchange& _channel) const
  -> const zpt::net::transport::http& {
    auto _body = _channel->to_send()["body"];
    if (_body->ok()) {
        auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
        std::ostringstream _os;
        _reply->header("Content-Type", _layer.translate(_os, _channel->content_type()[0], _body));
        _reply->body(_os.str());
    }
    return (*this);
}

auto
zpt::net::transport::http::set_method(zpt::http::req& _request, zpt::exchange& _channel) const
  -> const zpt::net::transport::http& {
    if (_channel->to_send()["performative"]->ok()) {
        _request->method(static_cast<unsigned int>(_channel->to_send()["performative"]));
    }
    return (*this);
}

auto
zpt::net::transport::http::set_status(zpt::http::rep& _response, zpt::exchange& _channel) const
  -> const zpt::net::transport::http& {
    if (_channel->to_send()["status"]->ok()) {
        _response->status(static_cast<zpt::http::status>(_channel->to_send()["status"]->integer()));
    }
    return (*this);
}

auto
zpt::net::transport::http::receive(zpt::exchange& _channel) const -> void {
    if (_channel->stream().state() == zpt::stream_state::IDLE) {
        this->receive_request(_channel);
        _channel->stream().state() = zpt::stream_state::PROCESSING;
    }
    else if (_channel->stream().state() == zpt::stream_state::WAITING) {
        this->receive_reply(_channel);
        _channel->stream().state() = zpt::stream_state::IDLE;
    }
}

auto
zpt::net::transport::http::send(zpt::exchange& _channel) const -> void {
    if (_channel->stream().state() == zpt::stream_state::IDLE) {
        this->send_request(_channel);
        _channel->stream().state() = zpt::stream_state::WAITING;
    }
    else if (_channel->stream().state() == zpt::stream_state::PROCESSING) {
        this->send_reply(_channel);
        _channel->stream().state() = zpt::stream_state::IDLE;
    }
}

auto
zpt::net::transport::http::resolve(zpt::json _uri) const -> zpt::exchange {
    expect(_uri["scheme"]->ok() && _uri["domain"]->ok(),
           "URI parameter must contain 'scheme', 'domain' and 'port'",
           500,
           0);
    expect(_uri["scheme"]->string().find("http") == 0, "scheme must be 'http(s)'", 500, 0);
    auto _stream = zpt::stream::alloc<zpt::basic_socketstream<char>>(
      _uri["domain"],
      _uri["port"]->ok() ? _uri["port"]->integer() : 80,
      _uri["scheme"]->string() == "https");
    _stream->transport("http");
    zpt::exchange _to_return{ _stream.release() };
    _to_return->to_send() = {
        "headers",
        { "Host",
          (_uri["domain"]->string() + std::string{ ":" } +
           (_uri["port"]->ok() ? static_cast<std::string>(_uri["port"]) : "80")) }
    };
    _to_return //
      ->scheme()
      .assign(_uri["scheme"]->string());
    _to_return //
      ->uri()
      .assign(zpt::path::join(_uri["path"]));
    _to_return //
      ->version()
      .assign("1.1");
    return _to_return;
}

auto
zpt::net::transport::http::receive_request(zpt::exchange& _channel) const -> void {
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
      .assign(_request->path());
    _channel->keep_alive() = (_request->header("Connection") == "keep-alive");
}

auto
zpt::net::transport::http::receive_reply(zpt::exchange& _channel) const -> void {
    zpt::http::rep _response;
    _channel->stream() >> _response;
    zlog("Received HTTP message:\n" << _response, zpt::trace);

    this //
      ->set_headers(_channel, *_response)
      .set_body(_channel, *_response)
      .set_method(_channel, _response)
      .set_status(_channel, _response);
    _channel //
      ->version()
      .assign(_response->version());
    _channel //
      ->scheme()
      .assign("http");
    _channel //
      ->uri()
      .assign("/" + std::to_string(static_cast<int>(_channel->stream())));
    _channel->keep_alive() = (_response->header("Connection") == "keep-alive");
}

auto
zpt::net::transport::http::send_request(zpt::exchange& _channel) const -> void {
    zpt::http::req _request;
    zpt::init(_request);

    this //
      ->set_method(_request, _channel)
      .set_headers(_request, _channel)
      .set_body(_request, _channel);
    _request->method(static_cast<unsigned int>(_channel->to_send()["performative"]));
    _request->url(_channel->uri());
    _request->version(_channel->version());
    zlog("Sending HTTP message:\n" << _request, zpt::trace);
    _channel->stream() << _request << std::flush;
}

auto
zpt::net::transport::http::send_reply(zpt::exchange& _channel) const -> void {
    zpt::http::rep _response;
    zpt::init(_response);

    if (_channel->to_send()->ok()) {
        this //
          ->set_status(_response, _channel)
          .set_headers(_response, _channel)
          .set_body(_response, _channel);
    }
    else { _response->status(zpt::http::HTTP404); }
    _response->version(_channel->version());
    zlog("Sending HTTP message:\n" << _response, zpt::trace);
    _channel->stream() << _response << std::flush;
}
