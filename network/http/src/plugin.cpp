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

extern "C" auto
_zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _boot = zpt::globals::get<zpt::startup::engine>(zpt::BOOT_ENGINE());
    auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
    auto& _config = _plugin->config();

    _layer.add("http", zpt::transport::alloc<zpt::net::transport::http>());
    if (_config["port"]->ok()) {
        auto& _server_sock = zpt::globals::alloc<zpt::serversocketstream>(
          zpt::HTTP_SERVER_SOCKET(),
          static_cast<std::uint16_t>(static_cast<unsigned int>(_config["port"])));

        _boot.add_thread([=]() mutable -> void {
            auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
            zlog("Starting HTTP transport on port " << _config["port"], zpt::info);

            try {
                do {
                    auto _client = _server_sock->accept();
                    _client->transport("http");
                    _polling.listen_on(_client);
                } while (true);
            }
            catch (zpt::failed_expectation const& _e) {
            }
            zlog("Stopping HTTP transport on port " << _config["port"], zpt::info);
        });
    }
}

extern "C" auto
_zpt_unload_(zpt::plugin& _plugin) {
    auto& _config = _plugin->config();
    if (_config["port"]->ok()) {
        zpt::globals::get<zpt::serversocketstream>(zpt::HTTP_SERVER_SOCKET())->close();
        zpt::globals::dealloc<zpt::serversocketstream>(zpt::HTTP_SERVER_SOCKET());
    }
}
