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

namespace zpt {
auto
TRANSPORT_LAYER() -> ssize_t&;

class basic_message {
  public:
    basic_message() = default;
    basic_message(basic_message const& _req, bool);
    virtual ~basic_message() = default;

    virtual auto performative() const -> zpt::performative = 0;
    virtual auto status() const -> zpt::status = 0;
    virtual auto uri() -> zpt::json& = 0;
    virtual auto uri() const -> zpt::json const = 0;
    virtual auto version() const -> std::string = 0;
    virtual auto scheme() const -> std::string = 0;
    virtual auto resource() const -> zpt::json const = 0;
    virtual auto parameters() const -> zpt::json const = 0;
    virtual auto headers() -> zpt::json& = 0;
    virtual auto headers() const -> zpt::json const = 0;
    virtual auto body() -> zpt::json& = 0;
    virtual auto body() const -> zpt::json const = 0;
    virtual auto keep_alive() const -> bool = 0;
    virtual auto content_type() const -> std::string = 0;
    virtual auto performative(zpt::performative _performative) -> void = 0;
    virtual auto status(zpt::status _status) -> void = 0;
    virtual auto uri(std::string const& _uri) -> void = 0;
    virtual auto version(std::string const& _version) -> void = 0;
    virtual auto to_stream(std::ostream& _out) const -> void = 0;
    virtual auto from_stream(std::istream& _in) -> void = 0;
};
using message = std::shared_ptr<basic_message>;

class json_message : public basic_message {
  public:
    json_message() = default;
    json_message(basic_message const& _req, bool);
    virtual ~json_message() = default;

    auto performative() const -> zpt::performative override;
    auto status() const -> zpt::status override;
    auto uri() -> zpt::json& override;
    auto uri() const -> zpt::json const override;
    auto version() const -> std::string override;
    auto scheme() const -> std::string override;
    auto resource() const -> zpt::json const override;
    auto parameters() const -> zpt::json const override;
    auto headers() -> zpt::json& override;
    auto headers() const -> zpt::json const override;
    auto body() -> zpt::json& override;
    auto body() const -> zpt::json const override;
    auto keep_alive() const -> bool override;
    auto content_type() const -> std::string override;
    auto to_stream(std::ostream& _out) const -> void override;
    auto from_stream(std::istream& _in) -> void override;
    auto performative(zpt::performative _performative) -> void override;
    auto status(zpt::status _status) -> void override;
    auto uri(std::string const& _uri) -> void override;
    auto version(std::string const& _version) -> void override;
    template<typename T>
    auto operator<<(T _to_add) -> zpt::json_message&;

  private:
    zpt::json __underlying;
};

class basic_transport {
  public:
    basic_transport() = default;
    virtual ~basic_transport() = default;

    virtual auto make_request() const -> zpt::message = 0;
    virtual auto make_reply() const -> zpt::message = 0;
    virtual auto make_reply(zpt::message _reuqest) const -> zpt::message = 0;
    virtual auto receive(zpt::basic_stream& _stream) const -> zpt::message = 0;
    virtual auto send(zpt::basic_stream& _stream, zpt::message _to_send) const -> void = 0;
};
using transport = std::shared_ptr<basic_transport>;

namespace network {
class layer {
  public:
    using translate_from_func = std::function<zpt::json(std::istream&)>;
    using translate_to_func = std::function<std::string(std::ostream&, zpt::json)>;

    layer();
    virtual ~layer() = default;

    auto add(std::string const& _scheme, zpt::transport _transport) -> layer&;
    auto get(std::string const& _scheme) const -> const zpt::transport;

    auto translate(std::istream& _io, std::string _mime = "*/*") const -> zpt::json;
    auto translate(std::ostream& _io, std::string _mime, zpt::json _content) const -> std::string;

    auto begin() const -> std::map<std::string, zpt::transport>::const_iterator;
    auto end() const -> std::map<std::string, zpt::transport>::const_iterator;

    auto resolve(std::string _uri) const -> zpt::transport;

  private:
    std::map<std::string, zpt::transport> __underlying;
    std::map<std::string, std::tuple<translate_from_func, translate_to_func>> __content_providers;

    auto add_content_provider(std::string const& _mime,
                              translate_from_func _callback_from,
                              translate_to_func _callback_to) -> layer&;
    static auto translate_from_default(std::istream& _io) -> zpt::json;
    static auto translate_to_default(std::ostream& _io, zpt::json _content) -> std::string;
    static auto translate_from_json(std::istream& _io) -> zpt::json;
    static auto translate_to_json(std::ostream& _io, zpt::json _content) -> std::string;
    static auto translate_from_raw(std::istream& _io) -> zpt::json;
    static auto translate_to_raw(std::ostream& _io, zpt::json _content) -> std::string;
    static auto translate_from_xml(std::istream& _io) -> zpt::json;
    static auto translate_to_xml(std::ostream& _io, zpt::json _content) -> std::string;
};

auto
resolve_content_type(zpt::basic_message const& _message) -> std::string;
} // namespace network

template<typename T, typename... Args>
auto
make_transport(Args... _args) -> zpt::transport;
template<typename T, typename... Args>
auto
make_message(Args... _args) -> zpt::message;
} // namespace zpt

auto
operator<<(std::ostream& _out, zpt::message _in) -> std::ostream&;
auto
operator>>(std::istream& _in, zpt::message _out) -> std::istream&;

template<typename T>
auto
message_cast(zpt::message _rhs) -> T& {
    return static_cast<T&>(*_rhs);
}

template<typename T>
auto
zpt::json_message::operator<<(T _to_add) -> zpt::json_message& {
    if (!this->__underlying("body")->ok()) { this->__underlying["body"] = zpt::json::object(); }
    this->__underlying["body"] << _to_add;
    return (*this);
}

template<typename T, typename... Args>
auto
zpt::make_transport(Args... _args) -> zpt::transport {
    return zpt::transport{ new T{ std::forward<Args>(_args)... } };
}

template<typename T, typename... Args>
auto
zpt::make_message(Args... _args) -> zpt::message {
    return zpt::message{ new T{ std::forward<Args>(_args)... } };
}
