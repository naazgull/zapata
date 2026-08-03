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
 * @file UPnPFactory.h
 * @brief Factory for creating UPnP channel instances.
 */

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>
#include <zapata/upnp/UPnP.h>

namespace zpt {

/** @brief Factory for creating and managing UPnP socket channels. */
class UPnPFactory : public zpt::ChannelFactory {
  public:
    /** @brief Constructs the UPnP channel factory. */
    UPnPFactory();
    /** @brief Destroys the UPnP channel factory. */
    virtual ~UPnPFactory();
    /** @brief Creates a new UPnP socket channel.
     @param _options Configuration JSON for the channel.
     @return A new UPnP socket.
     */
    virtual auto produce(zpt::json _options) -> zpt::socket;
    /** @brief Checks whether a channel of the given type is reusable.
     @param _type The channel type identifier.
     @return True if channels of this type can be reused.
     */
    virtual auto is_reusable(std::string const& _type) -> bool;
    /** @brief Cleans up and removes a socket from the channel pool.
     @param _socket The socket to clean up.
     @return True if the socket was successfully cleaned.
     */
    virtual auto clean(zpt::socket _socket) -> bool;

  private:
    /** @brief Map of registered channel instances keyed by type. */
    std::map<std::string, zpt::socket> __channels;
};
} // namespace zpt
