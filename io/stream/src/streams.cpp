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

namespace {
constexpr std::uint64_t POLL_WAIT_TIMEOUT{ 100000 };
}

auto zpt::STREAM_POLLING() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::basic_stream::basic_stream(std::ios& _rhs)
  : __underlying{ std::make_unique<std::stringstream>() } {
    this->__underlying->rdbuf(_rhs.rdbuf());
}

zpt::basic_stream::basic_stream(std::unique_ptr<std::iostream> _underlying)
  : __underlying{ _underlying.release() }
  , __fd{ -1 } {}

zpt::basic_stream::~basic_stream() { this->close(); }

auto zpt::basic_stream::operator=(int _rhs) -> zpt::basic_stream& {
    this->__fd = _rhs;
    return (*this);
}

auto zpt::basic_stream::operator<<(ostream_manipulator _in) -> zpt::basic_stream& {
    (*this->__underlying.get()) << _in;
    return (*this);
}

auto zpt::basic_stream::operator->() -> std::iostream* { return this->__underlying.get(); }

auto zpt::basic_stream::operator*() -> std::iostream& { return *this->__underlying.get(); }

zpt::basic_stream::operator int() { return this->__fd; }

auto zpt::basic_stream::close() -> zpt::basic_stream& {
    zlog("Closing connection to " << this->uri(), zpt::trace);
    this->__underlying.reset(nullptr);
    this->__fd = -1;
    this->__transport = "";
    this->__uri = "";
    this->__state = zpt::stream_state::IDLE;
    return (*this);
}

auto zpt::basic_stream::transport(const std::string& _rhs) -> zpt::basic_stream& {
    this->__transport = _rhs;
    return (*this);
}

auto zpt::basic_stream::transport() -> std::string& { return this->__transport; }

auto zpt::basic_stream::uri(const std::string& _rhs) -> zpt::basic_stream& {
    this->__uri = _rhs;
    return (*this);
}

auto zpt::basic_stream::uri() -> std::string& { return this->__uri; }

auto zpt::basic_stream::state() -> zpt::stream_state& { return this->__state; }

zpt::polling::polling()
  : __epoll_fd{ epoll_create(1) } {}

zpt::polling::~polling() { // ::close(this->__epoll_fd);
}

auto zpt::polling::register_delegate(delegate_fn_type _callback) -> zpt::polling& {
    this->__delegates.push_back(_callback);
    return (*this);
}

auto zpt::polling::listen_on(zpt::stream _stream) -> zpt::polling& {
    if (!this->__shutdown.load()) {
        this->unmute(*_stream);
        {
            zpt::locks::spin_lock::guard _sentry{ this->__poll_lock, zpt::locks::spin_lock::exclusive };
            this->__polled_streams.emplace(static_cast<int>(*_stream), std::move(_stream));
        }
    }
    return (*this);
}

auto zpt::polling::erase(zpt::basic_stream& _stream) -> zpt::polling& {
    auto _fd = static_cast<int>(_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
    {
        zpt::locks::spin_lock::guard _sentry{ this->__poll_lock, zpt::locks::spin_lock::exclusive };
        this->__polled_streams.erase(this->__polled_streams.find(_fd));
    }
    return (*this);
}

auto zpt::polling::mute(zpt::basic_stream& _stream) -> zpt::polling& {
    auto _fd = static_cast<int>(_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_DEL, _fd, nullptr);
    return (*this);
}

auto zpt::polling::unmute(zpt::basic_stream& _stream) -> zpt::polling& {
    zpt::epoll_event_t _new_event;
    _new_event.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    _new_event.data.ptr = static_cast<void*>(&_stream);

    auto _fd = static_cast<int>(_stream);
    epoll_ctl(this->__epoll_fd, EPOLL_CTL_ADD, _fd, &_new_event);

    return (*this);
}

auto zpt::polling::delegate(zpt::basic_stream& _stream) -> zpt::polling& {
    this->mute(_stream);
    for (auto& d : this->__delegates) {
        if (d((*this), _stream)) { return (*this); }
    }
    this->unmute(_stream);
    return (*this);
}

auto zpt::polling::poll() -> void {
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
            auto _stream = static_cast<zpt::basic_stream*>(_epoll_events[_k].data.ptr);

            if (((_epoll_events[_k].events & EPOLLPRI) == EPOLLPRI) ||
                ((_epoll_events[_k].events & EPOLLHUP) == EPOLLHUP) ||
                ((_epoll_events[_k].events & EPOLLERR) == EPOLLERR) ||
                ((_epoll_events[_k].events & EPOLLRDHUP) == EPOLLRDHUP)) {
                this->erase(*_stream);
            }
            else if ((_epoll_events[_k].events & EPOLLIN) == EPOLLIN) { this->delegate(*_stream); }
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
}

auto zpt::polling::shutdown() -> void { this->__shutdown.store(true); }
