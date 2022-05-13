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

#include <zapata/transport/transport.h>
#include <zapata/exceptions/NoMoreElementsException.h>
#include <zapata/uri/uri.h>

auto
zpt::TRANSPORT_LAYER() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::exchange::exchange(zpt::stream* _stream)
  : __underlying{ std::make_shared<zpt::exchange::exchange_t>(_stream) } {}

zpt::exchange::exchange(zpt::exchange const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::exchange::exchange(zpt::exchange&& _rhs)
  : __underlying{ std::move(_rhs.__underlying) } {}

auto
zpt::exchange::operator=(zpt::exchange const& _rhs) -> zpt::exchange& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::exchange::operator=(zpt::exchange&& _rhs) -> zpt::exchange& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto
zpt::exchange::operator->() const -> zpt::exchange::exchange_t* {
    return this->__underlying.get();
}

auto
zpt::exchange::operator*() const -> zpt::exchange::exchange_t& {
    return *this->__underlying.get();
}

zpt::exchange::exchange_t::exchange_t(zpt::stream* _stream)
  : __stream{ _stream }
  , __scheme{ _stream->transport() } {}

auto
zpt::exchange::exchange_t::stream() -> zpt::stream& {
    return *this->__stream;
}

auto
zpt::exchange::exchange_t::uri() -> std::string& {
    return this->__uri;
}

auto
zpt::exchange::exchange_t::version() -> std::string& {
    return this->__version;
}

auto
zpt::exchange::exchange_t::scheme() -> std::string& {
    return this->__scheme;
}

auto
zpt::exchange::exchange_t::received() -> zpt::json& {
    return this->__received;
}

auto
zpt::exchange::exchange_t::to_send() -> zpt::json& {
    return this->__send;
}

auto
zpt::exchange::exchange_t::keep_alive() -> bool& {
    return this->__keep_alive;
}

auto
zpt::exchange::exchange_t::content_type() -> zpt::json {
    zpt::json _return = zpt::json::array();
    if (this->__received["headers"]["Accept"]->ok()) {
        auto _accept = this->__received["headers"]["Accept"]->string();
        auto _mime_types = zpt::split(_accept, ",");
        for (auto [_, __, _mime] : _mime_types)
            _return << _mime->string().substr(0, _mime->string().find(";"));
    }
    if (this->__received["headers"]["Content-Type"]->ok()) {
        _return << this->__received["headers"]["Content-Type"];
    }
    _return << "application/json";
    return _return;
}

zpt::transport::transport(zpt::transport const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::transport::transport(zpt::transport&& _rhs)
  : __underlying{ std::move(_rhs.__underlying) } {}

auto
zpt::transport::operator=(zpt::transport const& _rhs) -> zpt::transport& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::transport::operator=(zpt::transport&& _rhs) -> zpt::transport& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto
zpt::transport::operator->() const -> zpt::transport::transport_t* {
    return this->__underlying.get();
}

auto
zpt::transport::operator*() const -> zpt::transport::transport_t& {
    return *this->__underlying.get();
}

zpt::transport::transport(std::unique_ptr<zpt::transport::transport_t> _underlying)
  : __underlying{ _underlying.release() } {}

zpt::transport::layer::layer() {
    this->add_content_provider("*/*",
                               zpt::transport::layer::translate_from_default,
                               zpt::transport::layer::translate_to_default);
    this->add_content_provider(
      "text", zpt::transport::layer::translate_from_raw, zpt::transport::layer::translate_to_raw);
    this->add_content_provider("text/plain",
                               zpt::transport::layer::translate_from_raw,
                               zpt::transport::layer::translate_to_raw);
    this->add_content_provider(
      "json", zpt::transport::layer::translate_from_json, zpt::transport::layer::translate_to_json);
    this->add_content_provider("application/json",
                               zpt::transport::layer::translate_from_json,
                               zpt::transport::layer::translate_to_json);
    this->add_content_provider("text/x-json",
                               zpt::transport::layer::translate_from_json,
                               zpt::transport::layer::translate_to_json);
    this->add_content_provider("text/xml",
                               zpt::transport::layer::translate_from_xml,
                               zpt::transport::layer::translate_to_xml);
}

auto
zpt::transport::layer::add(std::string const& _scheme, zpt::transport _transport)
  -> zpt::transport::layer& {
    this->__underlying.insert(std::make_pair(_scheme, _transport));
    return (*this);
}

auto
zpt::transport::layer::get(std::string const& _scheme) const -> const zpt::transport& {
    auto _found = this->__underlying.find(_scheme);
    if (_found == this->__underlying.end()) {
        throw zpt::NoMoreElementsException("there is no such transport");
    }
    return _found->second;
}

auto
zpt::transport::layer::translate(std::istream& _io, std::string _mime) const -> zpt::json {
    auto _found = this->__content_providers.find(_mime);
    if (_found != this->__content_providers.end()) { return std::get<0>(_found->second)(_io); }
    else { return zpt::transport::layer::translate_from_default(_io); }
    return zpt::undefined;
}

auto
zpt::transport::layer::translate(std::ostream& _io, std::string _mime, zpt::json _content) const
  -> std::string {
    auto _found = this->__content_providers.find(_mime);
    if (_found != this->__content_providers.end()) {
        return std::get<1>(_found->second)(_io, _content);
    }
    else { return zpt::transport::layer::translate_to_json(_io, _content); }
}

auto
zpt::transport::layer::begin() const -> std::map<std::string, zpt::transport>::const_iterator {
    return this->__underlying.begin();
}

auto
zpt::transport::layer::end() const -> std::map<std::string, zpt::transport>::const_iterator {
    return this->__underlying.end();
}

auto
zpt::transport::layer::resolve(std::string _uri) const -> zpt::exchange {
    auto _parsed = zpt::uri::parse(_uri);
    return this->get(_parsed["scheme"])->resolve(_parsed);
}

auto
zpt::transport::layer::add_content_provider(std::string const& _mime,
                                            translate_from_func _callback_from,
                                            translate_to_func _callback_to)
  -> zpt::transport::layer& {
    this->__content_providers.insert(std::pair(_mime, std::tuple(_callback_from, _callback_to)));
    return (*this);
}

auto
zpt::transport::layer::translate_from_default(std::istream& _io) -> zpt::json {
    try {
        return zpt::transport::layer::translate_from_json(_io);
    }
    catch (...) {
    }
    return zpt::transport::layer::translate_from_raw(_io);
}

auto
zpt::transport::layer::translate_to_default(std::ostream& _io, zpt::json _content) -> std::string {
    try {
        return zpt::transport::layer::translate_to_json(_io, _content);
    }
    catch (...) {
    }
    return zpt::transport::layer::translate_to_raw(_io, _content);
}

auto
zpt::transport::layer::translate_from_json(std::istream& _io) -> zpt::json {
    zpt::json _to_return;
    _io >> _to_return;
    return _to_return;
}

auto
zpt::transport::layer::translate_to_json(std::ostream& _io, zpt::json _content) -> std::string {
    _io << _content << std::flush;
    return "application/json";
}

auto
zpt::transport::layer::translate_from_raw(std::istream& _io) -> zpt::json {
    std::string _content;
    _io.seekg(0, std::ios::end);
    _content.reserve(_io.tellg());
    _io.seekg(0, std::ios::beg);
    _content.assign((std::istreambuf_iterator<char>(_io)), std::istreambuf_iterator<char>());
    return { _content };
}

auto
zpt::transport::layer::translate_to_raw(std::ostream& _io, zpt::json _content) -> std::string {
    _io << _content << std::flush;
    return "text/plain";
}

auto
zpt::transport::layer::translate_from_xml(std::istream& _io) -> zpt::json {
    std::string _content;
    _io.seekg(0, std::ios::end);
    _content.reserve(_io.tellg());
    _io.seekg(0, std::ios::beg);
    _content.assign((std::istreambuf_iterator<char>(_io)), std::istreambuf_iterator<char>());
    return { _content };
}

auto
zpt::transport::layer::translate_to_xml(std::ostream& _io, zpt::json _content) -> std::string {
    _io << "" << std::flush;
    return "text/xml";
}
