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

#pragma once

#include <iostream>
#include <memory>
#include <sys/epoll.h>
#include <systemd/sd-daemon.h>
#include <zapata/text/convert.h>
#include <zapata/lockfree/queue.h>
#include <zapata/lockfree/spin_lock.h>

namespace zpt {

using epoll_event_t = struct epoll_event;

auto
STREAM_POLLING() -> ssize_t&;

enum class stream_state { IDLE, WAITING, PROCESSING };

class stream {
  public:
    typedef std::ostream& (*ostream_manipulator)(std::ostream&);

    class polling {
      public:
        constexpr static int MAX_EVENT_PER_POLL{ 10 };

        polling(long _max_stream_readers, long _poll_wait_timeout = -1);
        virtual ~polling();

        auto listen_on(std::unique_ptr<zpt::stream>& _stream) -> zpt::stream::polling&;
        auto mute(zpt::stream& _stream) -> zpt::stream::polling&;
        auto pool() -> void;
        auto pop() -> zpt::stream*;
        auto shutdown() -> void;

      private:
        int __epoll_fd{ -1 };
        long long __poll_wait_timeout{ 0 };
        zpt::epoll_event_t __epoll_events[MAX_EVENT_PER_POLL];
        zpt::lf::queue<zpt::stream*>::hazard_domain __hazard_domain;
        zpt::lf::queue<zpt::stream*> __alive_streams;
        std::map<int, zpt::stream*> __polled_streams;
        std::atomic<bool> __shutdown{ false };
    };

    stream() = default;
    stream(std::ios& _rhs);
    stream(zpt::stream const& _rhs) = delete;
    stream(zpt::stream&& _rhs) = delete;
    virtual ~stream();

    auto operator=(zpt::stream const& _rhs) -> zpt::stream& = delete;
    auto operator=(zpt::stream&& _rhs) -> zpt::stream& = delete;

    auto operator=(int _rhs) -> zpt::stream&;
    template<typename T>
    auto operator>>(T& _out) -> zpt::stream&;
    template<typename T>
    auto operator<<(T _in) -> zpt::stream&;
    auto operator<<(ostream_manipulator _in) -> zpt::stream&;
    auto operator->() -> std::iostream*;
    auto operator*() -> std::iostream&;

    operator int();

    auto close() -> zpt::stream&;
    auto transport(const std::string& _rhs) -> zpt::stream&;
    auto transport() -> std::string&;
    auto uri(const std::string& _rhs) -> zpt::stream&;
    auto uri() -> std::string&;
    auto state() const -> zpt::stream_state&;

    auto swap(std::ios& _rhs) -> zpt::stream&;
    auto swap(zpt::stream& _rhs) -> zpt::stream&;
    auto swap(std::unique_ptr<zpt::stream>& _rhs) -> zpt::stream&;
    template<typename T, typename... Args>
    auto swap(Args... _args) -> zpt::stream&;

    template<typename T, typename... Args>
    static auto alloc(Args... _args) -> std::unique_ptr<zpt::stream>;

  private:
    std::unique_ptr<std::iostream> __underlying{ nullptr };
    int __fd{ -1 };
    std::string __transport{ "" };
    std::string __uri{ "" };
    zpt::lf::spin_lock __input_lock;
    zpt::lf::spin_lock __output_lock;
    zpt::stream_state __state{ zpt::stream_state::IDLE };

    stream(std::unique_ptr<std::iostream> _underlying);
};

#define CRLF "\r\n"
} // namespace zpt

template<typename T>
auto
stream_cast(zpt::stream& _rhs) -> T& {
    return *static_cast<T*>(&(*_rhs));
}

template<typename T>
auto
zpt::stream::operator>>(T& _out) -> zpt::stream& {
    zpt::lf::spin_lock::guard _sentry{ this->__input_lock, zpt::lf::spin_lock::exclusive };
    (*this->__underlying.get()) >> _out;
    return (*this);
}

template<typename T>
auto
zpt::stream::operator<<(T _in) -> zpt::stream& {
    zpt::lf::spin_lock::guard _sentry{ this->__output_lock, zpt::lf::spin_lock::exclusive };
    (*this->__underlying.get()) << _in;
    return (*this);
}

template<typename T, typename... Args>
auto
zpt::stream::swap(Args... _args) -> zpt::stream& {
    this->__underlying.swap(std::make_unique<T>(_args...));
    return (*this);
}

template<typename T, typename... Args>
auto
zpt::stream::alloc(Args... _args) -> std::unique_ptr<zpt::stream> {
    std::unique_ptr<zpt::stream> _to_return{ new zpt::stream{ std::make_unique<T>(_args...) } };
    if constexpr (std::is_convertible<T, int>::value) {
        (*_to_return) = static_cast<int>(static_cast<T&>(**_to_return));
    }
    if constexpr (std::is_convertible<T, std::string>::value) {
        _to_return->uri(static_cast<std::string>(static_cast<T&>(**_to_return)));
    }
    expect(!(**_to_return.get()).fail() && !(**_to_return.get()).bad(),
           "unable to open underlying `std::iostream` named '" << _to_return->uri() << "'",
           500,
           0);
    return _to_return;
}
