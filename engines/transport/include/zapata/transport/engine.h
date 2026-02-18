/**
 * @file engine.h
 * @brief Transport engine for handling network I/O and event dispatch.
 *
 * Provides the main processing loop that integrates transports with
 * the event system. Handles receiving messages from streams, resolving
 * them to event handlers, and dispatching responses.
 *
 * Key types:
 * - `zpt::transports::engine` - Main engine class
 * - `zpt::events::receive` - Event for receiving messages
 * - `zpt::events::send` - Event for sending messages
 * - `zpt::events::process` - Base class for message handlers
 *
 * @see zpt::transports::engine
 * @see zpt::events::process
 */

#pragma once

#include <list>
#include <zapata/events.h>
#include <zapata/net/socket.h>
#include <zapata/startup.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
namespace transports {

/**
 * @brief Transport engine coordinating network I/O with event dispatch.
 *
 * The engine manages the lifecycle of network communication:
 * 1. Receives messages from streams via polling
 * 2. Resolves messages to registered event handlers
 * 3. Dispatches events to consumer threads
 * 4. Sends responses back through streams
 *
 * @par Example
 * @code
 * auto& engine = zpt::TRANSPORT_ENGINE(config);
 * engine.add_resolver(my_resolver);
 *
 * // Engine runs in background, processing incoming connections
 * engine.dispatcher()->trap();  // Wait for shutdown
 * @endcode
 */
class engine {
  public:
    /** @brief Constructs an engine with the given configuration. */
    engine(zpt::json _config);
    /** @brief Destructor. */
    virtual ~engine() = default;

    /** @brief Adds an event resolver for routing messages. */
    auto add_resolver(zpt::events::resolver _resolver) -> engine&;
    /** @brief Resolves a message to matching event handlers. */
    auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
      -> std::list<zpt::event>;
    /** @brief Returns the event dispatcher. */
    auto dispatcher() -> zpt::events::dispatcher::ptr;
    /** @brief Initiates engine shutdown. */
    auto shutdown() -> engine&;

  private:
    zpt::json __configuration;
    std::vector<zpt::events::resolver> __resolvers;
    zpt::events::dispatcher::ptr __dispatcher;
};
} // namespace transports

namespace events {

/**
 * @brief Initialization data passed to transport events.
 *
 * Contains references to the dispatcher, polling instance, and
 * stream for use during event processing.
 */
class transport_event_init : public zpt::event_initialization {
  public:
    zpt::events::dispatcher::ptr __dispatcher; ///< Event dispatcher
    zpt::polling::ptr __polling;               ///< I/O polling instance
    zpt::stream __stream;                      ///< Source stream
};

/**
 * @brief Event operation for receiving messages from a stream.
 *
 * Reads a message from the stream using the appropriate transport,
 * resolves it to handlers, and triggers processing events.
 */
class receive {
  public:
    /** @brief Constructs a receive event for the given engine, polling instance, and stream. */
    receive(zpt::transports::engine& _engine, zpt::polling::ptr _polling, zpt::stream _stream);
    receive(zpt::events::receive const& _rhs) = delete;
    receive(zpt::events::receive&& _rhs) = delete;
    /** @brief Destructor. */
    virtual ~receive();

    auto operator=(zpt::events::receive const& _rhs) -> receive& = delete;
    auto operator=(zpt::events::receive&& _rhs) -> receive& = delete;

    /** @brief Stores dispatcher and polling references from initialization data. */
    auto initialize(zpt::event_initialization& init) -> void;
    /** @brief Returns false (receive events are never blocked). */
    auto blocked() const -> bool;
    /** @brief Returns true (receive events are always authorized). */
    auto authorized() const -> bool;
    /** @brief Handles generic exceptions during receive. Returns false. */
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    /** @brief Handles allocation failures during receive. Returns false. */
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    /** @brief Handles expectation failures during receive. Returns false. */
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
    /** @brief Reads a message from the stream and triggers processing events. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  protected:
    zpt::transports::engine& __engine;
    zpt::polling::ptr __polling;
    zpt::stream __stream;
};

/**
 * @brief Event operation for sending messages to a stream.
 *
 * Serializes and writes a message to a stream using the
 * appropriate transport protocol.
 */
class send {
  public:
    /** @brief Constructs a send event for the given stream and message. */
    send(zpt::polling::ptr _polling, zpt::stream _stream, zpt::message _to_send);
    send(zpt::events::send const& _rhs) = delete;
    send(zpt::events::send&& _rhs) = delete;
    /** @brief Destructor. */
    virtual ~send();

    auto operator=(zpt::events::send const& _rhs) -> send& = delete;
    auto operator=(zpt::events::send&& _rhs) -> send& = delete;

    /** @brief Stores dispatcher and polling references from initialization data. */
    auto initialize(zpt::event_initialization& init) -> void;
    /** @brief Returns false (send events are never blocked). */
    auto blocked() const -> bool;
    /** @brief Returns true (send events are always authorized). */
    auto authorized() const -> bool;
    /** @brief Handles generic exceptions during send. Returns false. */
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    /** @brief Handles allocation failures during send. Returns false. */
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    /** @brief Handles expectation failures during send. Returns false. */
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
    /** @brief Writes the message to the stream using the appropriate transport. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  protected:
    zpt::polling::ptr __polling;
    zpt::stream __stream;
    zpt::message __to_send;
};

/**
 * @brief Abstract base class for message processing events.
 *
 * Subclass this to implement custom message handlers. The process
 * class manages the received message and prepares the response.
 *
 * @par Example
 * @code
 * class MyHandler : public zpt::events::process {
 * public:
 *     MyHandler(zpt::message msg) : process(msg) {}
 *
 *     bool blocked() const override { return false; }
 *
 *     zpt::events::state operator()(zpt::events::dispatcher::ptr d) override {
 *         // Process received()
 *         to_send()->body() = { "status", "ok" };
 *         return zpt::events::finish;
 *     }
 * };
 * @endcode
 */
class process {
  public:
    using ptr = std::shared_ptr<process>;
    friend class zpt::events::receive;

    /** @brief Constructs a process event with the received message. */
    process(zpt::message _received);
    /** @brief Constructs a process event with the received message and call context. */
    process(zpt::message _received, zpt::call_context::ptr _context);
    process(zpt::events::process const& _rhs) = delete;
    process(zpt::events::process&& _rhs) = delete;
    /** @brief Destructor. Sends the response if a stream is available. */
    virtual ~process();

    auto operator=(zpt::events::process const& _rhs) -> process& = delete;
    auto operator=(zpt::events::process&& _rhs) -> process& = delete;

    /** @brief Returns the type of transport used to receive the message. */
    virtual auto transport_type() -> std::string const& final;
    /** @brief Returns the received message. */
    virtual auto received() const -> zpt::message const final;
    /** @brief Returns the message to send as response. */
    virtual auto to_send() -> zpt::message final;
    /** @brief Returns the call context for this process event. */
    virtual auto context() const -> zpt::call_context::ptr final;
    /** @brief Sets the call context for this process event. */
    virtual auto context(zpt::call_context::ptr _context) -> process& final;

    /** @brief Stores dispatcher, polling, and stream from initialization data. */
    virtual auto initialize(zpt::event_initialization& init) -> void final;
    /** @brief Returns true if the request is authorized. Default returns true. */
    virtual auto authorized() const -> bool;
    /** @brief Handles generic exceptions. Sends error response. Returns false. */
    virtual auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool final;
    /** @brief Handles allocation failures. Sends error response. Returns false. */
    virtual auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool final;
    /** @brief Handles expectation failures. Sends error response. Returns false. */
    virtual auto catch_error(zpt::failed_expectation const& _e,
                             zpt::events::dispatcher::ptr _dispatcher) -> bool final;

    /** @brief Returns true if processing is blocked waiting for something. */
    virtual auto blocked() const -> bool = 0;
    /** @brief Executes the message processing logic. */
    virtual auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state = 0;

  private:
    zpt::events::dispatcher::ptr __dispatcher;
    zpt::polling::ptr __polling;
    zpt::stream __stream;
    zpt::message __received{ nullptr };
    zpt::message __to_send{ nullptr };
    zpt::call_context::ptr __context{ nullptr };
};
} // namespace events
} // namespace zpt

/**
 * @brief Concept constraining types to zpt::events::process subclasses.
 */
template<typename T>
concept ProcessOperation = std::is_base_of<zpt::events::process, T>::value;

namespace zpt {
namespace events {

/**
 * @brief Default message processor that discards messages.
 *
 * Used as the default handler when no other processor is registered.
 * Simply completes without sending a response.
 */
class discard : public zpt::events::process {
  public:
    using zpt::events::process::process;
    /** @brief Destructor. */
    ~discard() = default;
    /** @brief Returns false (discard events are never blocked). */
    auto blocked() const -> bool;
    /** @brief Completes without sending a response. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};

/**
 * @brief Event operation for making outbound calls.
 *
 * Sends a message to a remote endpoint and registers a callback
 * for handling the response.
 *
 * @tparam T ProcessOperation type for handling the response.
 *
 * @par Example
 * @code
 * auto request = transport->make_request();
 * request->performative(zpt::Get);
 * request->uri()["path"] = "/api/resource";
 *
 * dispatcher->trigger<zpt::events::call<MyResponseHandler>>(resolver, request);
 * @endcode
 */
template<ProcessOperation T = zpt::events::discard>
class call {
  public:
    using ptr = std::shared_ptr<process>;
    friend class zpt::events::receive;

    /** @brief Constructs a call event with resolver, context, and outbound message. */
    call(zpt::events::resolver _resolver, zpt::call_context::ptr _context, zpt::message _send);
    call(zpt::events::call<T> const& _rhs) = delete;
    call(zpt::events::call<T>&& _rhs) = delete;
    /** @brief Destructor. */
    virtual ~call();

    auto operator=(zpt::events::call<T> const& _rhs) -> call& = delete;
    auto operator=(zpt::events::call<T>&& _rhs) -> call& = delete;

    /** @brief Stores dispatcher and polling references from initialization data. */
    auto initialize(zpt::event_initialization& init) -> void;
    /** @brief Returns false (call events are never blocked). */
    auto blocked() const -> bool;
    /** @brief Returns true (call events are always authorized). */
    auto authorized() const -> bool;
    /** @brief Handles generic exceptions during call. Returns false. */
    auto catch_error(std::exception const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    /** @brief Handles allocation failures during call. Returns false. */
    auto catch_error(std::bad_alloc const& _e, zpt::events::dispatcher::ptr _dispatcher) -> bool;
    /** @brief Handles expectation failures during call. Returns false. */
    auto catch_error(zpt::failed_expectation const& _e, zpt::events::dispatcher::ptr _dispatcher)
      -> bool;
    /** @brief Resolves the target and sends the message (internally or externally). */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;

  private:
    zpt::events::dispatcher::ptr __dispatcher;
    zpt::events::resolver __resolver;
    zpt::polling::ptr __polling;
    zpt::message __to_send;

    auto call_internally() -> call&;
    auto send_externally() -> call&;
};

/**
 * @brief Default processor for call reply messages.
 *
 * Delivers the reply to the call_context so the caller can retrieve the response.
 */
class process_call_reply : public zpt::events::process {
  public:
    using zpt::events::process::process;
    /** @brief Destructor. */
    ~process_call_reply() = default;
    /** @brief Returns false (reply processing is never blocked). */
    auto blocked() const -> bool;
    /** @brief Delivers the reply to the call context. */
    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state;
};
} // namespace events

/**
 * @brief Returns the global transport engine instance.
 * @param _config Optional configuration (used only on first call).
 * @return Reference to the global transport engine.
 */
auto TRANSPORT_ENGINE(zpt::json _config = nullptr) -> zpt::transports::engine&;

template<ProcessOperation T = zpt::events::process_call_reply>
auto make_call(zpt::events::resolver _resolver, zpt::message _to_send) -> zpt::call_context::ptr;
} // namespace zpt

template<ProcessOperation T>
zpt::events::call<T>::call(zpt::events::resolver _resolver,
                           zpt::call_context::ptr _context,
                           zpt::message _send)
  : __resolver{ _resolver }
  , __to_send{ _send } {
    if (!this->__to_send->headers()("X-Conversation-ID")->ok()) {
        this->__to_send->headers()["X-Conversation-ID"] = zpt::generate::r_uuid();
    }
    this->__resolver->add(_send, _context, zpt::events::make_callback<T>);
}

template<ProcessOperation T>
zpt::events::call<T>::~call() {}

template<ProcessOperation T>
auto zpt::events::call<T>::initialize(zpt::event_initialization& _init) -> void {
    auto _transport_init = reinterpret_cast<zpt::events::transport_event_init&>(_init);
    this->__dispatcher = _transport_init.__dispatcher;
    this->__polling = _transport_init.__polling;
}

template<ProcessOperation T>
auto zpt::events::call<T>::blocked() const -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::authorized() const -> bool {
    return true;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(std::exception const&, zpt::events::dispatcher::ptr)
  -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(std::bad_alloc const&, zpt::events::dispatcher::ptr)
  -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::catch_error(zpt::failed_expectation const&, zpt::events::dispatcher::ptr)
  -> bool {
    return false;
}

template<ProcessOperation T>
auto zpt::events::call<T>::operator()(zpt::events::dispatcher::ptr) -> zpt::events::state {
    auto& _uri = this->__to_send->uri();
    expect(_uri("path")->ok(), "Can't send a message without a resource path");

    bool _is_self{ false };
    if (!_uri("scheme")->ok() || !_uri("domain")->ok() || !_uri("port")->ok()) {
        auto _found = this->__resolver->search(
          std::format("/{}{}",
                      zpt::ontology::to_str(this->__to_send->performative()),
                      _uri("raw_path")->string()));
        expect(_found->ok() && _found->size() != 0,
               "Couldn't find a provider of '" << _uri("path")->string());

        for (auto [_, __, _service] : _found) {
            if ((_is_self = (_service("provider_id") == zpt::IDENTITY()("_id")))) { break; }
        }

        if (!_is_self) {
            auto _provider = this->__resolver->get_provider(_found(0)("provider_id")->string());
            expect(_provider->ok() && _provider->size() != 0,
                   "Couldn't find a provider of '" << _uri("path")->string());
            _uri["scheme"] = _provider(0)("protocols")("default");
            _uri["domain"] =
              _provider(0)("protocols")("registered")(_uri("scheme")->string())("bind");
            _uri["port"] =
              _provider(0)("protocols")("registered")(_uri("scheme")->string())("port");
        }
    }

    if (_is_self) { this->call_internally(); }
    else { this->send_externally(); }

    return zpt::events::finish;
}

template<ProcessOperation T>
auto zpt::events::call<T>::call_internally() -> call& {
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get("self");
    expect(_transport->has_capability(zpt::transport_capability::SYNCHRONOUS),
           "`call` only makes sense for synchronous protocols");

    auto _stream = std::make_shared<zpt::event_stream>();
    _stream->transport("self");

    this->__to_send->headers()["Content-Type"] = "application/json";
    _transport->send(_stream, this->__to_send);

    this->__polling->listen_on(_stream);

    return (*this);
}

template<ProcessOperation T>
auto zpt::events::call<T>::send_externally() -> call& {
    auto& _uri = this->__to_send->uri();
    auto _scheme = _uri("scheme")->string();
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(_scheme);
    expect(_transport->has_capability(zpt::transport_capability::SYNCHRONOUS),
           "`call` only makes sense for synchronous protocols");

    auto _stream = zpt::make_stream<zpt::socketstream>(
      _uri("domain")->string(), _uri("port")->integer(), zpt::NO_SSL, IPPROTO_TCP);
    _stream->transport(_scheme);

    this->__to_send->headers()["Content-Type"] = "application/json";
    _transport->send(_stream, this->__to_send);

    this->__polling->listen_on(_stream);

    return (*this);
}

template<ProcessOperation T>
auto zpt::make_call(zpt::events::resolver _resolver, zpt::message _to_send)
  -> zpt::call_context::ptr {
    auto _context = std::make_shared<zpt::call_context>();
    zpt::TRANSPORT_ENGINE() //
      .dispatcher()
      ->trigger<zpt::events::call<T>>(_resolver, _context, _to_send);
    return _context;
}
