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

#include <zapata/exceptions/NoMoreElementsException.h>
#include <zapata/transport/transport.h>
#include <zapata/uri/uri.h>

auto zpt::basic_transport::receive(zpt::stream _stream) const -> zpt::message {
    zpt::message _to_return;
    if (this->has_capability(zpt::transport_capability::SYNCHRONOUS)) {
        assert(_stream->state() == zpt::stream_state::IDLE ||
               _stream->state() == zpt::stream_state::WAITING);
        expect(_stream->state() == zpt::stream_state::IDLE ||
                 _stream->state() == zpt::stream_state::WAITING,
               "Stream not in a valid state for receiving");

        if (_stream->state() == zpt::stream_state::IDLE) {
            _stream->state() = zpt::stream_state::PROCESSING;
            _to_return = this->process_incoming_request(_stream);
        }
        else if (_stream->state() == zpt::stream_state::WAITING) {
            _stream->state() = zpt::stream_state::IDLE;
            _to_return = this->process_incoming_reply(_stream);
        }
    }
    else { _to_return = this->process_incoming_request(_stream); }
    zlog("Received message via '" << _stream->uri() << "': \n" << _to_return, zpt::trace);
    return _to_return;
}

auto zpt::basic_transport::send(zpt::stream _stream, zpt::message _to_send) const -> void {
    if (this->has_capability(zpt::transport_capability::SYNCHRONOUS)) {
        assert(_stream->state() == zpt::stream_state::IDLE ||
               _stream->state() == zpt::stream_state::PROCESSING ||
               _stream->state() == zpt::stream_state::ERRORING_OUT);
        expect(_stream->state() == zpt::stream_state::IDLE ||
                 _stream->state() == zpt::stream_state::PROCESSING ||
                 _stream->state() == zpt::stream_state::ERRORING_OUT,
               "Stream not in a valid state for sending");
    }
    zlog("Sending message via '" << _stream->uri() << "' : \n" << _to_send, zpt::trace);

    _stream->write<zpt::message>(_to_send);

    if (this->has_capability(zpt::transport_capability::SYNCHRONOUS)) {
        if (_stream->state() == zpt::stream_state::IDLE) {
            _stream->state() = zpt::stream_state::WAITING;
        }
        else if (_stream->state() == zpt::stream_state::PROCESSING ||
                 _stream->state() == zpt::stream_state::ERRORING_OUT) {
            _stream->state() = zpt::stream_state::IDLE;
        }
    }
}

auto zpt::basic_transport::publish(zpt::message) const -> void {
    expect(this->has_capability(zpt::transport_capability::PUB_SUB),
           "Transport isn't capable of PUB-SUB");
}

zpt::network::layer::layer(zpt::json _global_config)
  : __configuration{ _global_config } {
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

auto zpt::network::layer::add(std::string const& _scheme, zpt::transport _transport)
  -> zpt::network::layer& {
    if (!this->__configuration("transport")("addresses")->ok()) {
        this->__configuration["transport"]["addresses"] = zpt::json::array();
    }
    if (_scheme != "self") {
        std::string _host;
        std::string _port;

        if (this->__configuration(_scheme)->ok() &&
            (this->__configuration(_scheme)("port")->ok() ||
             this->__configuration(_scheme)("path")->ok())) {
            if (!this->__configuration(_scheme)("bind")->ok() &&
                !this->__configuration(_scheme)("path")->ok()) {
                this->__configuration[_scheme]["bind"] = "0.0.0.0";
            }

            if (this->__configuration(_scheme)("address")->ok()) {
                _host.assign(this->__configuration(_scheme)("address")->string());
            }
            else if (!this->__configuration(_scheme)("path")->ok()) {
                auto _bind = this->__configuration(_scheme)("bind");
                if (_bind->ok() && _bind != "0.0.0.0") { _host.assign(_bind->string()); }
                else { _host.assign(zpt::net::getip()); }
            }

            if (this->__configuration(_scheme)("port")->ok()) {
                _port.assign(std::format(":{}", this->__configuration(_scheme)("port")->integer()));
            }
            else if (this->__configuration(_scheme)("path")->ok()) {
                _port.assign(this->__configuration(_scheme)("path")->string());
            }

            this->__configuration[_scheme]["address"] = _host;
        }
        else if (this->__configuration("transport")("default")->ok() &&
                 this->__configuration("transport")("default")->string() == _scheme) {
            auto _protocol = _scheme == "upnp" ? "udp" : "tcp";
            auto _port_i = zpt::net::get_available_port(_protocol, 9999);
            _host.assign(zpt::net::getip());
            _port.assign(std::format(":{}", _port_i));

            this->__configuration[_scheme] +=
              { "address", _host, "bind", "0.0.0.0", "port", _port_i };
        }

        this->__configuration["transport"]["addresses"]
          << std::format("{}://{}{}", _scheme, _host, _port);
    }

    this->__underlying.insert(std::make_pair(_scheme, _transport));
    return (*this);
}

auto zpt::network::layer::get(std::string const& _scheme) const -> const zpt::transport {
    auto _found = this->__underlying.find(_scheme);
    if (_found == this->__underlying.end()) {
        throw zpt::NoMoreElementsException(std::string{ "there is no such transport '" } + _scheme +
                                           std::string{ "'" });
    }
    return _found->second;
}

auto zpt::network::layer::remove(std::string const& _scheme) -> zpt::network::layer& {
    this->__underlying.erase(_scheme);
    return (*this);
}

auto zpt::network::layer::clear() -> zpt::network::layer& {
    this->__underlying.clear();
    return (*this);
}

auto zpt::network::layer::translate(std::istream& _io, std::string _mime) const -> zpt::json {
    auto _found = this->__content_providers.find(_mime);
    if (_found != this->__content_providers.end()) { return std::get<0>(_found->second)(_io); }
    else { return zpt::network::layer::translate_from_default(_io); }
    return zpt::undefined;
}

auto zpt::network::layer::translate(std::ostream& _io, std::string _mime, zpt::json _content) const
  -> std::string {
    auto _found = this->__content_providers.find(_mime);
    if (_found != this->__content_providers.end()) {
        return std::get<1>(_found->second)(_io, _content);
    }
    else { return zpt::network::layer::translate_to_json(_io, _content); }
}

auto zpt::network::layer::begin() const -> std::map<std::string, zpt::transport>::const_iterator {
    return this->__underlying.begin();
}

auto zpt::network::layer::end() const -> std::map<std::string, zpt::transport>::const_iterator {
    return this->__underlying.end();
}

auto zpt::network::layer::resolve(std::string _uri) const -> zpt::transport {
    auto _parsed = zpt::uri::parse(_uri);
    return this->get(_parsed("scheme"));
}

auto zpt::network::layer::add_content_provider(std::string const& _mime,
                                               translate_from_func _callback_from,
                                               translate_to_func _callback_to)
  -> zpt::network::layer& {
    this->__content_providers.insert(std::pair(_mime, std::tuple(_callback_from, _callback_to)));
    return (*this);
}

auto zpt::network::layer::translate_from_default(std::istream& _io) -> zpt::json {
    try {
        return zpt::network::layer::translate_from_json(_io);
    }
    catch (...) {
    }
    return zpt::network::layer::translate_from_raw(_io);
}

auto zpt::network::layer::translate_to_default(std::ostream& _io, zpt::json _content)
  -> std::string {
    try {
        return zpt::network::layer::translate_to_json(_io, _content);
    }
    catch (...) {
    }
    return zpt::network::layer::translate_to_raw(_io, _content);
}

auto zpt::network::layer::translate_from_json(std::istream& _io) -> zpt::json {
    zpt::json _to_return;
    _io >> _to_return;
    return _to_return;
}

auto zpt::network::layer::translate_to_json(std::ostream& _io, zpt::json _content) -> std::string {
    _io << _content << std::flush;
    return "application/json";
}

auto zpt::network::layer::translate_from_raw(std::istream& _io) -> zpt::json {
    std::string _content;
    _io.seekg(0, std::ios::end);
    _content.reserve(_io.tellg());
    _io.seekg(0, std::ios::beg);
    _content.assign((std::istreambuf_iterator<char>(_io)), std::istreambuf_iterator<char>());
    return { _content };
}

auto zpt::network::layer::translate_to_raw(std::ostream& _io, zpt::json _content) -> std::string {
    _io << _content << std::flush;
    return "text/plain";
}

auto zpt::network::layer::translate_from_xml(std::istream& _io) -> zpt::json {
    std::string _content;
    _io.seekg(0, std::ios::end);
    _content.reserve(_io.tellg());
    _io.seekg(0, std::ios::beg);
    _content.assign((std::istreambuf_iterator<char>(_io)), std::istreambuf_iterator<char>());
    return { _content };
}

auto zpt::network::layer::translate_to_xml(std::ostream& _io, zpt::json) -> std::string {
    _io << "" << std::flush;
    return "text/xml";
}

auto zpt::network::resolve_content_type(zpt::message _message) -> std::string {
    if (_message->headers()("Accept")->ok()) {
        auto _accept = _message->headers()("Accept")->string();
        auto _mime_types = zpt::split(_accept, ",");
        double _weight{ 0 };
        std::string _highest{ "*/*" };
        for (auto&& [_, __, _mime] : _mime_types) {
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
    return _message->headers()("Content-Type")->ok() ? _message->headers()("Content-Type")->string()
                                                     : "*/*";
}

auto zpt::TRANSPORT_LAYER(zpt::json _config) -> zpt::network::layer& {
    static zpt::network::layer _global{ _config };
    return _global;
}
