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

#include <zapata/globals.h>
#include <zapata/json.h>
#include <zapata/ontology.h>
#include <zapata/streams.h>

namespace zpt {

enum transport_capability { SYNCHRONOUS = 1, PERSISTENT = 2 };

class basic_transport {
  public:
    basic_transport() = default;
    virtual ~basic_transport() = default;

    virtual auto has_capability(std::uint64_t _capability) const -> bool = 0;
    virtual auto make_request() const -> zpt::message = 0;
    virtual auto make_reply(bool _with_allocator = true) const -> zpt::message = 0;
    virtual auto make_reply(zpt::message _request) const -> zpt::message = 0;
    virtual auto process_incoming_request(zpt::stream _stream) const -> zpt::message = 0;
    virtual auto process_incoming_reply(zpt::stream _stream) const -> zpt::message = 0;
    virtual auto receive(zpt::stream _stream) const -> zpt::message final;
    virtual auto send(zpt::stream _stream, zpt::message _to_send) const -> void final;
};
using transport = std::shared_ptr<basic_transport>;

namespace network {
class layer {
  public:
    using translate_from_func = std::function<zpt::json(std::istream&)>;
    using translate_to_func = std::function<std::string(std::ostream&, zpt::json)>;

    layer(zpt::json _global_config);
    virtual ~layer() = default;

    auto add(std::string const& _scheme, zpt::transport _transport) -> layer&;
    auto get(std::string const& _scheme) const -> const zpt::transport;
    auto remove(std::string const& _scheme) -> layer&;
    auto clear() -> layer&;

    auto translate(std::istream& _io, std::string _mime = "*/*") const -> zpt::json;
    auto translate(std::ostream& _io, std::string _mime, zpt::json _content) const -> std::string;

    auto begin() const -> std::map<std::string, zpt::transport>::const_iterator;
    auto end() const -> std::map<std::string, zpt::transport>::const_iterator;

    auto resolve(std::string _uri) const -> zpt::transport;

  private:
    std::map<std::string, zpt::transport> __underlying;
    std::map<std::string, std::tuple<translate_from_func, translate_to_func>> __content_providers;
    zpt::json __configuration;

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

auto resolve_content_type(zpt::basic_message const& _message) -> std::string;
} // namespace network

auto TRANSPORT_LAYER(zpt::json _config = nullptr) -> zpt::network::layer&;
template<typename T, typename... Args>
auto make_transport(Args... _args) -> zpt::transport;
} // namespace zpt

template<typename T>
auto zpt::json_message::operator<<(T _to_add) -> zpt::json_message& {
    if (!this->__underlying("body")->ok()) { this->__underlying["body"] = zpt::json::object(); }
    this->__underlying["body"] << _to_add;
    return (*this);
}

template<typename T, typename... Args>
auto zpt::make_transport(Args... _args) -> zpt::transport {
    return std::allocate_shared<T>(zpt::allocator<T>{ zpt::MEM_POOL() },
                                   std::forward<Args>(_args)...);
}
