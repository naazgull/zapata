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
    PERSISTENT = 2,  ///< Transport maintains persistent connections
    PUB_SUB = 4,     ///< Transport follows pub/sub flow
    UPGRADED = 8     ///< Transport is upgraded from another transport
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

    /** @brief Checks if the transport has a specific capability.
     * @param _capability Capability flag to check (SYNCHRONOUS, PERSISTENT, etc.).
     * @return True if the transport has this capability. */
    virtual auto has_capability(std::uint64_t _capability) const -> bool = 0;
    /** @brief Creates a new request message for this transport.
     * @return Shared pointer to the new request message. */
    virtual auto make_request() const -> zpt::message = 0;
    /** @brief Creates a new reply message for this transport.
     * @param _with_allocator If true, uses the memory pool allocator.
     * @return Shared pointer to the new reply message. */
    virtual auto make_reply(bool _with_allocator = true) const -> zpt::message = 0;
    /** @brief Creates a reply message in response to a request.
     * @param _request The request message to reply to.
     * @return Shared pointer to the new reply message. */
    virtual auto make_reply(zpt::message _request) const -> zpt::message = 0;
    /** @brief Parses an incoming request from a stream.
     * @param _stream Input stream containing the serialized request.
     * @return Parsed request message. */
    virtual auto process_incoming_request(zpt::stream _stream) const -> zpt::message = 0;
    /** @brief Parses an incoming reply from a stream.
     * @param _stream Input stream containing the serialized reply.
     * @return Parsed reply message. */
    virtual auto process_incoming_reply(zpt::stream _stream) const -> zpt::message = 0;
    /** @brief Creates a copy of the given message with the transport's protocol and format.
     * @param _to_copy The message to copy.
     * @return The copied message. */
    virtual auto copy(zpt::message const& _to_copy) const -> zpt::message = 0;
    /**
     * @brief Retrieves from which transport this was upgraded.
     * @return Reference to the name of the original transport.
     */
    virtual auto upgraded_from() const -> std::string const&;
    /**
     * @brief Receives a message from a stream (request or reply based on context).
     * @param _stream The stream to receive from.
     * @return Received message.
     */
    virtual auto receive(zpt::stream _stream) const -> zpt::message final;
    /**
     * @brief Sends a message to a stream.
     * @param _stream The stream to send to.
     * @param _to_send The message to send.
     */
    virtual auto send(zpt::stream _stream, zpt::message _to_send) const -> void final;
    /**
     * @brief Publishes a message to a pub-sub topic.
     * @param _to_publish The message to publish.
     */
    virtual auto publish(zpt::message _to_publish) const -> void;
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

    /** @brief Constructs a network layer with the given configuration.
     * @param _global_config Global configuration object containing transport registry settings.
     * @return void (constructors implicitly initialize the object) */
    layer(zpt::json _global_config);
    /** @brief Destructor. */
    virtual ~layer() = default;

    /** @brief Registers a transport for a URI scheme.
     * @param _scheme URI scheme string (e.g., "http", "ws").
     * @param _transport Transport instance to register.
     * @return Reference to this layer. */
    auto add(std::string const& _scheme, zpt::transport _transport) -> layer&;
    /** @brief Gets the transport for a URI scheme.
     * @param _scheme URI scheme string.
     * @return Shared pointer to the registered transport, or null if not found. */
    auto get(std::string const& _scheme) const -> const zpt::transport;
    /** @brief Removes a transport registration.
     * @param _scheme URI scheme to remove.
     * @return Reference to this layer. */
    auto remove(std::string const& _scheme) -> layer&;
    /** @brief Removes all transport registrations.
     * @return Reference to this layer. */
    auto clear() -> layer&;

    /** @brief Deserializes content from a stream based on MIME type.
     * @param _io Input stream containing the serialized content.
     * @param _mime MIME type to deserialize as (default: "* / *" for auto-detect).
     * @return Parsed JSON object. */
    auto translate(std::istream& _io, std::string _mime = "*/*") const -> zpt::json;
    /** @brief Serializes content to a stream based on MIME type.
     * @param _io Output stream to write serialized content to.
     * @param _mime MIME type to serialize as.
     * @param _content JSON content to serialize.
     * @return Serialized string representation. */
    auto translate(std::ostream& _io, std::string _mime, zpt::json _content) const -> std::string;

    /** @brief Returns iterator to the beginning of registered transports.
     * @return Iterator to the first transport. */
    auto begin() const -> std::map<std::string, zpt::transport>::const_iterator;
    /** @brief Returns iterator to the end of registered transports.
     * @return Iterator past the last transport. */
    auto end() const -> std::map<std::string, zpt::transport>::const_iterator;

    /** @brief Resolves a URI to its appropriate transport.
     * @param _uri Full URI string (e.g., "http://localhost:8080/api").
     * @return Shared pointer to the transport that handles this URI. */
    auto resolve(std::string _uri) const -> zpt::transport;

  private:
    /** @brief Map of registered transports keyed by URI scheme. */
    std::map<std::string, zpt::transport> __underlying;
    /** @brief Map of MIME types to their serialize/deserialize function pairs. */
    std::map<std::string, std::tuple<translate_from_func, translate_to_func>> __content_providers;
    /** @brief Global configuration passed at construction. */
    zpt::json __configuration;

    /** @brief Registers a MIME type with its deserialization and serialization callbacks.
     * @param _mime MIME type string (e.g., "application/json", "text/plain").
     * @param _callback_from Deserialization callback function.
     * @param _callback_to Serialization callback function.
     * @return Reference to this layer. */
    auto add_content_provider(std::string const& _mime,
                              translate_from_func _callback_from,
                              translate_to_func _callback_to) -> layer&;
    /** @brief Default deserializer: tries JSON first, falls back to raw text.
     * @param _io Input stream to read from.
     * @return Parsed JSON object. */
    static auto translate_from_default(std::istream& _io) -> zpt::json;
    /** @brief Default serializer: tries JSON first, falls back to raw text.
     * @param _io Output stream to write to.
     * @param _content JSON content to serialize.
     * @return Serialized string representation. */
    static auto translate_to_default(std::ostream& _io, zpt::json _content) -> std::string;
    /** @brief Deserializes JSON from the input stream.
     * @param _io Input stream containing JSON data.
     * @return Parsed JSON object. */
    static auto translate_from_json(std::istream& _io) -> zpt::json;
    /** @brief Serializes JSON to the output stream.
     * @param _io Output stream to write serialized JSON to.
     * @param _content JSON content to serialize.
     * @return Serialized string representation. */
    static auto translate_to_json(std::ostream& _io, zpt::json _content) -> std::string;
    /** @brief Deserializes raw text from the input stream as a string.
     * @param _io Input stream containing raw text.
     * @return JSON string value containing the raw text. */
    static auto translate_from_raw(std::istream& _io) -> zpt::json;
    /** @brief Serializes raw text to the output stream.
     * @param _io Output stream to write raw text to.
     * @param _content JSON content to serialize as raw text.
     * @return Serialized string representation. */
    static auto translate_to_raw(std::ostream& _io, zpt::json _content) -> std::string;
    /** @brief Deserializes XML content from the input stream as a string.
     * @param _io Input stream containing XML data.
     * @return JSON string value containing the raw XML text. */
    static auto translate_from_xml(std::istream& _io) -> zpt::json;
    /** @brief Serializes XML content to the output stream.
     * @param _io Output stream to write XML data to.
     * @param _content JSON content to serialize as XML text.
     * @return Serialized string representation. */
    static auto translate_to_xml(std::ostream& _io, zpt::json _content) -> std::string;
};

/** @brief Determines the content type from a message's headers.
 * @param _message Message to extract content type from.
 * @return Content type string (e.g., "application/json"). */
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
