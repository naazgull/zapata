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
class message;
class transport {
  public:
    class transport_t {
      public:
        virtual auto receive(zpt::message& _out) -> void = 0;
        virtual auto send(zpt::message& _message) -> void = 0;
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
        layer();
        virtual ~layer() = default;

        auto add(std::string _scheme, zpt::transport _transport) -> zpt::transport::layer&;
        auto get(std::string _scheme) -> zpt::transport&;

        auto push_stream(std::string _scheme, zpt::stream _stream) -> zpt::transport::layer&;
        auto pop_stream() -> std::tuple<std::string, zpt::stream>;

        auto begin() -> std::map<std::string, zpt::transport>::iterator;
        auto end() -> std::map<std::string, zpt::transport>::iterator;

        auto on_load(std::function<void(zpt::transport&)> _listener) -> zpt::transport::layer&;

      private:
        std::map<std::string, zpt::transport> __underlying;
        std::vector<std::function<void(zpt::transport&)>> __on_load;
        zpt::lf::queue<std::tuple<std::string, zpt::stream>> __streams;
    };

  private:
    transport(std::unique_ptr<zpt::transport::transport_t> _underlying);

    std::shared_ptr<zpt::transport::transport_t> __underlying;
};

class message {
  private:
    class message_t {
      public:
        message_t(zpt::stream _stream);
        message_t(zpt::message::message_t const& _rhs) = delete;
        message_t(zpt::message::message_t&& _rhs) = delete;
        virtual ~message_t() = default;

        auto operator=(zpt::message::message_t const& _rhs) -> zpt::message::message_t& = delete;
        auto operator=(zpt::message::message_t&& _rhs) -> zpt::message::message_t& = delete;

        auto stream() -> zpt::stream&;
        auto uri() -> std::string&;
        auto version() -> std::string&;
        auto scheme() -> std::string&;
        auto method() -> zpt::performative&;
        auto headers() -> zpt::json&;
        auto received() -> zpt::json&;
        auto to_send() -> zpt::json&;
        auto status() -> zpt::status&;

      private:
        zpt::stream __stream;
        std::string __uri;
        std::string __version;
        std::string __scheme;
        zpt::performative __method{ 0 };
        zpt::json __headers;
        zpt::json __received{ zpt::json::object() };
        zpt::json __send{ zpt::json::object() };
        zpt::status __status{ 204 };
    };

  public:
    message(zpt::stream _stream);
    message(zpt::message const& _rhs);
    message(zpt::message&& _rhs);
    virtual ~message() = default;

    auto operator=(zpt::message const& _rhs) -> zpt::message&;
    auto operator=(zpt::message&& _rhs) -> zpt::message&;

    auto operator-> () -> zpt::message::message_t*;
    auto operator*() -> zpt::message::message_t&;

  private:
    std::shared_ptr<zpt::message::message_t> __underlying;
};
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::transport::alloc(Args... _args) -> zpt::transport {
    return zpt::transport{ std::make_unique<T>(_args...) };
}
