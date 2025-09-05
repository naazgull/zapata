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
#include <atomic>
#include <sys/epoll.h>
#include <systemd/sd-daemon.h>
#include <zapata/text/convert.h>
#include <zapata/locks/spin_mutex.h>

namespace zpt {
enum class stream_state { IDLE, WAITING, PROCESSING, ERRORING_OUT };
using epoll_event_t = struct epoll_event;
class polling;

class basic_stream {
  public:
    typedef std::ostream& (*ostream_manipulator)(std::ostream&);
    friend class polling;

    basic_stream() = default;
    basic_stream(std::ios& _rhs);
    basic_stream(std::unique_ptr<std::iostream> _underlying);
    basic_stream(basic_stream const& _rhs) = delete;
    basic_stream(basic_stream&& _rhs) = delete;
    virtual ~basic_stream();

    auto operator=(basic_stream const& _rhs) -> basic_stream& = delete;
    auto operator=(basic_stream&& _rhs) -> basic_stream& = delete;

    virtual auto operator=(int _rhs) -> basic_stream&;
    template<typename T>
    auto read(T& _out) -> basic_stream&;
    template<typename T>
    auto write(T _in) -> basic_stream&;
    template<typename T>
    auto operator>>(T& _out) -> basic_stream&;
    template<typename T>
    auto operator<<(T _in) -> basic_stream&;
    auto operator<<(ostream_manipulator _in) -> basic_stream&;
    auto operator*() -> std::iostream&;

    virtual operator int();

    template<typename IOStream>
    auto set_peer(std::string const& _address, unsigned int _port) -> basic_stream&;
    virtual auto close() -> basic_stream&;
    virtual auto shutdown() -> basic_stream&;
    virtual auto transport(const std::string& _rhs) -> basic_stream&;
    virtual auto transport() -> std::string&;
    virtual auto uri(const std::string& _rhs) -> basic_stream&;
    virtual auto uri() -> std::string&;
    virtual auto state() -> stream_state&;

  protected:
    std::unique_ptr<std::iostream> __underlying{ nullptr };
    int __fd{ -1 };
    std::string __transport{ "" };
    std::string __uri{ "" };
    zpt::stream_state __state{ zpt::stream_state::IDLE };
    bool __muted{ true };

    auto extract_uri() -> void;
};

using stream = std::shared_ptr<zpt::basic_stream>;

class polling : public std::enable_shared_from_this<polling> {
  public:
    using ptr = std::shared_ptr<polling>;
    using delegate_fn_type = std::function<bool(zpt::polling::ptr _poll, zpt::stream _stream)>;
    constexpr static int MAX_EVENT_PER_POLL{ 100 };

    polling();
    virtual ~polling();

    auto close() -> zpt::polling&;
    auto register_delegate(delegate_fn_type _callback) -> zpt::polling&;
    auto listen_on(zpt::stream _stream) -> zpt::polling&;
    auto mute(zpt::stream _stream) -> zpt::polling&;
    auto unmute(zpt::stream _stream) -> zpt::polling&;

    auto poll() -> zpt::polling&;
    auto shutdown() -> zpt::polling&;
    auto is_in_shutdown() const -> bool;

  private:
    int __epoll_fd{ -1 };
    zpt::locks::spin_mutex __poll_lock{};
    std::map<int, zpt::stream> __polled_streams;
    std::vector<delegate_fn_type> __delegates;
    std::atomic<bool> __shutdown{ false };

    auto erase(zpt::stream _stream) -> zpt::polling&;
    auto delegate(zpt::stream _stream) -> zpt::polling&;
};

auto STREAM_POLLING() -> zpt::polling::ptr;

template<typename T, typename... Args>
static auto make_stream(Args... _args) -> zpt::stream;

#define CRLF "\r\n"

template<typename T>
auto stream_cast(zpt::stream& _rhs) -> T& {
    return static_cast<T&>(**_rhs);
}
} // namespace zpt

template<typename T>
auto zpt::basic_stream::read(T& _out) -> zpt::basic_stream& {
    auto& _underlying = *this->__underlying.get();
    if constexpr (!std::is_same<T, std::string>::value && std::is_class<T>::value) {
        _out->from_stream(_underlying);
    }
    else { _underlying >> _out; }
    return (*this);
}

template<typename T>
auto zpt::basic_stream::write(T _in) -> zpt::basic_stream& {
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
    zpt::stream _to_return{ new zpt::basic_stream{ std::make_unique<T>(_args...) } };
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
