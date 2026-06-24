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
 * @file transport.h
 * @brief Abstract transport interface and network layer.
 *
 * Provides the base class for all transport implementations (HTTP, TCP,
 * WebSocket, etc.) and a registry for managing available transports.
 *
 * @par Transport Model
 * Each transport handles a specific protocol's wire format:
 * - Creating request/reply messages
 * - Serializing messages to streams
 * - Parsing messages from streams
 *
 * @par Content Negotiation
 * The network layer handles MIME-based content negotiation,
 * automatically translating between JSON, XML, and raw formats.
 *
 * @see zpt::basic_transport
 * @see zpt::network::layer
 */

#pragma once

#include <zapata/globals.h>
#include <zapata/json.h>
#include <zapata/ontology.h>
#include <zapata/streams.h>

namespace zpt {

/**
 * @brief Transport capability flags.
 */
enum transport_capability {
    SYNCHRONOUS = 1, ///< Transport supports request-response pattern
    PERSISTENT = 2   ///< Transport maintains persistent connections
};

/**
 * @brief Abstract base class for protocol transports.
 *
 * Defines the interface for all transport implementations. Each transport
 * handles message creation, serialization, and parsing for a specific
 * protocol (HTTP, WebSocket, TCP, etc.).
 *
 * @par Implementing a Transport
 * @code
 * class my_transport : public zpt::basic_transport {
 *     auto has_capability(std::uint64_t cap) const -> bool override {
 *         return cap & zpt::SYNCHRONOUS;
 *     }
 *
 *     auto make_request() const -> zpt::message override {
 *         return zpt::make_message<zpt::json_message>();
 *     }
 *
 *     auto process_incoming_request(zpt::stream s) const -> zpt::message override {
 *         auto msg = make_request();
 *         // Parse from stream...
 *         return msg;
 *     }
 *     // ... other overrides
 * };
 * @endcode
 */
class basic_transport {
  public:
    /** @brief Default constructor. */
    basic_transport() = default;
    /** @brief Destructor. */
    virtual ~basic_transport() = default;

    /** @brief Checks if the transport has a specific capability. */
    virtual auto has_capability(std::uint64_t _capability) const -> bool = 0;
    /** @brief Creates a new request message for this transport. */
    virtual auto make_request() const -> zpt::message = 0;
    /** @brief Creates a new reply message for this transport. */
    virtual auto make_reply(bool _with_allocator = true) const -> zpt::message = 0;
    /** @brief Creates a reply message in response to a request. */
    virtual auto make_reply(zpt::message _request) const -> zpt::message = 0;
    /** @brief Parses an incoming request from a stream. */
    virtual auto process_incoming_request(zpt::stream _stream) const -> zpt::message = 0;
    /** @brief Parses an incoming reply from a stream. */
    virtual auto process_incoming_reply(zpt::stream _stream) const -> zpt::message = 0;
    /** @brief Receives a message from a stream (request or reply based on context). */
    virtual auto receive(zpt::stream _stream) const -> zpt::message final;
    /** @brief Sends a message to a stream. */
    virtual auto send(zpt::stream _stream, zpt::message _to_send) const -> void final;
};

/** @brief Shared pointer to a transport. */
using transport = std::shared_ptr<basic_transport>;

namespace network {

/**
 * @brief Transport registry and content negotiation layer.
 *
 * Manages available transports by URI scheme and provides MIME-based
 * content translation between different formats (JSON, XML, raw).
 *
 * @par Example
 * @code
 * auto& layer = zpt::TRANSPORT_LAYER(config);
 *
 * // Register transports
 * layer.add("http", zpt::make_transport<zpt::net::transport::http>());
 * layer.add("ws", zpt::make_transport<zpt::net::transport::websocket>());
 *
 * // Resolve transport by URI
 * auto transport = layer.resolve("http://localhost:8080/api");
 *
 * // Content translation
 * auto json = layer.translate(stream, "application/json");
 * @endcode
 */
class layer {
  public:
    /** @brief Function type for deserializing content from a stream. */
    using translate_from_func = std::function<zpt::json(std::istream&)>;
    /** @brief Function type for serializing content to a stream. */
    using translate_to_func = std::function<std::string(std::ostream&, zpt::json)>;

    /** @brief Constructs a network layer with the given configuration. */
    layer(zpt::json _global_config);
    /** @brief Destructor. */
    virtual ~layer() = default;

    /** @brief Registers a transport for a URI scheme. */
    auto add(std::string const& _scheme, zpt::transport _transport) -> layer&;
    /** @brief Gets the transport for a URI scheme. */
    auto get(std::string const& _scheme) const -> const zpt::transport;
    /** @brief Removes a transport registration. */
    auto remove(std::string const& _scheme) -> layer&;
    /** @brief Removes all transport registrations. */
    auto clear() -> layer&;

    /** @brief Deserializes content from a stream based on MIME type. */
    auto translate(std::istream& _io, std::string _mime = "*/*") const -> zpt::json;
    /** @brief Serializes content to a stream based on MIME type. */
    auto translate(std::ostream& _io, std::string _mime, zpt::json _content) const -> std::string;

    /** @brief Returns iterator to the beginning of registered transports. */
    auto begin() const -> std::map<std::string, zpt::transport>::const_iterator;
    /** @brief Returns iterator to the end of registered transports. */
    auto end() const -> std::map<std::string, zpt::transport>::const_iterator;

    /** @brief Resolves a URI to its appropriate transport. */
    auto resolve(std::string _uri) const -> zpt::transport;

  private:
    std::map<std::string, zpt::transport> __underlying;
    std::map<std::string, std::tuple<translate_from_func, translate_to_func>> __content_providers;
    zpt::json __configuration;

    auto add_content_provider(std::string const& _mime,
                              translate_from_func _callback_from,
                              translate_to_func _callback_to) -> layer&;
    static auto translate_from_default(std::istream& _io) -> zpt::json;
    static auto translate_to_default(std::ostream& _io, zpt::json _content) -> std::string;
    static auto translate_from_json(std::istream& _io) -> zpt::json;
    static auto translate_to_json(std::ostream& _io, zpt::json _content) -> std::string;
    static auto translate_from_raw(std::istream& _io) -> zpt::json;
    static auto translate_to_raw(std::ostream& _io, zpt::json _content) -> std::string;
    static auto translate_from_xml(std::istream& _io) -> zpt::json;
    static auto translate_to_xml(std::ostream& _io, zpt::json _content) -> std::string;
};

/** @brief Determines the content type from a message's headers. */
auto resolve_content_type(zpt::message _message) -> std::string;
} // namespace network

/**
 * @brief Returns the global transport layer instance.
 * @param _config Optional configuration (used only on first call).
 * @return Reference to the global network layer.
 */
auto TRANSPORT_LAYER(zpt::json _config = nullptr) -> zpt::network::layer&;

/**
 * @brief Creates a transport instance using the memory pool allocator.
 * @tparam T Transport type to create.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to T's constructor.
 * @return Shared pointer to the new transport.
 */
template<typename T, typename... Args>
auto make_transport(Args... _args) -> zpt::transport;
} // namespace zpt

template<typename T>
auto zpt::json_message::operator<<(T _to_add) -> zpt::json_message& {
    if (!this->__underlying("body")->ok()) { this->__underlying["body"] = zpt::json::object(); }
    this->__underlying["body"] << _to_add;
    return (*this);
}

template<typename T, typename... Args>
auto zpt::make_transport(Args... _args) -> zpt::transport {
    return std::allocate_shared<T>(zpt::allocator<T>{ zpt::MEM_POOL() },
                                   std::forward<Args>(_args)...);
}
