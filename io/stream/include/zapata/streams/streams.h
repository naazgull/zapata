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
 * @file streams.h
 * @brief Core stream abstractions and epoll-based polling.
 *
 * Provides a unified stream interface that wraps std::iostream and
 * adds file descriptor support for use with epoll-based I/O multiplexing.
 *
 * @see zpt::basic_stream
 * @see zpt::polling
 */

#pragma once

#include <any>
#include <atomic>
#include <iostream>
#include <memory>
#include <sys/epoll.h>
#include <systemd/sd-daemon.h>
#include <zapata/allocator.h>
#include <zapata/exceptions/NoMoreElementsException.h>
#include <zapata/locks/spin_mutex.h>
#include <zapata/text/convert.h>

namespace zpt {

/**
 * @brief Stream processing states for polling.
 */
enum class stream_state {
    IDLE,        ///< Stream is idle, not being processed
    WAITING,     ///< Stream is waiting for I/O
    PROCESSING,  ///< Stream is being processed by a delegate
    ERRORING_OUT ///< Stream encountered an error
};

/** @brief Type alias for epoll event structure. */
using epoll_event_t = struct epoll_event;

class polling;

/**
 * @brief Abstract stream wrapper with file descriptor support.
 *
 * Wraps a std::iostream and provides file descriptor access for
 * integration with epoll-based polling. Supports reading/writing
 * arbitrary types via operator>> and operator<<.
 *
 * @par Example
 * @code
 * // Create a socket stream
 * auto stream = zpt::make_stream<zpt::socketstream>("localhost", 8080);
 *
 * // Write a message
 * stream << my_message;
 *
 * // Read response
 * zpt::message response;
 * stream >> response;
 * @endcode
 */
class basic_stream : public std::enable_shared_from_this<basic_stream> {
  public:
    typedef std::ostream& (*ostream_manipulator)(std::ostream&);
    friend class polling;

    /** @brief Default constructor. */
    basic_stream() = default;
    /** @brief Constructs from a unique pointer to a stream. */
    basic_stream(zpt::allocator<std::iostream>::unique_pointer _underlying);
    basic_stream(basic_stream const& _rhs) = delete;
    basic_stream(basic_stream&& _rhs) = delete;
    /** @brief Destructor. Closes the stream. */
    virtual ~basic_stream();

    auto operator=(basic_stream const& _rhs) -> basic_stream& = delete;
    auto operator=(basic_stream&& _rhs) -> basic_stream& = delete;

    /** @brief Sets the file descriptor. */
    virtual auto operator=(int _rhs) -> basic_stream&;
    /** @brief Reads a value from the stream using the transport protocol. */
    template<typename T>
    auto read(T& _out) -> basic_stream&;
    /** @brief Writes a value to the stream using the transport protocol. */
    template<typename T>
    auto write(T _in) -> basic_stream&;
    /** @brief Reads without performing I/O (e.g., from internal buffer). */
    virtual auto read_without_io(std::any& _out) -> basic_stream&;
    /** @brief Writes without performing I/O (e.g., to internal buffer). */
    virtual auto write_without_io(std::any const& _in) -> basic_stream&;
    /** @brief Whether or not the stream consumed several messages and more are available. */
    virtual auto has_next() const -> bool;
    /** @brief Stream extraction operator. */
    template<typename T>
    auto operator>>(T& _out) -> basic_stream&;
    /** @brief Stream insertion operator. */
    template<typename T>
    auto operator<<(T _in) -> basic_stream&;
    /** @brief Stream manipulator support (e.g., std::endl). */
    auto operator<<(ostream_manipulator _in) -> basic_stream&;
    /** @brief Dereferences to the underlying iostream. */
    auto operator*() -> std::iostream&;

    /** @brief Returns the file descriptor. */
    virtual operator int();

    /** @brief Sets the peer address and port on the underlying socket. */
    template<typename IOStream>
    auto set_peer(std::string const& _address, unsigned int _port) -> basic_stream&;
    virtual auto close() -> basic_stream&;
    virtual auto shutdown() -> basic_stream&;
    virtual auto transport(const std::string& _rhs) -> basic_stream&;
    virtual auto transport() -> std::string&;
    virtual auto uri(const std::string& _rhs) -> basic_stream&;
    virtual auto uri() -> std::string&;
    virtual auto state() -> stream_state&;
    virtual auto persistent() -> bool;
    virtual auto metadata(std::any _metadata) -> basic_stream&;
    virtual auto metadata() const -> std::any const&;

  protected:
    /** @brief Underlying iostream wrapped by this stream. */
    zpt::allocator<std::iostream>::unique_pointer __underlying{ nullptr };
    /** @brief File descriptor associated with this stream. */
    int __fd{ -1 };
    /** @brief Transport scheme (e.g., "tcp", "udp", "unix"). */
    std::string __transport{ "" };
    /** @brief URI string representation of this stream. */
    std::string __uri{ "" };
    /** @brief Current stream processing state. */
    zpt::stream_state __state{ zpt::stream_state::IDLE };
    /** @brief Arbitrary metadata attached to this stream. */
    std::any __metadata;
    /** @brief Whether this stream is currently muted (not monitored by polling). */
    bool __muted{ true };

    /** @brief Extracts the URI from the underlying iostream. */
    auto extract_uri() -> void;
};

/** @brief Shared pointer type for streams. */
using stream = std::shared_ptr<zpt::basic_stream>;

/**
 * @brief Epoll-based I/O multiplexer for efficient stream handling.
 *
 * Monitors multiple streams for I/O readiness using Linux epoll and
 * dispatches to registered delegate functions when data is available.
 *
 * @par Example
 * @code
 * auto poll = zpt::STREAM_POLLING();
 *
 * // Register handler
 * poll->register_delegate([](zpt::polling::ptr p, zpt::stream s) {
 *     zpt::message msg;
 *     s >> msg;
 *     process(msg);
 *     return true;  // Keep listening
 * });
 *
 * // Add streams to monitor
 * poll->listen_on(my_stream);
 *
 * // Poll loop
 * while (!poll->is_in_shutdown()) {
 *     poll->poll();
 * }
 * @endcode
 */
class polling : public std::enable_shared_from_this<polling> {
  public:
    using ptr = std::shared_ptr<polling>;
    /** @brief Delegate function signature: returns true to keep stream, false to remove. */
    using delegate_fn_type = std::function<bool(zpt::polling::ptr _poll, zpt::stream _stream)>;
    /** @brief Maximum events processed per poll() call. */
    constexpr static int MAX_EVENT_PER_POLL{ 100 };

    polling();
    virtual ~polling();

    /** @brief Closes the polling instance and all registered streams. */
    auto close() -> zpt::polling&;
    /** @brief Registers a delegate function called when streams are ready. */
    auto register_delegate(delegate_fn_type _callback) -> zpt::polling&;
    /** @brief Registers a delegate function called when streams are ready. */
    auto unregister_delegate(delegate_fn_type _callback) -> zpt::polling&;
    /** @brief Adds a stream to be monitored for I/O. */
    auto listen_on(zpt::stream _stream) -> zpt::polling&;
    /** @brief Temporarily stops monitoring a stream. */
    auto mute(zpt::stream _stream) -> zpt::polling&;
    /** @brief Resumes monitoring a muted stream. */
    auto unmute(zpt::stream _stream) -> zpt::polling&;

    /** @brief Waits for I/O events and dispatches to delegates. */
    auto poll() -> zpt::polling&;
    /** @brief Initiates shutdown of the polling loop. */
    auto shutdown() -> zpt::polling&;
    /** @brief Returns true if shutdown has been initiated. */
    auto is_in_shutdown() const -> bool;

  private:
    /** @brief Epoll file descriptor for I/O multiplexing. */
    int __epoll_fd{ -1 };
    /** @brief Mutex protecting the polled streams map. */
    zpt::locks::spin_mutex __poll_lock;
    // std::shared_mutex __poll_lock;
    /** @brief Map of file descriptors to stream pointers currently being monitored. */
    std::map<int, zpt::stream> __polled_streams;
    /** @brief List of delegate functions called when streams are ready. */
    std::vector<delegate_fn_type> __delegates;
    /** @brief Flag indicating that shutdown has been initiated. */
    std::atomic<bool> __shutdown{ false };

    /** @brief Registers a stream with epoll (called by listen_on). */
    auto insert(zpt::stream _stream) -> zpt::polling&;
    /** @brief Retrieves the stream associated with the given identifier. */
    auto get(int _stream_id) -> zpt::stream;
    /** @brief Removes a stream from epoll and the polled map. */
    auto erase(zpt::stream _stream) -> zpt::polling&;
    /** @brief Dispatches a ready stream to all registered delegates. */
    auto delegate(zpt::stream _stream) -> zpt::polling&;
};

/**
 * @brief Returns the global stream polling instance.
 * @return Shared pointer to the polling instance.
 */
auto STREAM_POLLING() -> zpt::polling::ptr;

/**
 * @brief Creates a stream wrapping a specific iostream type.
 * @tparam T The underlying iostream type (e.g., socketstream).
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to T's constructor.
 * @return Shared pointer to the stream.
 */
template<typename T, typename... Args>
static auto make_stream(Args... _args) -> zpt::stream;

#define CRLF "\r\n"

/**
 * @brief Casts a stream to access its underlying iostream type.
 * @tparam T Target iostream type.
 * @param _rhs Stream to cast.
 * @return Reference to the underlying iostream as type T.
 */
template<typename T>
auto stream_cast(zpt::stream& _rhs) -> T& {
    return static_cast<T&>(**_rhs);
}
} // namespace zpt

template<typename T>
auto zpt::basic_stream::read(T& _out) -> zpt::basic_stream& {
    if (this->__underlying == nullptr) {
        if constexpr (std::is_copy_assignable<T>::value) {
            std::any _without_io;
            this->read_without_io(_without_io);
            _out = std::any_cast<T>(_without_io);
        }
        return (*this);
    }

    auto& _underlying = *this->__underlying.get();
    if constexpr (!std::is_same<T, std::string>::value && std::is_class<T>::value) {
        _out->from_stream(_underlying);
    }
    else { _underlying >> _out; }
    return (*this);
}

template<typename T>
auto zpt::basic_stream::write(T _in) -> zpt::basic_stream& {
    if (this->__underlying == nullptr) {
        this->write_without_io(std::make_any<T>(_in));
        return (*this);
    }

    auto& _underlying = *this->__underlying.get();
    if constexpr (!std::is_same<T, std::string>::value && std::is_class<T>::value) {
        _in->to_stream(_underlying);
        _underlying << std::flush;
    }
    else { _underlying << _in << std::flush; }
    return (*this);
}

template<typename T>
auto zpt::basic_stream::operator>>(T& _out) -> zpt::basic_stream& {
    return this->read<T>(_out);
}

template<typename T>
auto zpt::basic_stream::operator<<(T _in) -> zpt::basic_stream& {
    return this->write<T>(_in);
}

template<typename IOStream>
auto zpt::basic_stream::set_peer(std::string const& _address, unsigned int _port) -> basic_stream& {
    static_cast<IOStream&>(**this).set_peer(_address, _port);
    return (*this);
}

template<typename T, typename... Args>
auto zpt::make_stream(Args... _args) -> zpt::stream {
    zpt::stream _to_return{ new zpt::basic_stream{
      zpt::allocate_unique<T>(std::forward<Args>(_args)...) } };
    if constexpr (std::is_convertible<T, int>::value) {
        (*_to_return) = static_cast<int>(static_cast<T&>(**_to_return));
    }
    if constexpr (std::is_convertible<T, std::string>::value) {
        _to_return->uri(static_cast<std::string>(static_cast<T&>(**_to_return)));
    }
    expect(!(**_to_return.get()).fail() && !(**_to_return.get()).bad(),
           "unable to open underlying `std::iostream` named '" << _to_return->uri() << "'");
    return _to_return;
}
