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

/**
 * @file plugin.cpp
 * @brief WebSocket transport plugin registration.
 *
 * Registers the "ws" transport with the transport layer on load,
 * and removes it on unload.
 *
 * @see zpt::TRANSPORT_LAYER
 */

#include <iostream>
#include <zapata/net/socket.h>
#include <zapata/net/websocket.h>
#include <zapata/startup.h>

/** @brief Flag tracking whether the plugin has been unloaded. */
static zpt::padded_atomic<bool> _has_exited{ false };

/** @brief Plugin entry point: registers the WebSocket transport ("ws" scheme).
 * @param _plugin Plugin handle (unused). */
extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zpt::TRANSPORT_LAYER() //
      .add("ws", zpt::make_transport<zpt::net::transport::websocket>());
    zlog("Loaded WebSocket connection upgrade support", zpt::info);
}

/** @brief Plugin exit point: unregisters the WebSocket transport.
 * @param _plugin Plugin handle (unused). */
extern "C" auto _zpt_unload_(zpt::plugin&) {
    zlog("Unloading WebSocket connection upgrade support", zpt::info);
    zpt::TRANSPORT_LAYER().remove("ws");
}
