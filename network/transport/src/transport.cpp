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

zpt::json_message::json_message(basic_message const& _request, bool) {
    auto _req_headers = _request.headers();
    auto _headers = zpt::json::object();
    _headers["Content-Type"] = _request.content_type();
    _headers["Cache-Control"] =
      _req_headers("Cache-Control")->ok() ? _req_headers("Cache-Control") : "no-store";
    _headers["X-Conversation-ID"] =
      _req_headers("X-Conversation-ID")->ok() ? _req_headers("X-Conversation-ID") : "0";
    _headers["X-Version"] = _req_headers("X-Version")->ok() ? _req_headers("X-Version") : "1.0";

    this->__underlying                                       //
      << "performative" << zpt::ontology::to_str(zpt::Reply) //
      << "uri" << _request.uri()                             //
      << "headers" << _headers;
}

auto
zpt::json_message::performative() const -> zpt::performative {
    return zpt::ontology::from_str(this->__underlying("performative")->string());
}

auto
zpt::json_message::status() const -> zpt::status {
    return this->__underlying("status")->integer();
}

auto
zpt::json_message::uri() -> zpt::json& {
    return this->__underlying["uri"];
}

auto
zpt::json_message::uri() const -> zpt::json const {
    return this->__underlying("uri");
}

auto
zpt::json_message::version() const -> std::string {
    return this->__underlying("headers")("X-Version")->string();
}

auto
zpt::json_message::scheme() const -> std::string {
    return this->__underlying("uri")("scheme")->string();
}

auto
zpt::json_message::resource() const -> zpt::json const {
    return this->__underlying("uri")("path");
}

auto
zpt::json_message::parameters() const -> zpt::json const {
    return this->__underlying("uri")("params");
}

auto
zpt::json_message::headers() -> zpt::json& {
    return this->__underlying["headers"];
}

auto
zpt::json_message::headers() const -> zpt::json const {
    return this->__underlying("headers");
}

auto
zpt::json_message::body() -> zpt::json& {
    return this->__underlying["body"];
}

auto
zpt::json_message::body() const -> zpt::json const {
    return this->__underlying("body");
}

auto
zpt::json_message::keep_alive() const -> bool {
    return this->__underlying("headers")("Connection") == "keep-alive" ? true : false;
}

auto
zpt::json_message::content_type() const -> std::string {
    return this->__underlying("headers")("Content-Type")->string();
}

auto
zpt::json_message::to_stream(std::ostream& _out) const -> void {
    _out << this->__underlying;
}

auto
zpt::json_message::from_stream(std::istream& _in) -> void {
    _in >> std::noskipws >> this->__underlying;
}

auto
zpt::json_message::performative(zpt::performative _performative) -> void {
    this->__underlying["performative"] = zpt::ontology::to_str(_performative);
}

auto
zpt::json_message::status(zpt::status _status) -> void {
    this->__underlying["satus"] = _status;
}

auto
zpt::json_message::uri(std::string const& _uri) -> void {
    this->__underlying["uri"] = zpt::uri::parse(_uri);
}

auto
zpt::json_message::version(std::string const& _version) -> void {
    this->__underlying["headers"]["X-Version"] = _version;
}

zpt::network::layer::layer() {
    this->add_content_provider("*/*",
                               zpt::network::layer::translate_from_default,
                               zpt::network::layer::translate_to_default);
    this->add_content_provider(
      "text", zpt::network::layer::translate_from_raw, zpt::network::layer::translate_to_raw);
    this->add_content_provider(
      "text/plain", zpt::network::layer::translate_from_raw, zpt::network::layer::translate_to_raw);
    this->add_content_provider(
      "json", zpt::network::layer::translate_from_json, zpt::network::layer::translate_to_json);
    this->add_content_provider("application/json",
                               zpt::network::layer::translate_from_json,
                               zpt::network::layer::translate_to_json);
    this->add_content_provider("text/x-json",
                               zpt::network::layer::translate_from_json,
                               zpt::network::layer::translate_to_json);
    this->add_content_provider(
      "text/xml", zpt::network::layer::translate_from_xml, zpt::network::layer::translate_to_xml);
}

auto
zpt::network::layer::add(std::string const& _scheme, zpt::transport _transport)
  -> zpt::network::layer& {
    this->__underlying.insert(std::make_pair(_scheme, _transport));
    return (*this);
}

auto
zpt::network::layer::get(std::string const& _scheme) const -> const zpt::transport {
    auto _found = this->__underlying.find(_scheme);
    if (_found == this->__underlying.end()) {
        throw zpt::NoMoreElementsException("there is no such transport");
    }
    return _found->second;
}

auto
zpt::network::layer::translate(std::istream& _io, std::string _mime) const -> zpt::json {
    auto _found = this->__content_providers.find(_mime);
    if (_found != this->__content_providers.end()) { return std::get<0>(_found->second)(_io); }
    else { return zpt::network::layer::translate_from_default(_io); }
    return zpt::undefined;
}

auto
zpt::network::layer::translate(std::ostream& _io, std::string _mime, zpt::json _content) const
  -> std::string {
    auto _found = this->__content_providers.find(_mime);
    if (_found != this->__content_providers.end()) {
        return std::get<1>(_found->second)(_io, _content);
    }
    else { return zpt::network::layer::translate_to_json(_io, _content); }
}

auto
zpt::network::layer::begin() const -> std::map<std::string, zpt::transport>::const_iterator {
    return this->__underlying.begin();
}

auto
zpt::network::layer::end() const -> std::map<std::string, zpt::transport>::const_iterator {
    return this->__underlying.end();
}

auto
zpt::network::layer::resolve(std::string _uri) const -> zpt::transport {
    auto _parsed = zpt::uri::parse(_uri);
    return this->get(_parsed["scheme"]);
}

auto
zpt::network::layer::add_content_provider(std::string const& _mime,
                                          translate_from_func _callback_from,
                                          translate_to_func _callback_to) -> zpt::network::layer& {
    this->__content_providers.insert(std::pair(_mime, std::tuple(_callback_from, _callback_to)));
    return (*this);
}

auto
zpt::network::layer::translate_from_default(std::istream& _io) -> zpt::json {
    try {
        return zpt::network::layer::translate_from_json(_io);
    }
    catch (...) {
    }
    return zpt::network::layer::translate_from_raw(_io);
}

auto
zpt::network::layer::translate_to_default(std::ostream& _io, zpt::json _content) -> std::string {
    try {
        return zpt::network::layer::translate_to_json(_io, _content);
    }
    catch (...) {
    }
    return zpt::network::layer::translate_to_raw(_io, _content);
}

auto
zpt::network::layer::translate_from_json(std::istream& _io) -> zpt::json {
    zpt::json _to_return;
    _io >> _to_return;
    return _to_return;
}

auto
zpt::network::layer::translate_to_json(std::ostream& _io, zpt::json _content) -> std::string {
    _io << _content << std::flush;
    return "application/json";
}

auto
zpt::network::layer::translate_from_raw(std::istream& _io) -> zpt::json {
    std::string _content;
    _io.seekg(0, std::ios::end);
    _content.reserve(_io.tellg());
    _io.seekg(0, std::ios::beg);
    _content.assign((std::istreambuf_iterator<char>(_io)), std::istreambuf_iterator<char>());
    return { _content };
}

auto
zpt::network::layer::translate_to_raw(std::ostream& _io, zpt::json _content) -> std::string {
    _io << _content << std::flush;
    return "text/plain";
}

auto
zpt::network::layer::translate_from_xml(std::istream& _io) -> zpt::json {
    std::string _content;
    _io.seekg(0, std::ios::end);
    _content.reserve(_io.tellg());
    _io.seekg(0, std::ios::beg);
    _content.assign((std::istreambuf_iterator<char>(_io)), std::istreambuf_iterator<char>());
    return { _content };
}

auto
zpt::network::layer::translate_to_xml(std::ostream& _io, zpt::json _content) -> std::string {
    _io << "" << std::flush;
    return "text/xml";
}

auto
zpt::network::resolve_content_type(zpt::basic_message const& _message) -> std::string {
    if (_message.headers()("Accept")->ok()) {
        auto _accept = _message.headers()["Accept"]->string();
        auto _mime_types = zpt::split(_accept, ",");
        double _weight{ 0 };
        std::string _highest{ "*/*" };
        for (auto [_, __, _mime] : _mime_types) {
            auto _semicolon = _mime->string().find(";");
            auto _mime_name = _mime->string().substr(0, _semicolon);
            if (_semicolon != std::string::npos) {
                auto _mime_weight = _mime->string().substr(_semicolon);
                double _w{ 0 };
                std::istringstream _iss;
                _iss.str(_mime_weight);
                _iss >> _w;
                if (_w > _weight) {
                    _highest.assign(_mime_name);
                    _weight = _w;
                }
            }
            else { return _mime_name; }
        }
        return _highest;
    }
    return _message.headers()("Content-Type")->ok() ? _message.headers()["Content-Type"]->string()
                                                    : "*/*";
}

auto
operator<<(std::ostream& _out, zpt::message _in) -> std::ostream& {
    _in->to_stream(_out);
    return _out;
}

auto
operator>>(std::istream& _in, zpt::message _out) -> std::istream& {
    _out->from_stream(_in);
    return _in;
}
