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
#include <array>
#include <atomic>
#include <ios>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <sys/epoll.h>
#include <systemd/sd-daemon.h>
#include <zapata/allocator.h>
#include <zapata/exceptions/NoMoreElementsException.h>
#include <zapata/lockfree/queue.h>
#include <zapata/text/convert.h>
#include <zapata/uuid.h>

/** @brief Sentinel index denoting that a stream is not present in a stream set. */
constexpr size_t stream_index_null = std::numeric_limits<size_t>::max();

namespace zpt {

/**
 * @brief Stream processing states for polling.
 */
enum stream_state {
    CONNECTED = 0,        ///< Stream has successfully connected to peer
    IDLE = 1,             ///< Stream is idle, not being processed
    WAITING = 2,          ///< Stream is waiting for I/O
    PROCESSING = 3,       ///< Stream is being processed by a delegate
    ERRORING_OUT = 4,     ///< Stream encountered an error
    DISCONNECTED = 5,     ///< Stream has been disconnected
    GRACEFULLY_CLOSE = 6, ///< Stream should be closed
    END
};

inline const char* stream_state_names[] = { "CONNECTED",        "IDLE",         "WAITING",
                                            "PROCESSING",       "ERRORING_OUT", "DISCONNECTED",
                                            "GRACEFULLY_CLOSE", "END" };

/** @brief Type alias for epoll event structure. */
using epoll_event_t = struct epoll_event;
/**
 * @brief Stable per-stream payload stored in `epoll_event::data.ptr`.
 *
 * Carries the stream's identity so that, at poll time, the stream can be
 * looked up and claimed by (index, uuid) without dereferencing a raw
 * pointer that may no longer be alive.
 */
struct epoll_event_data_t {
    /** @brief 128-bit stream UUID. */
    __uint128_t __uuid{ 0 };
    /** @brief Index of the stream within its stream set. */
    size_t __index{ 0 };
};

class polling;
class stream_set;

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
    friend class stream_set;

    /**
     * @brief Constructs a stream with the given transport scheme.
     * @param _transport Transport scheme (e.g., "tcp", "udp", "unix").
     * @return void (constructors implicitly initialize the object).
     */
    basic_stream(std::string const& _transport);
    /**
     * @brief Constructs from a unique pointer to a stream.
     * @tparam T Underlying stream type.
     * @param _args Arguments forwarded to T's constructor.
     * @return void (constructors implicitly initialize the object).
     */
    template<typename T, typename... Args>
    basic_stream(std::in_place_type_t<T>, std::string const& _transport, Args... _args);
    basic_stream(basic_stream const& _rhs) = delete;
    basic_stream(basic_stream&& _rhs) = delete;
    /**
     * @brief Destructor. Closes the stream.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~basic_stream();

    auto operator=(basic_stream const& _rhs) -> basic_stream& = delete;
    auto operator=(basic_stream&& _rhs) -> basic_stream& = delete;

    /**
     * @brief Sets the file descriptor.
     * @param _rhs File descriptor value.
     * @return Reference to this stream.
     */
    virtual auto operator=(int _rhs) -> basic_stream&;
    /**
     * @brief Reads a value from the stream using the transport protocol.
     * @tparam T Value type to read.
     * @param _out Output reference for the read value.
     * @return Reference to this stream.
     * @throws zpt::InterruptedException If the stream has no buffered message to read.
     */
    template<typename T>
    auto read(T& _out) -> basic_stream&;
    /**
     * @brief Writes a value to the stream using the transport protocol.
     * @tparam T Value type to write.
     * @param _in Value to write.
     * @return Reference to this stream.
     */
    template<typename T>
    auto write(T _in) -> basic_stream&;
    /**
     * @brief Reads without performing I/O (e.g., from internal buffer).
     * @param _out Output reference for the read value.
     * @return Reference to this stream.
     */
    virtual auto read_without_io(std::any& _out) -> basic_stream&;
    /**
     * @brief Writes without performing I/O (e.g., to internal buffer).
     * @param _in Value to write.
     * @return Reference to this stream.
     */
    virtual auto write_without_io(std::any const& _in) -> basic_stream&;
    /**
     * @brief Whether or not the stream consumed several messages and more are available.
     * @return True if more data is available.
     */
    virtual auto has_next() const -> bool;
    /**
     * @brief Stream extraction operator.
     * @tparam T Value type to extract.
     * @param _out Output reference for the extracted value.
     * @return Reference to this stream.
     */
    template<typename T>
    auto operator>>(T& _out) -> basic_stream&;
    /**
     * @brief Stream insertion operator.
     * @tparam T Value type to insert.
     * @param _in Value to insert.
     * @return Reference to this stream.
     */
    template<typename T>
    auto operator<<(T _in) -> basic_stream&;
    /**
     * @brief Stream manipulator support (e.g., std::endl).
     * @param _in Manipulator function.
     * @return Reference to this stream.
     */
    auto operator<<(ostream_manipulator _in) -> basic_stream&;
    /**
     * @brief Dereferences to the underlying iostream.
     * @return Reference to the underlying iostream.
     */
    auto operator*() -> std::iostream&;

    /**
     * @brief Returns the file descriptor.
     * @return File descriptor value.
     */
    virtual operator int();

    /**
     * @brief Sets the peer address and port on the underlying socket.
     * @tparam IOStream Underlying stream type.
     * @param _address Peer address.
     * @param _port Peer port.
     * @return Reference to this stream.
     */
    template<typename IOStream>
    auto set_peer(std::string const& _address, unsigned int _port) -> basic_stream&;
    /**
     * @brief Returns the stream's unique identifier.
     * @return UUID of this stream.
     */
    virtual auto uuid() const -> zpt::uuid const& final;
    /**
     * @brief Closes the stream.
     * @return Reference to this stream.
     */
    virtual auto close() -> basic_stream&;
    /**
     * @brief Upgrades the stream to a different transport.
     * @param _to_transport Target transport scheme.
     * @return Reference to this stream.
     */
    virtual auto upgrade(std::string const& _to_transport) -> basic_stream&;
    /**
     * @brief Returns the transport scheme.
     * @return Reference to transport scheme string.
     */
    virtual auto transport() const -> std::string const&;
    /**
     * @brief Returns the URI string.
     * @return Reference to URI string.
     */
    virtual auto uri() const -> std::string const&;
    /**
     * @brief Sets the stream processing state.
     * @param _state New processing state.
     * @return Reference to this stream.
     */
    virtual auto state(stream_state _state) -> basic_stream&;
    /**
     * @brief Returns the current stream processing state.
     * @return Current stream state.
     */
    virtual auto state() const -> stream_state;
    /**
     * @brief Returns whether the stream should remain in the polling set.
     * @return True if stream should persist.
     */
    virtual auto persistent() const -> bool;
    /**
     * @brief Sets arbitrary metadata on this stream.
     * @param _metadata Metadata to attach.
     * @return Reference to this stream.
     */
    virtual auto metadata(std::any _metadata) -> basic_stream&;
    /**
     * @brief Returns the attached metadata.
     * @return Reference to metadata value.
     */
    virtual auto metadata() const -> std::any const&;
    /**
     * @brief Returns the stable epoll event data block for this stream.
     *
     * The block (created on first call) holds this stream's UUID and set
     * index and is the value stored in `epoll_event::data.ptr`.
     * @return Pointer to the per-stream `epoll_event_data_t` block.
     */
    virtual auto event_data() -> void*;

  protected:
    /** @brief Underlying iostream wrapped by this stream. */
    zpt::allocator<std::iostream>::unique_pointer __underlying{ nullptr };
    /** @brief File descriptor associated with this stream. */
    int __fd{ -1 };
    /** @brief The stream's unique identifier */
    zpt::uuid __uuid;
    /** @brief This stream's index within its stream set. */
    size_t __index{ stream_index_null };
    /** @brief Current stream processing state. */
    std::atomic<zpt::stream_state> __state{ zpt::stream_state::IDLE };
    /** @brief Transport scheme (e.g., "tcp", "udp", "unix"). */
    std::string __transport{ "" };
    /** @brief URI string representation of this stream. */
    std::string __uri{ "" };
    /** @brief Arbitrary metadata attached to this stream. */
    std::any __metadata;
    /** @brief Lazily-created stable epoll event data block (see event_data()). */
    std::unique_ptr<zpt::epoll_event_data_t> __event_data{ nullptr };
};

/** @brief Shared pointer type for streams. */
using stream = std::shared_ptr<zpt::basic_stream>;

/**
 * @brief Fixed-capacity, thread-safe container of polled streams.
 *
 * Each stream occupies a slot addressed by an integer drawn from a free-index
 * pool. Every slot carries a lock-free claim token: the 128-bit UUID of the
 * stream while it is released (present in the poll, ready to be taken), and
 * zero while it is claimed (muted out of the poll) or the slot is free. Claiming
 * and releasing are CAS operations on that token, so the slot arrays are accessed
 * without a mutex; only the UUID and URI lookup maps are guarded by a shared
 * mutex.
 */
class stream_set {
  public:
    /** @brief Array of streams, one per slot. */
    using stream_array_type = std::unique_ptr<zpt::stream[]>;
    /** @brief Per-slot lock-free claim token (stream UUID, or zero). */
    using id_array_type = std::unique_ptr<std::atomic<__uint128_t>[]>;
    /** @brief Map of stream UUID to stream. */
    using by_uuid_type = std::map<zpt::uuid, zpt::stream>;
    /** @brief Map of stream URI to stream. */
    using by_uri_type = std::unordered_map<std::string, zpt::stream>;
    /**
     * @brief Constructs a stream set with a fixed number of slots.
     * @param _capacity Maximum number of streams the set can hold.
     * @return void (constructors implicitly initialize the object).
     */
    stream_set(size_t _capacity);
    /**
     * @brief Destructor. Closes all streams in the set.
     * @return void (destructors implicitly clean up the object).
     */
    ~stream_set();

    /** @brief Inserts a stream into the set and marks it ready (released).
     * @param _stream Stream to insert.
     * @return True if the stream was inserted, false otherwise. */
    auto insert(zpt::stream _stream) -> bool;
    /** @brief Removes a claimed stream from the set and frees its slot.
     * @param _stream Stream to remove.
     * @return True if the stream was removed, false otherwise. */
    auto erase(zpt::stream _stream) -> bool;
    /** @brief Changes the underlying transport of a stream.
     * @param _stream Stream to upgrade.
     * @param _transport New transport scheme.
     * @return Reference to this instance. */
    auto upgrade(zpt::stream _stream, std::string const& _transport) -> zpt::stream_set&;
    /** @brief Claims (mutes) a stream by its identity.
     * @param _stream Stream to claim.
     * @return True if the stream was claimed, false otherwise. */
    auto acquire(zpt::stream _stream) -> bool;
    /** @brief Claims (mutes) a stream by slot index and UUID.
     * @param _data The event data holding the slot index and the 128-bit UUID of the stream.
     * @return The claimed stream, or nullptr if it could not be claimed. */
    auto acquire(zpt::epoll_event_data_t const& _data) -> zpt::stream;
    /** @brief Returns whether a stream is currently claimed (muted).
     * @param _stream Stream to query.
     * @return True if the stream is claimed, false otherwise. */
    auto acquired(zpt::stream _stream) const -> bool;
    /** @brief Releases a claimed stream, marking it ready (unmuted).
     * @param _stream Stream to release.
     * @return True if the stream was released, false otherwise. */
    auto release(zpt::stream _stream) -> bool;
    /** @brief Retrieves the stream associated with the given identifier.
     * @param _stream_id UUID to look up.
     * @return Stream associated with the UUID, or nullptr. */
    auto get(zpt::uuid const& _stream_id) const -> zpt::stream;
    /** @brief Retrieves the stream associated with the given identifier.
     * @param _uri URI string to look up.
     * @return Stream associated with the URI, or nullptr. */
    auto get(std::string const& _uri) const -> zpt::stream;
    /** @brief Closes all streams in the set and clears it.
     * @return Reference to this instance. */
    auto close() -> zpt::stream_set&;

  private:
    /** @brief Total number of slots in the set. */
    size_t __capacity{ 0 };
    /** @brief Streams by slot index (nullptr for free slots). */
    stream_array_type __by_fd;
    /** @brief Per-slot lock-free claim token (stream UUID, or zero when claimed/free). */
    id_array_type __by_fd_id;
    /** @brief Pool of free slot indexes available for allocation. */
    zpt::lf::queue<size_t> __available_indexes;
    /** @brief Mutex protecting the UUID and URI lookup maps. */
    mutable std::shared_mutex __by_mutex;
    /** @brief Map of stream UUID to stream currently in the set. */
    by_uuid_type __by_uuid;
    /** @brief Map of stream URI to stream currently in the set. */
    by_uri_type __by_uri;

    /** @brief Claims (mutes) a stream by slot index and UUID.
     * @param _index Slot index of the stream.
     * @param _uuid 128-bit UUID of the stream.
     * @return The claimed stream, or nullptr if it could not be claimed. */
    auto acquire(size_t _index, __uint128_t _uuid) -> zpt::stream;
};

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
    using state_fn_type = std::function<void(zpt::uuid const&, std::string const&)>;
    /** @brief Maximum events processed per poll() call. */
    constexpr static int MAX_EVENT_PER_POLL{ 100 };

    /**
     * @brief Constructs a polling instance with epoll.
     * @param _max_streams Maximum number of polled streams.
     * @return void (constructors implicitly initialize the object).
     */
    polling(size_t _max_streams);
    /**
     * @brief Destructor. Closes the polling instance.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~polling();

    /**
     * @brief Closes the polling instance and all registered streams.
     * @return Reference to this polling instance.
     */
    auto close() -> zpt::polling&;
    /**
     * @brief Registers a delegate function called when streams are ready.
     * @param _callback Delegate function to register.
     * @return Reference to this polling instance.
     */
    auto register_delegate(delegate_fn_type _callback) -> zpt::polling&;
    /**
     * @brief Unregisters a previously registered delegate function.
     * @param _callback Delegate function to unregister.
     * @return Reference to this polling instance.
     */
    auto unregister_delegate(delegate_fn_type _callback) -> zpt::polling&;
    auto register_stream_state_listener(zpt::stream_state _type, state_fn_type _callback)
      -> zpt::polling&;
    auto unregister_stream_state_listener(zpt::stream_state _type, state_fn_type _callback)
      -> zpt::polling&;
    /**
     * @brief Adds a stream to be monitored for I/O.
     * @param _stream Stream to monitor.
     * @return Reference to this polling instance.
     */
    auto listen_on(zpt::stream _stream) -> zpt::polling&;
    /**
     * @brief Temporarily stops monitoring a stream.
     *
     * Claims the stream in the polled set and removes it from epoll. Throws
     * `zpt::NoSuchElementException` if the stream is disconnected and `zpt::failed_expectation`
     * if it is already claimed (busy); callers turn the latter into a `retrigger`.
     * @param _stream Stream to mute.
     * @return Reference to this polling instance.
     */
    auto mute(zpt::stream _stream) -> zpt::polling&;
    /**
     * @brief Temporarily stops monitoring a stream by UUID.
     * @param _id UUID of the stream to mute.
     * @return Muted stream.
     */
    auto mute(zpt::uuid const& _id) -> zpt::stream;
    /**
     * @brief Temporarily stops monitoring a stream by URI.
     * @param _uri URI of the stream to mute.
     * @return Muted stream.
     */
    auto mute(std::string const& _uri) -> zpt::stream;
    /**
     * @brief Temporarily stops monitoring a stream by slot index and UUID.
     * @param _data The event data holding the slot index and the 128-bit UUID of the stream.
     * @return Muted stream.
     */
    auto mute(zpt::epoll_event_data_t const& _data) -> zpt::stream;
    /**
     * @brief Releases a muted stream, draining any buffered work first.
     *
     * A `DISCONNECTED` stream is left alone. A non-persistent stream, or one in the
     * `ERRORING_OUT` / `GRACEFULLY_CLOSE` state, is erased from the polled set. Otherwise the
     * stream is released and re-armed in epoll. A stream whose `has_next()` is still true is
     * instead re-delegated synchronously so its backlog is pumped out in the current cycle;
     * for the `self` transport's `event_stream`, `has_next()` is always `false`, so its backlog
     * drains one message per ready event via the still-pending `eventfd` counter.
     * @param _stream Stream to unmute.
     * @return Reference to this polling instance.
     */
    auto unmute(zpt::stream _stream) -> zpt::polling&;
    /**
     * @brief Changes the underlying transport of a stream.
     * @param _stream Stream to upgrade.
     * @param _transport New transport scheme.
     * @return Reference to this polling instance.
     */
    auto upgrade(zpt::stream _stream, std::string const& _transport) -> zpt::polling&;
    /**
     * @brief Returns true if the stream is alive.
     * @param _stream_uuid The UUID of the stream to check.
     * @return True if the stream is alive, false otherwise.
     */
    auto is_alive(zpt::uuid _stream_uuid) const -> bool;
    /**
     * @brief Waits for I/O events and dispatches to delegates.
     * @return Reference to this polling instance.
     */
    auto poll() -> zpt::polling&;
    /**
     * @brief Initiates shutdown of the polling loop.
     * @return Reference to this polling instance.
     */
    auto shutdown() -> zpt::polling&;
    /**
     * @brief Returns true if shutdown has been initiated.
     * @return True if shutdown is in progress.
     */
    auto is_in_shutdown() const -> bool;

  private:
    /** @brief Epoll file descriptor for I/O multiplexing. */
    int __epoll_fd{ -1 };
    /** @brief Set of streams being currently polled. */
    zpt::stream_set __polled_streams;
    /** @brief List of delegate functions called when streams are ready. */
    std::vector<delegate_fn_type> __delegates;
    /** @brief List of stream state listener callbacks. */
    std::array<std::vector<state_fn_type>, zpt::stream_state::END> __state_callbacks;
    std::vector<zpt::epoll_event_t> __pending_events;
    /** @brief Flag indicating that shutdown has been initiated. */
    std::atomic<bool> __shutdown{ false };

    auto process_event(zpt::epoll_event_t& _event) -> bool;
    /** @brief Registers a stream with epoll (called by listen_on).
     * @param _stream Stream to register.
     * @return Reference to this polling instance. */
    auto insert(zpt::stream _stream) -> zpt::polling&;
    /** @brief Removes a stream from epoll and the polled map.
     * @param _stream Stream to remove.
     * @return Reference to this polling instance. */
    auto erase(zpt::stream _stream) -> zpt::polling&;
    /** @brief Dispatches a ready stream to all registered delegates.
     * @param _stream Stream that is ready for I/O.
     * @return Reference to this polling instance. */
    auto delegate(zpt::stream _stream) -> zpt::polling&;
    auto callback(zpt::stream_state _type, zpt::stream const& _stream) -> zpt::polling&;
};

/**
 * @brief Returns the global stream polling instance.
 * @param _max_streams Maximum number of polled streams.
 * @return Shared pointer to the polling instance.
 */
auto STREAM_POLLING(size_t _max_streams = 0) -> zpt::polling::ptr;

/**
 * @brief Creates a stream wrapping a specific iostream type.
 * @tparam T The underlying iostream type (e.g., socketstream).
 * @tparam Args Constructor argument types.
 * @param _args Arguments forwarded to T's constructor.
 * @return Shared pointer to the stream.
 */
template<typename T, typename... Args>
static auto make_stream(std::string const& _transport, Args... _args) -> zpt::stream;

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

template<typename T, typename... Args>
zpt::basic_stream::basic_stream(std::in_place_type_t<T>,
                                std::string const& _transport,
                                Args... _args)
  : __underlying{ zpt::allocate_unique<T>(std::forward<Args>(_args)...) }
  , __transport{ _transport } {
    if constexpr (std::is_convertible<T, int>::value) {
        this->__fd = static_cast<int>(static_cast<T&>(*this->__underlying));
    }
    if constexpr (std::is_convertible<T, std::string>::value) {
        auto _socket_uri = static_cast<std::string>(static_cast<T&>(*this->__underlying));
        this->__uri = std::format("{}{}{}",
                                  this->__transport,
                                  _socket_uri.find("+ssl") != std::string::npos ? "+ssl" : "",
                                  _socket_uri.substr(_socket_uri.find("://")));
    }
    zlog("Opening connection to " << this->__uri, zpt::trace);
    expect(!this->__underlying->fail() && !this->__underlying->bad(),
           "unable to open underlying `std::iostream` named '" << this->__uri << "'");
    this->__underlying->exceptions(std::ios_base::badbit);
}

template<typename T>
auto zpt::basic_stream::read(T& _out) -> zpt::basic_stream& {
    if (this->__underlying == nullptr) {
        if constexpr (std::is_copy_assignable<T>::value) {
            std::any _without_io;
            this->read_without_io(_without_io);
            if (_without_io.has_value()) { _out = std::any_cast<T>(_without_io); }
            else { throw zpt::InterruptedException{ "Empty message" }; }
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
auto zpt::make_stream(std::string const& _transport, Args... _args) -> zpt::stream {
    zpt::stream _to_return{ new zpt::basic_stream{
      std::in_place_type_t<T>{}, _transport, std::forward<Args>(_args)... } };
    return _to_return;
}
