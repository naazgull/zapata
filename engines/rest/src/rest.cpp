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

#include <zapata/rest/rest.h>

auto zpt::REST_ENGINE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::rest::engine::engine(zpt::json _configuration) {
    zpt::global_cast<zpt::polling>(zpt::STREAM_POLLING())
      .register_delegate([this](zpt::polling& _poll, zpt::basic_stream& _stream) -> bool {
          return this->delegate(_poll, _stream);
      });
}

auto zpt::rest::engine::delegate(zpt::polling& _poll, zpt::basic_stream& _stream) -> bool {
    zpt::global_cast<zpt::events::dispatcher>(zpt::DISPATCHER())
      .trigger<zpt::rest::receive_message>(_poll, _stream);
    return true;
}

zpt::rest::receive_message::receive_message(zpt::polling& _polling, zpt::basic_stream& _stream)
  : __polling{ _polling }
  , __stream{ _stream } {}

auto zpt::rest::receive_message::blocked() const -> bool { return false; }

auto zpt::rest::receive_message::operator()(zpt::events::dispatcher& _dispatcher) -> zpt::events::state {
    auto _transport = zpt::global_cast<zpt::network::layer>(zpt::TRANSPORT_LAYER()) //
                        .get(this->__stream.transport());
    try {
        auto _request = _transport->receive(this->__stream);
        auto _reply = _transport->make_reply(_request);
        _reply->status(200);
        _reply->body() = "<h1>Hello WORLD!!!</h1>";
        _transport->send(this->__stream, _reply);
    }
    catch (zpt::SyntaxErrorException const& _e) {
        auto _reply = _transport->make_reply();
        _reply->status(500);
        _reply->headers()["Content-Type"] = "application/json";
        _reply->body() = { "error",    500,            //
                           "type",     "Protocol",     //
                           "category", "Syntax Error", //
                           "message",  _e.what() };
        _transport->send(this->__stream, _reply);
    }
    this->__polling.unmute(this->__stream);
    return zpt::events::finish;
}
