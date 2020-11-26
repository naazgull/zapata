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

#include <zapata/streams/streams.h>
#include <systemd/sd-daemon.h>
#include <errno.h>

auto
zpt::STREAM_POLLING() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::stream::stream(std::ios& _rhs)
  : __underlying{ std::make_unique<std::stringstream>() } {
    this->__underlying->rdbuf(_rhs.rdbuf());
}

zpt::stream::~stream() { this->close(); }

auto
zpt::stream::operator=(int _rhs) -> zpt::stream& {
    this->__fd = _rhs;
    return (*this);
}

auto
zpt::stream::operator<<(ostream_manipulator _in) -> zpt::stream& {
    (*this->__underlying.get()) << _in;
    return (*this);
}

auto
zpt::stream::operator->() -> std::iostream* {
    return this->__underlying.get();
}

auto
zpt::stream::operator*() -> std::iostream& {
    return *this->__underlying.get();
}

zpt::stream::operator int() { return this->__fd; }

auto
zpt::stream::close() -> zpt::stream& {
    zlog("Closing connection to " << this->uri(), zpt::trace);
    this->__underlying.reset(nullptr);
    this->__fd = -1;
    this->__transport = "";
    this->__uri = "";
    this->__state = zpt::stream_state::IDLE;
    return (*this);
}

auto
zpt::stream::transport(const std::string& _rhs) -> zpt::stream& {
    this->__transport = _rhs;
    return (*this);
}

auto
zpt::stream::transport() -> std::string& {
    return this->__transport;
}

auto
zpt::stream::uri(const std::string& _rhs) -> zpt::stream& {
    this->__uri = _rhs;
    return (*this);
}

auto
zpt::stream::uri() -> std::string& {
    return this->__uri;
}

auto
zpt::stream::state() -> zpt::stream_state& {
    return this->__state;
}

auto
zpt::stream::swap(std::ios& _rhs) -> zpt::stream& {
    this->__underlying->rdbuf(_rhs.rdbuf());
    return (*this);
}

auto
zpt::stream::swap(zpt::stream& _rhs) -> zpt::stream& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto
zpt::stream::swap(std::unique_ptr<zpt::stream>& _rhs) -> zpt::stream& {
    this->__underlying = std::move(_rhs->__underlying);
    return (*this);
}

zpt::stream::stream(std::unique_ptr<std::iostream> _underlying)
  : __underlying{ _underlying.release() }
  , __fd{ -1 } {}

zpt::stream::polling::polling(long _max_stream_readers, long _poll_wait_timeout)
  : __epoll_fd{ epoll_create(1) }
  , __poll_wait_timeout{ _poll_wait_timeout }
  , __hazard_domain{ _max_stream_readers, 8 }
  , __alive_streams{ __hazard_domain, 0 } {}

zpt::stream::polling::~polling() { ::close(this->__epoll_fd); }

auto
zpt::stream::polling::listen_on(std::unique_ptr<zpt::stream>& _stream) -> zpt::stream::polling& {
    if (!this->__shutdown.load()) {
        zpt::stream* _new_stream = _stream.release();

        zpt::epoll_event_t _new_event;
        _new_event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
        _new_event.data.ptr = static_cast<void*>(_new_stream);

        epoll_ctl(this->__epoll_fd, EPOLL_CTL_ADD, static_cast<int>(*_new_stream), &_new_event);
    }
    return (*this);
}

auto
zpt::stream::polling::mute(zpt::stream& _stream) -> zpt::stream::polling& {
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, static_cast<int>(_stream), nullptr);
    return (*this);
}

auto
zpt::stream::polling::pool() -> void {
    std::uint64_t _sd_watchdog_usec = 100000;
    bool _sd_watchdog_enabled = sd_watchdog_enabled(0, &_sd_watchdog_usec) != 0;
    long long _poll_timeout =
      _sd_watchdog_enabled ? this->__poll_wait_timeout > 0
                               ? std::min(this->__poll_wait_timeout / 1000,
                                          static_cast<long long>(_sd_watchdog_usec / 1000 / 2))
                               : static_cast<long long>(_sd_watchdog_usec / 1000 / 2)
                           : std::min(this->__poll_wait_timeout / 1000, this->__poll_wait_timeout);

    do {
        int _n_alive =
          epoll_wait(this->__epoll_fd, this->__epoll_events, MAX_EVENT_PER_POLL, _poll_timeout);
        if (this->__shutdown.load()) { return; }
        if (_n_alive < 0) { continue; }

        if (_sd_watchdog_enabled) { sd_notify(0, "WATCHDOG=1"); }

        for (int _k = 0; _k != _n_alive; ++_k) {
            zpt::stream* _stream = static_cast<zpt::stream*>(this->__epoll_events[_k].data.ptr);

            if (((this->__epoll_events[_k].events & EPOLLPRI) == EPOLLPRI) ||
                ((this->__epoll_events[_k].events & EPOLLHUP) == EPOLLHUP) ||
                ((this->__epoll_events[_k].events & EPOLLERR) == EPOLLERR) ||
                ((this->__epoll_events[_k].events & EPOLLRDHUP) == EPOLLRDHUP)) {
                std::unique_ptr<zpt::stream> _to_dispose{ _stream };
                this->mute(*_stream);
            }
            else if ((this->__epoll_events[_k].events & EPOLLIN) == EPOLLIN) {
                this->mute(*_stream);
                this->__alive_streams.push(_stream);
            }
            else {
                expect(((this->__epoll_events[_k].events & EPOLLPRI) == EPOLLPRI) ||
                         ((this->__epoll_events[_k].events & EPOLLHUP) == EPOLLHUP) ||
                         ((this->__epoll_events[_k].events & EPOLLERR) == EPOLLERR) ||
                         ((this->__epoll_events[_k].events & EPOLLRDHUP) == EPOLLRDHUP) ||
                         ((this->__epoll_events[_k].events & EPOLLIN) == EPOLLIN),
                       "unrecognized polling event",
                       500,
                       0);
            }
        }

    } while (true);
}

auto
zpt::stream::polling::pop() -> zpt::stream* {
    return this->__alive_streams.pop();
}

auto
zpt::stream::polling::shutdown() -> void {
    expect(!this->__shutdown.load(), "Stream polling shutdown already started elsewhere", 500, 0);
    this->__shutdown.store(true);
    for (auto [_fd, _stream] : this->__polled_streams) {
        epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
        std::unique_ptr<zpt::stream> _to_erase{ _stream };
        zlog("Closing connection to " << _stream->uri(), zpt::trace);
    }
    zlog("Closing stream polling", zpt::info);
}
