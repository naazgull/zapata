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

auto
zpt::net::transport::http::set_headers(zpt::message& _message, zpt::http::req& _request)
  -> zpt::net::transport::http& {
    zpt::json& _headers = _message->headers();
    for (auto [_key, _value] : _request->headers()) {
        _headers << _key << _value;
    }
    return (*this);
}

auto
zpt::net::transport::http::set_body(zpt::message& _message, zpt::http::req& _request)
  -> zpt::net::transport::http& {
    if (_request->body().length() != 0) {
        if (!_message->received()->ok()) {
            _message->received() = zpt::json::object();
        }
        if (_message->headers()["Content-Type"] == "application/json") {
            std::istringstream _iss;
            _iss.str(_request->body());
            zpt::json _body;
            _iss >> _body;
            _message->received() << "body" << _body;
        }
        else {
            _message->received() << "body" << _request->body();
        }
    }
    return (*this);
}

auto
zpt::net::transport::http::set_params(zpt::message& _message, zpt::http::req& _request)
  -> zpt::net::transport::http& {
    zpt::json _params = zpt::json::object();
    for (auto [_key, _value] : _request->params()) {
        _params << _key << _value;
    }
    if (_params->size() != 0) {
        if (!_message->received()->ok()) {
            _message->received() = zpt::json::object();
        }
        _message->received() << "params" << _params;
    }
    return (*this);
}

auto
zpt::net::transport::http::receive(zpt::message& _message) -> void {
    zpt::http::req _request;
    _message->stream() >> _request;
    zlog("Received HTTP message " << zpt::http::method_names[_request->method()] << " "
                                  << _request->url() << " with " << _request->body().length()
                                  << " bytes content",
         zpt::trace);

    this
      ->set_headers(_message, _request) //
      .set_body(_message, _request)
      .set_params(_message, _request);
    _message
      ->version() //
      .assign(_request->version());
    _message
      ->scheme() //
      .assign("http");
    _message
      ->uri() //
      .assign(_request->url());
    _message->method() = _request->method();
    _message->keep_alive() = (_request->header("Connection") == "keep-alive");
}

auto
zpt::net::transport::http::send(zpt::message& _message) -> void {
    zpt::http::rep _response;
    zpt::init(_response);

    zpt::http::status _status = static_cast<zpt::http::status>(_message->status());
    _response->status(_status);
    _response->version(_message->version());
    if (_message->to_send()->ok()) {
        _response->body(_message->to_send());
        _response->header("Content-Type", "application/json");
    }
    zlog("Sending HTTP message " << _status << " with length "
                                 << _response->header("Content-Length") << " bytes content",
         zpt::trace);
    _message->stream() << _response << std::flush;
}
