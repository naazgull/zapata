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

zpt::basic_stream::~basic_stream() { this->close(); }

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
    this->__transport = "";
    this->__uri = "";
    this->__state = zpt::stream_state::IDLE;
    return (*this);
}

auto zpt::basic_stream::shutdown() -> zpt::basic_stream& {
    ::shutdown(this->__fd, SHUT_RDWR);
    ::close(this->__fd);
    return (*this);
}

auto zpt::basic_stream::upgrade(const std::string& _rhs) -> zpt::basic_stream& {
    this->__transport = _rhs;
    this->__uri =
      std::format("{}{}", this->__transport, this->__uri.substr(this->__uri.find("://")));
    return (*this);
}

auto zpt::basic_stream::transport() -> std::string& { return this->__transport; }

auto zpt::basic_stream::uri() -> std::string& { return this->__uri; }

auto zpt::basic_stream::state(zpt::stream_state _rhs) -> basic_stream& {
    this->__state.store(_rhs);
    return (*this);
}

auto zpt::basic_stream::state() -> zpt::stream_state { return this->__state; }

auto zpt::basic_stream::persistent() -> bool { return true; }

auto zpt::basic_stream::metadata(std::any _metadata) -> basic_stream& {
    this->__metadata = _metadata;
    return (*this);
}

auto zpt::basic_stream::metadata() const -> std::any const& { return this->__metadata; }

zpt::polling::polling()
  : __epoll_fd{ epoll_create(1) } {}

zpt::polling::~polling() {
    this->shutdown();
    ::close(this->__epoll_fd);
}

auto zpt::polling::close() -> zpt::polling& {
    for (auto& [_, _stream] : this->__polled_streams) { _stream->shutdown(); }
    this->__polled_streams.clear();
    this->__polled_streams_by_uuid.clear();
    this->__polled_streams_by_uri.clear();
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

auto zpt::polling::listen_on(zpt::stream _stream) -> zpt::polling& {
    if (!this->__shutdown.load()) { this->insert(_stream); }
    return (*this);
}

auto zpt::polling::mute(zpt::stream _stream) -> zpt::polling& {
    expect(!_stream->__muted.exchange(true), "Stream is already muted");

    auto _fd = static_cast<int>(*_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
    return (*this);
}

auto zpt::polling::mute(zpt::uuid const& _id) -> zpt::stream {
    auto _stream = this->get(_id);
    this->mute(_stream);
    return _stream;
}

auto zpt::polling::mute(std::string const& _uri) -> zpt::stream {
    auto _stream = this->get(_uri);
    this->mute(_stream);
    return _stream;
}

auto zpt::polling::unmute(zpt::stream _stream) -> zpt::polling& {
    if (!_stream->persistent()) {
        this->erase(_stream);
        return (*this);
    }
    if (_stream->has_next()) {
        this->delegate(_stream);
        return (*this);
    }

    expect(_stream->__muted.exchange(false), "Stream is already unmuted");

    zpt::epoll_event_t _new_event;
    _new_event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    _new_event.data.ptr = _stream.get();

    auto _fd = static_cast<int>(*_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_ADD, _fd, &_new_event);
    return (*this);
}

auto zpt::polling::upgrade(zpt::stream _stream, std::string const& _transport) -> zpt::polling& {
    std::unique_lock _sentry{ this->__poll_lock };
    this->__polled_streams_by_uri.erase(this->__polled_streams_by_uri.find(_stream->uri()));
    _stream->upgrade(_transport);
    this->__polled_streams_by_uri.insert(std::make_pair(_stream->uri(), _stream));
    return (*this);
}

auto zpt::polling::insert(zpt::stream _stream) -> zpt::polling& {
    zpt::epoll_event_t _new_event;
    _new_event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    _new_event.data.ptr = _stream.get();
    {
        std::unique_lock _sentry{ this->__poll_lock };
        this->__polled_streams.insert(std::make_pair(static_cast<int>(*_stream), _stream));
        this->__polled_streams_by_uuid.insert(std::make_pair(_stream->uuid(), _stream));
        this->__polled_streams_by_uri.insert(std::make_pair(_stream->uri(), _stream));
    }
    _stream->__muted = false;
    auto _fd = static_cast<int>(*_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_ADD, _fd, &_new_event);
    return (*this);
}

auto zpt::polling::erase(zpt::stream _stream) -> zpt::polling& {
    auto _fd = static_cast<int>(*_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
    {
        std::unique_lock _sentry{ this->__poll_lock };
        this->__polled_streams.erase(this->__polled_streams.find(_fd));
        this->__polled_streams_by_uuid.erase(this->__polled_streams_by_uuid.find(_stream->uuid()));
        this->__polled_streams_by_uri.erase(this->__polled_streams_by_uri.find(_stream->uri()));
    }
    _stream->shutdown();
    return (*this);
}

auto zpt::polling::get(int _stream_fd) const -> zpt::stream {
    std::shared_lock _sentry{ this->__poll_lock };
    auto _found = this->__polled_streams.find(_stream_fd);
    if (_found == this->__polled_streams.end()) {
        throw zpt::NoSuchElementException{ "No such stream" };
    }
    return _found->second;
}

auto zpt::polling::get(zpt::uuid const& _stream_id) const -> zpt::stream {
    std::shared_lock _sentry{ this->__poll_lock };
    auto _found = this->__polled_streams_by_uuid.find(_stream_id);
    if (_found == this->__polled_streams_by_uuid.end()) {
        throw zpt::NoSuchElementException{ "No such stream" };
    }
    return _found->second;
}

auto zpt::polling::get(std::string const& _uri) const -> zpt::stream {
    std::shared_lock _sentry{ this->__poll_lock };
    auto _found = this->__polled_streams_by_uri.find(_uri);
    if (_found == this->__polled_streams_by_uri.end()) {
        throw zpt::NoSuchElementException{ "No such stream" };
    }
    return _found->second;
}

auto zpt::polling::delegate(zpt::stream _stream) -> zpt::polling& {
#ifdef ALLOCATOR_DEBUG_MODE
    zpt::mem::print_still_allocated();
    zpt::mem::start_tracking();
#endif
    this->mute(_stream);
    for (auto& d : this->__delegates) {
        if (d(this->shared_from_this(), _stream)) { return (*this); }
    }
    this->unmute(_stream);
    malloc_trim(0);
    return (*this);
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

        if (_sd_watchdog_enabled) { sd_notify(0, "WATCHDOG=1"); }

        for (auto _k = 0; _k != _n_alive; ++_k) {
            auto _stream =
              static_cast<zpt::basic_stream*>(_epoll_events[_k].data.ptr)->shared_from_this();
            _epoll_events[_k].data.ptr = nullptr;

            if (((_epoll_events[_k].events & EPOLLPRI) == EPOLLPRI) ||
                ((_epoll_events[_k].events & EPOLLHUP) == EPOLLHUP) ||
                ((_epoll_events[_k].events & EPOLLERR) == EPOLLERR) ||
                ((_epoll_events[_k].events & EPOLLRDHUP) == EPOLLRDHUP)) {
                this->erase(_stream);
            }
            else if ((_epoll_events[_k].events & EPOLLIN) == EPOLLIN) { this->delegate(_stream); }
            else {
                expect(((_epoll_events[_k].events & EPOLLPRI) == EPOLLPRI) ||
                         ((_epoll_events[_k].events & EPOLLHUP) == EPOLLHUP) ||
                         ((_epoll_events[_k].events & EPOLLERR) == EPOLLERR) ||
                         ((_epoll_events[_k].events & EPOLLRDHUP) == EPOLLRDHUP) ||
                         ((_epoll_events[_k].events & EPOLLIN) == EPOLLIN),
                       "unrecognized polling event");
            }
        }

    } while (!this->__shutdown.load());
    this->close();
    return (*this);
}

auto zpt::polling::shutdown() -> zpt::polling& {
    this->__shutdown.store(true);
    return (*this);
}

auto zpt::polling::is_in_shutdown() const -> bool { return this->__shutdown.load(); }

auto zpt::STREAM_POLLING() -> zpt::polling::ptr {
    static zpt::polling::ptr _global = zpt::allocate_shared<zpt::polling>();
    return _global;
}
