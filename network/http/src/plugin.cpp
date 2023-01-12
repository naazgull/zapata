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
#include <zapata/startup.h>
#include <zapata/net/socket.h>
#include <zapata/net/http.h>

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _config = _plugin.config();

    zpt::global_cast<zpt::network::layer>(zpt::TRANSPORT_LAYER())
      .add("http", zpt::make_transport<zpt::net::transport::http>());

    if (_config("port")->ok()) {
        auto& _server_sock = zpt::make_global<zpt::serversocketstream>(
          zpt::HTTP_SERVER_SOCKET(),
          static_cast<std::uint16_t>(static_cast<unsigned int>(_config("port"))));

        _plugin.add_thread([&]() -> void {
            auto& _polling = zpt::global_cast<zpt::polling>(zpt::STREAM_POLLING());
            zlog("Started HTTP transport on port " << _config("port"), zpt::info);

            do {
                try {
                    auto _client = _server_sock->accept();
                    _client->transport("http");
                    _polling.listen_on(std::move(_client));
                }
                catch (zpt::ClosedException const& _e) {
                    expect(_plugin.is_shutdown_ongoing(),
                           "HTTP server socket closed but plugin not shutding down");
                }
            } while (!_plugin.is_shutdown_ongoing());
            zlog("Stopped HTTP transport on port " << _config("port"), zpt::info);
        });
    }
}

extern "C" auto _zpt_unload_(zpt::plugin& _plugin) {
    auto& _config = _plugin.config();
    if (_config("port")->ok()) {
        zpt::global_cast<zpt::serversocketstream>(zpt::HTTP_SERVER_SOCKET())->close();
        zpt::release_global<zpt::serversocketstream>(zpt::HTTP_SERVER_SOCKET());
    }
}
