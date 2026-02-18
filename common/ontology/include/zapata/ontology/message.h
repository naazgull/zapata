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
    basic_message() = default;
    basic_message(basic_message const& _req, bool);
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

class call_context {
  public:
    using ptr = std::shared_ptr<call_context>;

    call_context() = default;
    ~call_context() = default;

    auto state() const -> int;
    auto reply() const -> zpt::message;
    auto reply(zpt::message _to_update) -> call_context&;
    auto is_replied() const -> bool;
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
    json_message();
    json_message(basic_message const& _req, bool);
    virtual ~json_message() = default;

    auto performative() const -> zpt::performative override;
    auto status() const -> zpt::status override;
    auto uri() -> zpt::json& override;
    auto uri() const -> zpt::json const override;
    auto version() const -> std::string override;
    auto scheme() const -> std::string override;
    auto resource() const -> zpt::json const override;
    auto parameters() const -> zpt::json const override;
    auto headers() -> zpt::json& override;
    auto headers() const -> zpt::json const override;
    auto body() -> zpt::json& override;
    auto body() const -> zpt::json const override;
    auto keep_alive() const -> bool override;
    auto content_type() const -> std::string override;
    auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    auto from_stream(std::istream& _in) -> zpt::basic_message& override;
    auto performative(zpt::performative _performative) -> zpt::basic_message& override;
    auto status(zpt::status _status) -> zpt::basic_message& override;
    auto uri(std::string const& _uri) -> zpt::basic_message& override;
    auto version(std::string const& _version) -> zpt::basic_message& override;
    auto empty() const -> bool override;
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
