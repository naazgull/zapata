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
    /** @brief Retrieves a clone of this message.
     * @return Shared pointer to cloned message. */
    virtual auto clone() const -> std::shared_ptr<basic_message> = 0;
    /** @brief Returns the request method (GET, POST, etc.).
     * @return Performative enum value. */
    virtual auto performative() const -> zpt::performative = 0;
    /** @brief Returns the response status code.
     * @return Status code (200, 404, etc.). */
    virtual auto status() const -> zpt::status = 0;
    /** @brief Returns mutable reference to URI.
     * @return Non-const reference to URI JSON object. */
    virtual auto uri() -> zpt::json& = 0;
    /** @brief Returns the URI (const).
     * @return Const reference to URI JSON object. */
    virtual auto uri() const -> zpt::json const = 0;
    /** @brief Returns protocol version (e.g., "1.1").
     * @return Protocol version string. */
    virtual auto version() const -> std::string = 0;
    /** @brief Returns URI scheme (http, https, ws, etc.).
     * @return Scheme string. */
    virtual auto scheme() const -> std::string = 0;
    /** @brief Returns the resource path.
     * @return JSON object representing the resource path. */
    virtual auto resource() const -> zpt::json const = 0;
    /** @brief Returns query parameters.
     * @return JSON object of query parameters. */
    virtual auto parameters() const -> zpt::json const = 0;
    /** @brief Returns mutable reference to headers.
     * @return Non-const reference to headers JSON object. */
    virtual auto headers() -> zpt::json& = 0;
    /** @brief Returns headers (const).
     * @return Const reference to headers JSON object. */
    virtual auto headers() const -> zpt::json const = 0;
    /**
     * @brief Adds or updates a header.
     * @param _name Header name.
     * @param _value Header value.
     * @return Reference for chaining.
     */
    virtual auto header(std::string const& _name, std::string const& _value)
      -> zpt::basic_message& = 0;
    /** @brief Returns mutable reference to body.
     * @return Non-const reference to body JSON object. */
    virtual auto body() -> zpt::json& = 0;
    /** @brief Returns body (const).
     * @return Const reference to body JSON object. */
    virtual auto body() const -> zpt::json const = 0;
    /** @brief Returns true if connection should persist.
     * @return True if keep-alive is enabled. */
    virtual auto keep_alive() const -> bool = 0;
    /** @brief Returns Content-Type header value.
     * @return Content-Type string (e.g., "application/json"). */
    virtual auto content_type() const -> std::string = 0;
    /** @brief Sets the request method.
     * @param _performative New performative value.
     * @return Reference to this message. */
    virtual auto performative(zpt::performative _performative) -> basic_message& = 0;
    /** @brief Sets the response status code.
     * @param _status New status code.
     * @return Reference to this message. */
    virtual auto status(zpt::status _status) -> basic_message& = 0;
    /** @brief Sets the URI from a string.
     * @param _uri URI string to parse.
     * @return Reference to this message. */
    virtual auto uri(std::string const& _uri) -> basic_message& = 0;
    /** @brief Sets the protocol version.
     * @param _version Version string (e.g., "1.1").
     * @return Reference to this message. */
    virtual auto version(std::string const& _version) -> basic_message& = 0;
    /** @brief Serializes message to output stream.
     * @param _out Output stream to serialize to.
     * @return Const reference to this message. */
    virtual auto to_stream(std::ostream& _out) const -> basic_message const& = 0;
    /** @brief Deserializes message from input stream.
     * @param _in Input stream to deserialize from.
     * @return Reference to this message. */
    virtual auto from_stream(std::istream& _in) -> basic_message& = 0;
    /** @brief Returns true if message is empty/uninitialized.
     * @return True if message has no content. */
    virtual auto empty() const -> bool = 0;
    /** @brief Tries to acquire the ownership of the reply to this message.
     * @return True if the ownership was acquired, false otherwise. */
    virtual auto acquire_reply() -> bool final;
    /** @brief Sets the number of processing events acting upon this message.
     * @return Reference to this message. */
    virtual auto set_processors(size_t _n_processors) -> basic_message& final;
    /** @brief Decrements the number of events processing this message.
     * @return The number of processors remaining. */
    virtual auto finish_processor() -> size_t final;
    /** @brief Retrieves a copy of this message of the given template type.
     * @return Shared pointer to cloned message. */
    template<typename T>
    auto copy() const -> std::shared_ptr<basic_message>;
    /**
     * @brief Serializes the message to the given output stream.
     * @param _out Output stream.
     * @param _in Message to serialize.
     * @return Reference to the output stream.
     */
    friend auto operator<<(std::ostream& _out, zpt::basic_message const& _in) -> std::ostream& {
        _in.to_stream(_out);
        return _out;
    }
    /**
     * @brief Deserializes a message from the given input stream.
     * @param _in Input stream.
     * @param _out Message to populate.
     * @return Reference to the input stream.
     */
    friend auto operator>>(std::istream& _in, zpt::basic_message& _out) -> std::istream& {
        _out.from_stream(_in);
        return _in;
    }

  private:
    std::atomic<bool> __reply_acquired{ false };
    std::atomic<unsigned int> __active_processors{ 0 };
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

    /** @brief Returns the current call state (UNPROCESSED, SENT, SUCCESS_REPLY, FAILURE_REPLY).
     * @return State integer value. */
    auto state() const -> int;
    /** @brief Returns the reply message (may be null if not yet replied).
     * @return Reply message, or null if no reply received. */
    auto reply() const -> zpt::message;
    /** @brief Sets the reply message and updates the call state.
     * @param _to_update Reply message to set.
     * @return Reference to this context. */
    auto reply(zpt::message _to_update) -> call_context&;
    /** @brief Returns true if a reply has been received.
     * @return True if reply is available. */
    auto is_replied() const -> bool;
    /** @brief Returns true if the reply indicates a failure.
     * @return True if reply status indicates error. */
    auto has_error() const -> bool;

  private:
    /** @brief Current call state (UNPROCESSED, SENT, SUCCESS_REPLY, FAILURE_REPLY). */
    zpt::padded_atomic<int> __state{ zpt::CALL_STATE_SENT };
    /** @brief The reply message, set when a response is received. */
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
    /**
     * @brief Constructs an empty JSON message.
     * @return void (constructors implicitly initialize the object).
     */
    json_message();
    /**
     * @brief Populates this message with the given JSON.
     * @param _other JSON object to copy into this message.
     */
    json_message(zpt::json const& _other);
    /**
     * @brief Constructs a JSON reply from an existing request.
     * @param _req Request message to reply to.
     * @param _is_reply Flag indicating this is a reply.
     */
    json_message(zpt::message _req, bool);
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~json_message() = default;

    /**
     * @brief Retrieves a clone of this message.
     * @return Cloned message.
     */
    virtual auto clone() const -> zpt::message override;
    /**
     * @brief Returns the request method.
     * @return Performative enum value.
     */
    virtual auto performative() const -> zpt::performative override;
    /**
     * @brief Returns the response status code.
     * @return Status code value.
     */
    virtual auto status() const -> zpt::status override;
    /**
     * @brief Returns mutable reference to URI.
     * @return Non-const reference to URI JSON object.
     */
    virtual auto uri() -> zpt::json& override;
    /**
     * @brief Returns the URI (const).
     * @return Const reference to URI JSON object.
     */
    virtual auto uri() const -> zpt::json const override;
    /**
     * @brief Returns protocol version.
     * @return Protocol version string.
     */
    virtual auto version() const -> std::string override;
    /**
     * @brief Returns URI scheme.
     * @return Scheme string (e.g., "http", "https").
     */
    virtual auto scheme() const -> std::string override;
    /**
     * @brief Returns the resource path.
     * @return Resource path JSON object.
     */
    virtual auto resource() const -> zpt::json const override;
    /**
     * @brief Returns query parameters.
     * @return Query parameters JSON object.
     */
    virtual auto parameters() const -> zpt::json const override;
    /**
     * @brief Returns mutable reference to headers.
     * @return Non-const reference to headers JSON object.
     */
    virtual auto headers() -> zpt::json& override;
    /**
     * @brief Returns headers (const).
     * @return Const reference to headers JSON object.
     */
    virtual auto headers() const -> zpt::json const override;
    /** @brief Adds or updates a header.
     * @param _name Header name.
     * @param _value Header value.
     * @return Reference to the base message interface. */
    virtual auto header(std::string const& _name, std::string const& _value)
      -> zpt::basic_message& override;
    /** @brief Returns mutable reference to body.
     * @return Non-const reference to body JSON object. */
    virtual auto body() -> zpt::json& override;
    /** @brief Returns body (const).
     * @return Const reference to body JSON object. */
    virtual auto body() const -> zpt::json const override;
    /** @brief Returns true if connection should persist.
     * @return True if keep-alive is enabled. */
    virtual auto keep_alive() const -> bool override;
    /** @brief Returns Content-Type header value.
     * @return Content-Type string. */
    virtual auto content_type() const -> std::string override;
    /** @brief Serializes message to output stream as JSON.
     * @param _out Output stream.
     * @return Const reference to this message. */
    virtual auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    /** @brief Deserializes message from input stream.
     * @param _in Input stream to read from.
     * @return Reference to this message. */
    virtual auto from_stream(std::istream& _in) -> zpt::basic_message& override;
    /** @brief Sets the request method.
     * @param _performative New performative value.
     * @return Reference to the base message interface. */
    virtual auto performative(zpt::performative _performative) -> zpt::basic_message& override;
    /** @brief Sets the response status code.
     * @param _status New status code.
     * @return Reference to the base message interface. */
    virtual auto status(zpt::status _status) -> zpt::basic_message& override;
    /** @brief Sets the URI from a string.
     * @param _uri URI string.
     * @return Reference to the base message interface. */
    virtual auto uri(std::string const& _uri) -> zpt::basic_message& override;
    /** @brief Sets the protocol version.
     * @param _version Version string.
     * @return Reference to the base message interface. */
    virtual auto version(std::string const& _version) -> zpt::basic_message& override;
    /** @brief Returns true if message is empty/uninitialized.
     * @return True if message has no content. */
    virtual auto empty() const -> bool override;
    /** @brief Appends a value to the underlying JSON.
     * @tparam T Value type to append.
     * @param _to_add Value to append.
     * @return Reference to this message. */
    template<typename T>
    auto operator<<(T _to_add) -> zpt::json_message&;

  protected:
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
auto uri_to_string(zpt::json const& _uri) -> std::string;
} // namespace zpt

auto operator<<(std::ostream& _out, zpt::message _in) -> std::ostream&;
auto operator>>(std::istream& _in, zpt::message _out) -> std::istream&;

template<typename T>
auto zpt::basic_message::copy() const -> std::shared_ptr<basic_message> {
    auto _copy = zpt::make_message<T>();

    _copy->headers() = this->headers()->clone();
    _copy->body() = this->body()->clone();
    _copy //
      ->performative(this->performative())
      .uri(zpt::uri_to_string(this->uri()))
      .version(this->version());

    if (this->performative() == zpt::Reply) { _copy->status(this->status()); }

    return _copy;
}

template<typename T, typename... Args>
auto zpt::make_message(Args... _args) -> zpt::message {
    return zpt::allocate_shared<T>(std::forward<Args>(_args)...);
}

template<typename T, typename... Args>
auto zpt::allocate_message(Args... _args) -> zpt::message {
    return zpt::allocate_shared<T>(std::forward<Args>(_args)...);
}
