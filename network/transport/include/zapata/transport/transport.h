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

#include <zapata/streams.h>
#include <zapata/json.h>
#include <zapata/lockfree.h>

namespace zpt {
class exchange {
  private:
    class exchange_t {
      public:
        exchange_t(zpt::stream* _stream);
        exchange_t(zpt::exchange::exchange_t const& _rhs) = delete;
        exchange_t(zpt::exchange::exchange_t&& _rhs) = delete;
        virtual ~exchange_t() = default;

        auto operator=(zpt::exchange::exchange_t const& _rhs)
          -> zpt::exchange::exchange_t& = delete;
        auto operator=(zpt::exchange::exchange_t&& _rhs) -> zpt::exchange::exchange_t& = delete;

        auto stream() -> zpt::stream&;
        auto uri() -> std::string&;
        auto version() -> std::string&;
        auto scheme() -> std::string&;
        auto method() -> zpt::performative&;
        auto options() -> zpt::json&;
        auto headers() -> zpt::json&;
        auto received() -> zpt::json&;
        auto to_send() -> zpt::json&;
        auto status() -> zpt::status&;
        auto keep_alive() -> bool&;

      private:
        zpt::stream* __stream{ nullptr };
        std::string __uri;
        std::string __version;
        std::string __scheme;
        zpt::performative __method{ 0 };
        zpt::json __options{ zpt::json::object() };
        zpt::json __headers{ zpt::json::object() };
        zpt::json __received{ zpt::undefined };
        zpt::json __send{ zpt::undefined };
        zpt::status __status{ 404 };
        bool __keep_alive{ false };
    };

  public:
    exchange() = default;
    exchange(zpt::stream* _stream);
    exchange(zpt::exchange const& _rhs);
    exchange(zpt::exchange&& _rhs);
    virtual ~exchange() = default;

    auto operator=(zpt::exchange const& _rhs) -> zpt::exchange&;
    auto operator=(zpt::exchange&& _rhs) -> zpt::exchange&;

    auto operator-> () -> zpt::exchange::exchange_t*;
    auto operator*() -> zpt::exchange::exchange_t&;

    friend auto operator<<(std::ostream& _out, zpt::exchange& _in) -> std::ostream& {
        _out << _in->method() << " " << _in->scheme() << " " << _in->uri() << std::endl
             << zpt::pretty(_in->headers()) << std::endl
             << zpt::pretty(_in->received()) << std::flush;
        return _out;
    }

  private:
    std::shared_ptr<zpt::exchange::exchange_t> __underlying;
};

class transport {
  public:
    class transport_t {
      public:
        virtual auto receive_request(zpt::exchange& _channel) -> void = 0;
        virtual auto send_reply(zpt::exchange& _channel) -> void = 0;
        virtual auto send_request(zpt::exchange& _channel) -> void = 0;
        virtual auto receive_reply(zpt::exchange& _channel) -> void = 0;
        virtual auto resolve(zpt::json _uri) -> zpt::exchange = 0;
    };

    transport(zpt::transport const& _rhs);
    transport(zpt::transport&& _rhs);
    virtual ~transport() = default;

    auto operator=(zpt::transport const& _rhs) -> zpt::transport&;
    auto operator=(zpt::transport&& _rhs) -> zpt::transport&;

    auto operator-> () -> zpt::transport::transport_t*;
    auto operator*() -> zpt::transport::transport_t&;

    template<typename T, typename... Args>
    static auto alloc(Args... _args) -> zpt::transport;

    class layer {
      public:
        layer() = default;
        virtual ~layer() = default;

        auto add(std::string const& _scheme, zpt::transport _transport) -> zpt::transport::layer&;
        auto get(std::string const& _scheme) -> zpt::transport&;

        auto begin() -> std::map<std::string, zpt::transport>::iterator;
        auto end() -> std::map<std::string, zpt::transport>::iterator;

        auto resolve(std::string _uri) -> zpt::exchange;

      private:
        std::map<std::string, zpt::transport> __underlying;
    };

  private:
    transport(std::unique_ptr<zpt::transport::transport_t> _underlying);

    std::shared_ptr<zpt::transport::transport_t> __underlying;
};
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::transport::alloc(Args... _args) -> zpt::transport {
    return zpt::transport{ std::make_unique<T>(_args...) };
}
