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

#include <zapata/net/socket.h>
#include <zapata/net/upnp.h>
#include <zapata/transport.h>
#include <zapata/upnp/UPNPObj.h>

auto main(int _argc, char* _argv[]) -> int {
    if (_argc > 3) {
        std::string _role{ _argv[1] };
        std::stringstream _iss;
        _iss.str(std::string{ _argv[3] });
        std::uint16_t _port{ 0 };
        _iss >> _port;
        zpt::json _config{ "bind", _argv[2], "port", _port };
        zlog(_config, zpt::debug);
        zpt::transport _transport{ new zpt::net::transport::upnp{} };

        if (_role == "server") {
            auto _stream = zpt::make_stream<zpt::socketstream>(
              _config("bind")->string(), _config("port")->integer(), zpt::NO_SSL, IPPROTO_UDP);
            _stream->transport("upnp");

            zpt::polling::ptr _polling = zpt::allocate_shared<zpt::polling>();
            _polling //
              ->register_delegate(
                [&_transport](zpt::polling::ptr _poll, zpt::stream _stream) -> bool {
                    try {
                        _transport->receive(_stream);
                    }
                    catch (...) {
                        zlog("Nothing to receive", zpt::debug);
                    }
                    _poll->unmute(_stream);
                    return true;
                })
              .listen_on(std::move(_stream))
              .poll()
              .shutdown();
        }
        if (_role == "client") {
            auto _message = _transport->make_request();
            auto& _upnp = zpt::message_cast<zpt::upnp::basic_request>(_message);
            _upnp //
              .performative(zpt::Msearch)
              .uri("*");
            zlog(_upnp, zpt::debug);

            auto _stream = zpt::make_stream<zpt::socketstream>(zpt::NO_SSL, IPPROTO_UDP);
            _stream //
              ->transport("upnp")
              .set_peer<zpt::socketstream>(_config("bind")->string(), _config("port")->integer());

            _transport->send(_stream, _message);

            if (zpt::stream_cast<zpt::socketstream>(_stream).is_error()) {
                zlog(zpt::stream_cast<zpt::socketstream>(_stream).error_string(), zpt::debug);
            }
        }
    }
}
