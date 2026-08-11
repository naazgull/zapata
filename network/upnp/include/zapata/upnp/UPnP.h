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
 * @file UPnP.h
 * @brief UPnP/SSDP protocol implementation for service discovery.
 *
 * Provides multicast-based service discovery using the Simple Service
 * Discovery Protocol (SSDP) portion of UPnP.
 */

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <zapata/events.h>
#include <zapata/json.h>

#define UPNP_RAW -7

namespace zpt {

class UPnP;
class UPnPPtr;

/** @brief Shared pointer wrapper for UPnP instances. */
class UPnPPtr : public std::shared_ptr<zpt::UPnP> {
  public:
    /**
     * @brief Constructs an UPnPPtr with default options.
     * @return void (constructors implicitly initialize the object).
     */
    UPnPPtr();
    /** @brief Constructs an UPnPPtr with the given options.
     @param _options Configuration JSON for the UPnP channel.
     */
    UPnPPtr(zpt::json _options);
    /**
     * @brief Destroys the UPnPPtr.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~UPnPPtr();
};

namespace upnp {
typedef zpt::UPnPPtr broker;
}

/**
 * @brief UPnP/SSDP channel for multicast service discovery.
 *
 * Implements M-SEARCH and NOTIFY operations for discovering and
 * announcing services on the local network via multicast UDP.
 */
class UPnP : public zpt::Channel {
  public:
    /** @brief Constructs a UPnP/SSDP channel with the given configuration.
     @param _options Configuration JSON for the UPnP channel.
     */
    UPnP(zpt::json _options);
    /**
     * @brief Destroys the UPnP channel and cleans up resources.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~UPnP();

    /** @brief Sends an SSDP NOTIFY message for service announcement.
     @param _search The notification type (NNT header value).
     @param _location The location URL where service details can be retrieved.
     */
    virtual auto notify(std::string const& _search, std::string const& _location) -> void;
    /** @brief Sends an SSDP M-SEARCH multicast request to discover services.
     @param _search The search target (ST header value).
     */
    virtual auto search(std::string const& _search) -> void;
    /** @brief Listens for incoming SSDP messages on the multicast socket.
     @return The parsed HTTP request from the incoming SSDP packet.
     */
    virtual auto listen() -> zpt::http::req;

    /** @brief Receives the next available message from the socket.
     @return A JSON object containing the message data.
     */
    virtual auto recv() -> zpt::json;
    /** @brief Sends a message with the given performative, resource, and payload.
     @param _performative The message performative (e.g., REQUEST, NOTIFICATION).
     @param _resource The target resource path or search type.
     @param _payload The JSON payload to send.
     @return A JSON object containing the response.
     */
    virtual auto send(zpt::performative _performative,
                      std::string const& _resource,
                      zpt::json _payload) -> zpt::json;
    /** @brief Sends a message using a pre-built JSON envelope.
     @param _envelope The complete JSON message envelope.
     @return A JSON object containing the response.
     */
    virtual auto send(zpt::json _envelope) -> zpt::json;

    /**
     * @brief Returns the unique identifier for this UPnP channel.
     * @return Unique channel identifier string.
     */
    virtual auto id() -> std::string;
    /**
     * @brief Returns the underlying socket stream for the send connection.
     * @return Socket stream pointer.
     */
    virtual auto underlying() -> zpt::socketstream_ptr;
    /**
     * @brief Returns the multicast ZMQ socket used for UPnP communication.
     * @return ZMQ socket pointer.
     */
    virtual auto socket() -> zmq::socket_ptr;
    /**
     * @brief Returns the input ZMQ socket.
     * @return Input ZMQ socket pointer.
     */
    virtual auto in() -> zmq::socket_ptr;
    /**
     * @brief Returns the output ZMQ socket.
     * @return Output ZMQ socket pointer.
     */
    virtual auto out() -> zmq::socket_ptr;
    /**
     * @brief Returns the file descriptor of the underlying socket.
     * @return File descriptor integer.
     */
    virtual auto fd() -> int;
    /**
     * @brief Returns a reference to the input mutex for thread synchronization.
     * @return Reference to the input mutex.
     */
    virtual auto in_mtx() -> std::mutex&;
    /**
     * @brief Returns a reference to the output mutex for thread synchronization.
     * @return Reference to the output mutex.
     */
    virtual auto out_mtx() -> std::mutex&;
    /**
     * @brief Returns the short integer type identifier for this channel.
     * @return Channel type identifier.
     */
    virtual auto type() -> short int;
    /**
     * @brief Returns the protocol string for this channel.
     * @return Protocol string (e.g., "upnp").
     */
    virtual auto protocol() -> std::string;
    /**
     * @brief Closes the UPnP channel and releases associated resources.
     * @return void.
     */
    virtual auto close() -> void;
    /**
     * @brief Returns whether there is data available to read.
     * @return True if data is available.
     */
    virtual auto available() -> bool;
    /**
     * @brief Returns whether this channel instance can be reused.
     * @return True if the channel can be reused for subsequent requests.
     */
    virtual auto is_reusable() -> bool;

  private:
    /** @brief Mutex protecting the underlying send socket. */
    std::mutex __mtx_underlying;
    /** @brief Mutex protecting send operations. */
    std::mutex __mtx_send;
    /** @brief The underlying socket stream for outgoing send operations. */
    zpt::socketstream_ptr __underlying;
    /** @brief The socket stream used for sending. */
    zpt::socketstream_ptr __send;
};
} // namespace zpt
