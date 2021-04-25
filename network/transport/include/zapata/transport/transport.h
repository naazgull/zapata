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
auto
TRANSPORT_LAYER() -> ssize_t&;

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
        auto received() -> zpt::json&;
        auto to_send() -> zpt::json&;
        auto keep_alive() -> bool&;

      private:
        zpt::stream* __stream{ nullptr };
        std::string __uri;
        std::string __version;
        std::string __scheme;
        zpt::json __received{ zpt::undefined };
        zpt::json __send{ zpt::undefined };
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

    auto operator->() const -> zpt::exchange::exchange_t*;
    auto operator*() const -> zpt::exchange::exchange_t&;

    friend auto operator<<(std::ostream& _out, zpt::exchange& _in) -> std::ostream& {
        _out << _in->scheme() << ":" << _in->uri() << std::endl
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
        transport_t() = default;
        transport_t(transport_t const&) = delete;
        transport_t(transport_t&&) = delete;
        virtual ~transport_t() = default;
        
        auto operator=(transport_t const&) -> transport_t& = delete;
        auto operator=(transport_t&&) -> transport_t& = delete;

        virtual auto receive(zpt::exchange& _channel) const -> void = 0;
        virtual auto send(zpt::exchange& _channel) const -> void = 0;
        virtual auto resolve(zpt::json _uri) const -> zpt::exchange = 0;
    };

    transport(zpt::transport const& _rhs);
    transport(zpt::transport&& _rhs);
    virtual ~transport() = default;

    auto operator=(zpt::transport const& _rhs) -> zpt::transport&;
    auto operator=(zpt::transport&& _rhs) -> zpt::transport&;

    auto operator->() const -> zpt::transport::transport_t*;
    auto operator*() const -> zpt::transport::transport_t&;

    template<typename T, typename... Args>
    static auto alloc(Args... _args) -> zpt::transport;

    class layer {
      public:
        layer();
        virtual ~layer() = default;

        auto add(std::string const& _scheme, zpt::transport _transport) -> zpt::transport::layer&;
        auto get(std::string const& _scheme) const -> const zpt::transport&;

        auto translate(std::istream& _io, std::string _mime = "*/*") const -> zpt::json;

        auto begin() const -> std::map<std::string, zpt::transport>::const_iterator;
        auto end() const -> std::map<std::string, zpt::transport>::const_iterator;

        auto resolve(std::string _uri) const -> zpt::exchange;

      private:
        std::map<std::string, zpt::transport> __underlying;
        std::map<std::string, std::function<zpt::json(std::istream&)>> __content_providers;

        auto add_content_provider(std::string const& _scheme,
                                  std::function<zpt::json(std::istream&)> _callback)
          -> zpt::transport::layer&;
        static auto translate_from_default(std::istream& _io) -> zpt::json;
        static auto translate_from_json(std::istream& _io) -> zpt::json;
        static auto translate_from_raw(std::istream& _io) -> zpt::json;
    };

  private:
    std::shared_ptr<zpt::transport::transport_t> __underlying;

    transport(std::unique_ptr<zpt::transport::transport_t> _underlying);
};
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::transport::alloc(Args... _args) -> zpt::transport {
    return zpt::transport{ std::make_unique<T>(_args...) };
}
