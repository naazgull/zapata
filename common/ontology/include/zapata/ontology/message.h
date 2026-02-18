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
 * @file message.h
 * @brief Protocol-agnostic message interfaces for transport abstraction.
 *
 * Defines abstract message types that can be serialized to/from different
 * transport protocols (HTTP, WebSocket, etc.). The interface mirrors HTTP
 * semantics with performatives (methods), URIs, headers, and bodies.
 *
 * @see zpt::basic_message
 * @see zpt::json_message
 */

#pragma once

#include <zapata/allocator.h>
#include <zapata/globals.h>
#include <zapata/json.h>
#include <zapata/ontology/performative.h>

namespace zpt {

/**
 * @brief Abstract base class for protocol-agnostic messages.
 *
 * Defines the interface for request/response messages that can be
 * transported over various protocols. Implementations include
 * `zpt::http::basic_request`, `zpt::http::basic_reply`, and `zpt::json_message`.
 *
 * @par Message Components
 * - Performative: The HTTP-like method (GET, POST, etc.)
 * - URI: Target resource with path and parameters
 * - Headers: Key-value metadata
 * - Body: Message payload (typically JSON)
 * - Status: Response status code
 */
class basic_message {
  public:
    /** @brief Default constructor. */
    basic_message() = default;
    /** @brief Constructs a reply message from a request. */
    basic_message(basic_message const& _req, bool);
    /** @brief Destructor. */
    virtual ~basic_message() = default;

    /** @brief Returns the request method (GET, POST, etc.). */
    virtual auto performative() const -> zpt::performative = 0;
    /** @brief Returns the response status code. */
    virtual auto status() const -> zpt::status = 0;
    /** @brief Returns mutable reference to URI. */
    virtual auto uri() -> zpt::json& = 0;
    /** @brief Returns the URI (const). */
    virtual auto uri() const -> zpt::json const = 0;
    /** @brief Returns protocol version (e.g., "1.1"). */
    virtual auto version() const -> std::string = 0;
    /** @brief Returns URI scheme (http, https, ws, etc.). */
    virtual auto scheme() const -> std::string = 0;
    /** @brief Returns the resource path. */
    virtual auto resource() const -> zpt::json const = 0;
    /** @brief Returns query parameters. */
    virtual auto parameters() const -> zpt::json const = 0;
    /** @brief Returns mutable reference to headers. */
    virtual auto headers() -> zpt::json& = 0;
    /** @brief Returns headers (const). */
    virtual auto headers() const -> zpt::json const = 0;
    /** @brief Returns mutable reference to body. */
    virtual auto body() -> zpt::json& = 0;
    /** @brief Returns body (const). */
    virtual auto body() const -> zpt::json const = 0;
    /** @brief Returns true if connection should persist. */
    virtual auto keep_alive() const -> bool = 0;
    /** @brief Returns Content-Type header value. */
    virtual auto content_type() const -> std::string = 0;
    /** @brief Sets the request method. */
    virtual auto performative(zpt::performative _performative) -> basic_message& = 0;
    /** @brief Sets the response status code. */
    virtual auto status(zpt::status _status) -> basic_message& = 0;
    /** @brief Sets the URI from a string. */
    virtual auto uri(std::string const& _uri) -> basic_message& = 0;
    /** @brief Sets the protocol version. */
    virtual auto version(std::string const& _version) -> basic_message& = 0;
    /** @brief Serializes message to output stream. */
    virtual auto to_stream(std::ostream& _out) const -> basic_message const& = 0;
    /** @brief Deserializes message from input stream. */
    virtual auto from_stream(std::istream& _in) -> basic_message& = 0;
    /** @brief Returns true if message is empty/uninitialized. */
    virtual auto empty() const -> bool = 0;

    friend auto operator<<(std::ostream& _out, zpt::basic_message const& _in) -> std::ostream& {
        _in.to_stream(_out);
        return _out;
    }

    friend auto operator>>(std::istream& _in, zpt::basic_message& _out) -> std::istream& {
        _out.from_stream(_in);
        return _in;
    }
};
/** @brief Shared pointer type for messages. */
using message = std::shared_ptr<basic_message>;

constexpr int CALL_STATE_UNPROCESSED = 0;
constexpr int CALL_STATE_SENT = 1;
constexpr int CALL_STATE_SUCCESS_REPLY = 2;
constexpr int CALL_STATE_FAILURE_REPLY = 3;

/**
 * @brief Context for tracking an outbound call and its reply.
 *
 * Holds the state of an asynchronous call (unprocessed, sent, success, failure)
 * and the reply message once received. Used with zpt::events::call to
 * correlate requests with responses.
 */
class call_context {
  public:
    using ptr = std::shared_ptr<call_context>;

    /** @brief Default constructor. Initial state is CALL_STATE_SENT. */
    call_context() = default;
    /** @brief Destructor. */
    ~call_context() = default;

    /** @brief Returns the current call state (UNPROCESSED, SENT, SUCCESS_REPLY, FAILURE_REPLY). */
    auto state() const -> int;
    /** @brief Returns the reply message (may be null if not yet replied). */
    auto reply() const -> zpt::message;
    /** @brief Sets the reply message and updates the call state. */
    auto reply(zpt::message _to_update) -> call_context&;
    /** @brief Returns true if a reply has been received. */
    auto is_replied() const -> bool;
    /** @brief Returns true if the reply indicates a failure. */
    auto has_error() const -> bool;

  private:
    zpt::padded_atomic<int> __state{ zpt::CALL_STATE_SENT };
    zpt::message __reply{ nullptr };
};

/**
 * @brief JSON-based message implementation.
 *
 * Stores message data as JSON internally, suitable for internal
 * message passing and JSON-based protocols.
 */
class json_message : public basic_message {
  public:
    /** @brief Constructs an empty JSON message. */
    json_message();
    /** @brief Constructs a JSON reply from an existing request. */
    json_message(basic_message const& _req, bool);
    /** @brief Destructor. */
    virtual ~json_message() = default;

    /** @brief Returns the request method. */
    auto performative() const -> zpt::performative override;
    /** @brief Returns the response status code. */
    auto status() const -> zpt::status override;
    /** @brief Returns mutable reference to URI. */
    auto uri() -> zpt::json& override;
    /** @brief Returns the URI (const). */
    auto uri() const -> zpt::json const override;
    /** @brief Returns protocol version. */
    auto version() const -> std::string override;
    /** @brief Returns URI scheme. */
    auto scheme() const -> std::string override;
    /** @brief Returns the resource path. */
    auto resource() const -> zpt::json const override;
    /** @brief Returns query parameters. */
    auto parameters() const -> zpt::json const override;
    /** @brief Returns mutable reference to headers. */
    auto headers() -> zpt::json& override;
    /** @brief Returns headers (const). */
    auto headers() const -> zpt::json const override;
    /** @brief Returns mutable reference to body. */
    auto body() -> zpt::json& override;
    /** @brief Returns body (const). */
    auto body() const -> zpt::json const override;
    /** @brief Returns true if connection should persist. */
    auto keep_alive() const -> bool override;
    /** @brief Returns Content-Type header value. */
    auto content_type() const -> std::string override;
    /** @brief Serializes message to output stream as JSON. */
    auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    /** @brief Deserializes message from input stream. */
    auto from_stream(std::istream& _in) -> zpt::basic_message& override;
    /** @brief Sets the request method. */
    auto performative(zpt::performative _performative) -> zpt::basic_message& override;
    /** @brief Sets the response status code. */
    auto status(zpt::status _status) -> zpt::basic_message& override;
    /** @brief Sets the URI from a string. */
    auto uri(std::string const& _uri) -> zpt::basic_message& override;
    /** @brief Sets the protocol version. */
    auto version(std::string const& _version) -> zpt::basic_message& override;
    /** @brief Returns true if message is empty/uninitialized. */
    auto empty() const -> bool override;
    /** @brief Appends a value to the underlying JSON. */
    template<typename T>
    auto operator<<(T _to_add) -> zpt::json_message&;

  private:
    zpt::json __underlying;
};

/**
 * @brief Creates a message using standard allocator.
 * @tparam T Message type (e.g., json_message, http::basic_request).
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to T's constructor.
 * @return Shared pointer to the message.
 */
template<typename T, typename... Args>
auto make_message(Args... _args) -> zpt::message;

/**
 * @brief Creates a message using the memory pool allocator.
 * @tparam T Message type.
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to T's constructor.
 * @return Shared pointer to the message.
 */
template<typename T, typename... Args>
auto allocate_message(Args... _args) -> zpt::message;

/**
 * @brief Casts a message to a specific derived type.
 * @tparam T Target message type.
 * @param _rhs Message to cast.
 * @return Reference to the message as type T.
 */
template<typename T>
auto message_cast(zpt::message _rhs) -> T& {
    return static_cast<T&>(*_rhs);
}
} // namespace zpt

auto operator<<(std::ostream& _out, zpt::message _in) -> std::ostream&;
auto operator>>(std::istream& _in, zpt::message _out) -> std::istream&;

template<typename T, typename... Args>
auto zpt::make_message(Args... _args) -> zpt::message {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}

template<typename T, typename... Args>
auto zpt::allocate_message(Args... _args) -> zpt::message {
    return std::allocate_shared<T>(zpt::allocator<T>{ zpt::MEM_POOL() },
                                   std::forward<Args>(_args)...);
}
