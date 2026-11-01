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

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <systemd/sd-daemon.h>
#include <zapata/exceptions/NoSuchElementException.h>
#include <zapata/streams/streams.h>

namespace {
/** @brief Default poll wait timeout in milliseconds. */
constexpr std::uint64_t POLL_WAIT_TIMEOUT{ 1000 };
} // namespace

zpt::basic_stream::basic_stream(std::string const& _transport)
  : __transport{ _transport } {}

zpt::basic_stream::~basic_stream() {
    if (this->__fd != -1) { this->close(); }
}

auto zpt::basic_stream::operator=(int _rhs) -> zpt::basic_stream& {
    this->__fd = _rhs;
    return (*this);
}

auto zpt::basic_stream::operator<<(ostream_manipulator _in) -> zpt::basic_stream& {
    (*this->__underlying.get()) << _in;
    return (*this);
}

auto zpt::basic_stream::operator*() -> std::iostream& { return *this->__underlying.get(); }

zpt::basic_stream::operator int() { return this->__fd; }

auto zpt::basic_stream::read_without_io(std::any&) -> basic_stream& {
    expect(false, "Not implemented for `basic_stream`");
    return (*this);
}

auto zpt::basic_stream::write_without_io(std::any const&) -> basic_stream& {
    expect(false, "Not implemented for `basic_stream`");
    return (*this);
}

auto zpt::basic_stream::has_next() const -> bool { return false; }

auto zpt::basic_stream::uuid() const -> zpt::uuid const& { return this->__uuid; }

auto zpt::basic_stream::close() -> zpt::basic_stream& {
    zlog("Closing connection to " << this->uri(), zpt::trace);
    this->__underlying.reset(nullptr);
    this->__fd = -1;
    this->__state = zpt::stream_state::DISCONNECTED;
    return (*this);
}

auto zpt::basic_stream::upgrade(const std::string& _rhs) -> zpt::basic_stream& {
    this->__transport = _rhs;
    this->__uri =
      std::format("{}{}", this->__transport, this->__uri.substr(this->__uri.find("://")));
    return (*this);
}

auto zpt::basic_stream::transport() const -> std::string const& { return this->__transport; }

auto zpt::basic_stream::uri() const -> std::string const& { return this->__uri; }

auto zpt::basic_stream::state(zpt::stream_state _rhs) -> basic_stream& {
    this->__state.store(_rhs);
    return (*this);
}

auto zpt::basic_stream::state() const -> zpt::stream_state { return this->__state; }

auto zpt::basic_stream::persistent() const -> bool { return true; }

auto zpt::basic_stream::metadata(std::any _metadata) -> basic_stream& {
    this->__metadata = _metadata;
    return (*this);
}

auto zpt::basic_stream::metadata() const -> std::any const& { return this->__metadata; }

auto zpt::basic_stream::event_data() -> void* {
    if (this->__event_data == nullptr) {
        this->__event_data = std::make_unique<zpt::epoll_event_data_t>(
          static_cast<__uint128_t>(this->__uuid), this->__index);
    }
    return static_cast<void*>(this->__event_data.get());
}

zpt::polling::polling(size_t _max_streams)
  : __epoll_fd{ epoll_create(1) }
  , __polled_streams{ _max_streams } {}

zpt::polling::~polling() {
    this->shutdown();
    ::close(this->__epoll_fd);
}

auto zpt::polling::close() -> zpt::polling& {
    this->__polled_streams.close();
    return (*this);
}

auto zpt::polling::register_delegate(delegate_fn_type _callback) -> zpt::polling& {
    this->__delegates.push_back(_callback);
    return (*this);
}

auto zpt::polling::unregister_delegate(delegate_fn_type _callback) -> zpt::polling& {
    for (auto _it = this->__delegates.begin(); _it != this->__delegates.end();) {
        if (_it->target<delegate_fn_type>() == _callback.target<delegate_fn_type>()) {
            _it = this->__delegates.erase(_it);
        }
        else { ++_it; }
    }
    return (*this);
}

auto zpt::polling::register_stream_state_listener(zpt::stream_state _type, state_fn_type _callback)
  -> zpt::polling& {
    this->__state_callbacks[_type].push_back(_callback);
    return (*this);
}

auto zpt::polling::unregister_stream_state_listener(zpt::stream_state _type,
                                                    state_fn_type _callback) -> zpt::polling& {
    for (auto _it = this->__state_callbacks[_type].begin();
         _it != this->__state_callbacks[_type].end();) {
        if (_it->target<delegate_fn_type>() == _callback.target<delegate_fn_type>()) {
            _it = this->__state_callbacks[_type].erase(_it);
        }
        else { ++_it; }
    }
    return (*this);
}

auto zpt::polling::listen_on(zpt::stream _stream) -> zpt::polling& {
    if (this->__shutdown.load()) { return (*this); }
    this->insert(_stream);
    return (*this);
}

auto zpt::polling::mute(zpt::stream _stream) -> bool {
    if (_stream->state() == zpt::stream_state::DISCONNECTED) { return false; }
    if (this->__polled_streams.acquire(_stream)) {
        auto _fd = static_cast<int>(*_stream);
        epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
        return true;
    }
    return false;
}

auto zpt::polling::mute(zpt::uuid const& _id) -> zpt::stream {
    auto _stream = this->__polled_streams.get(_id);
    if (_stream == nullptr) { throw zpt::NoSuchElementException{ "No such stream" }; }
    if (this->mute(_stream)) { return _stream; }
    return nullptr;
}

auto zpt::polling::mute(std::string const& _uri) -> zpt::stream {
    auto _stream = this->__polled_streams.get(_uri);
    if (_stream == nullptr) { throw zpt::NoSuchElementException{ "No such stream" }; }
    if (this->mute(_stream)) { return _stream; }
    return nullptr;
}

auto zpt::polling::mute(zpt::epoll_event_data_t const& _data) -> zpt::stream {
    auto _stream = this->__polled_streams.acquire(_data);
    if (_stream != nullptr) {
        auto _fd = static_cast<int>(*_stream);
        epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
    }
    return _stream;
}

auto zpt::polling::unmute(zpt::stream _stream) -> bool {
    if (_stream->state() == zpt::stream_state::DISCONNECTED) { return false; }
    if (!_stream->persistent() || _stream->state() == zpt::stream_state::ERRORING_OUT ||
        _stream->state() == zpt::stream_state::GRACEFULLY_CLOSE) {
        this->erase(_stream);
        return false;
    }
    if (_stream->has_next()) {
        this->delegate(_stream);
        return false;
    }

    if (!this->__polled_streams.release(_stream)) { return false; }

    zpt::epoll_event_t _new_event;
    _new_event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    _new_event.data.ptr = _stream->event_data();

    auto _fd = static_cast<int>(*_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_ADD, _fd, &_new_event);
    return true;
}

auto zpt::polling::upgrade(zpt::stream _stream, std::string const& _transport) -> zpt::polling& {
    this->__polled_streams.upgrade(_stream, _transport);
    return (*this);
}

auto zpt::polling::is_alive(zpt::uuid _stream_uuid) const -> bool {
    return this->__polled_streams.get(_stream_uuid) != nullptr;
}

auto zpt::polling::poll() -> zpt::polling& {
    std::uint64_t _sd_watchdog_usec{ ::POLL_WAIT_TIMEOUT * 1000 };
    auto _sd_watchdog_enabled = sd_watchdog_enabled(0, &_sd_watchdog_usec) != 0;
    zpt::epoll_event_t _epoll_events[MAX_EVENT_PER_POLL];
    do {
        auto _n_alive = epoll_wait(this->__epoll_fd,
                                   _epoll_events,
                                   MAX_EVENT_PER_POLL,
                                   std::min(::POLL_WAIT_TIMEOUT, _sd_watchdog_usec / 1000));
        if (_n_alive < 0) { continue; }

        for (auto _k = 0; _k != _n_alive; ++_k) {
            auto& _event = _epoll_events[_k];
            if (!this->process_event(_event) && (_event.events & EPOLLIN) == EPOLLIN) {
                this->__pending_events.push_back(_event);
            }
        }
        for (auto _it = this->__pending_events.begin(); _it != this->__pending_events.end();) {
            auto& _event = *_it;
            if (this->process_event(_event) || (_event.events & EPOLLIN) == 0) {
                _it = this->__pending_events.erase(_it);
            }
            else { ++_it; }
        }

        if (_sd_watchdog_enabled) { sd_notify(0, "WATCHDOG=1"); }
    } while (!this->__shutdown.load());
    return (*this);
}

auto zpt::polling::shutdown() -> zpt::polling& {
    this->__shutdown.store(true);
    return (*this);
}

auto zpt::polling::is_in_shutdown() const -> bool { return this->__shutdown.load(); }

auto zpt::polling::process_event(zpt::epoll_event_t& _event) -> bool {
    auto _data = static_cast<zpt::epoll_event_data_t*>(_event.data.ptr);
    auto _stream = this->mute(*_data);
    if (_stream == nullptr) { return false; }

    if (((_event.events & EPOLLPRI) == EPOLLPRI) || ((_event.events & EPOLLHUP) == EPOLLHUP) ||
        ((_event.events & EPOLLERR) == EPOLLERR) || ((_event.events & EPOLLRDHUP) == EPOLLRDHUP)) {
        this->erase(_stream);
    }
    else if ((_event.events & EPOLLIN) == EPOLLIN) { this->delegate(_stream); }
    else {
        expect(
          ((_event.events & EPOLLPRI) == EPOLLPRI) || ((_event.events & EPOLLHUP) == EPOLLHUP) ||
            ((_event.events & EPOLLERR) == EPOLLERR) ||
            ((_event.events & EPOLLRDHUP) == EPOLLRDHUP) || ((_event.events & EPOLLIN) == EPOLLIN),
          "unrecognized polling event");
    }

    return true;
}

auto zpt::polling::insert(zpt::stream _stream) -> zpt::polling& {
    auto _fd = static_cast<int>(*_stream);

    if (this->__polled_streams.insert(_stream)) {
        zpt::epoll_event_t _new_event;
        _new_event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
        _new_event.data.ptr = _stream->event_data();
        epoll_ctl(this->__epoll_fd, EPOLL_CTL_ADD, _fd, &_new_event);
        this->callback(zpt::stream_state::CONNECTED, _stream);
    }

    return (*this);
}

auto zpt::polling::erase(zpt::stream _stream) -> zpt::polling& {
    auto _fd = static_cast<int>(*_stream);
    if (this->__polled_streams.erase(_stream)) {
        epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
        this->callback(zpt::stream_state::DISCONNECTED, _stream);
    }
    return (*this);
}

auto zpt::polling::delegate(zpt::stream _stream) -> zpt::polling& {
#ifdef ALLOCATOR_DEBUG_MODE
    zpt::mem::print_still_allocated();
    zpt::mem::start_tracking();
#endif
    for (auto& d : this->__delegates) {
        if (d(this->shared_from_this(), _stream)) { return (*this); }
    }

    this->unmute(_stream);
    malloc_trim(0);
    return (*this);
}

auto zpt::polling::callback(zpt::stream_state _type, zpt::stream const& _stream) -> zpt::polling& {
    if (_stream->transport() == "self") { return (*this); }

    for (auto _callback : this->__state_callbacks[_type]) {
        _callback(_stream->uuid(), _stream->uri());
    }
    return (*this);
}

auto zpt::STREAM_POLLING(size_t _max_streams) -> zpt::polling::ptr {
    static zpt::polling::ptr _global = zpt::allocate_shared<zpt::polling>(_max_streams);
    return _global;
}
