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

#include <zapata/upnp/UPNPObj.h>

#include <iostream>
#include <zapata/log/log.h>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>
#include <zapata/json/json.h>
#include <zapata/uri.h>

zpt::upnp::basic_request::basic_request() {
    this->__underlying["performative"] = zpt::ontology::to_str(zpt::Notify);
    zpt::init(static_cast<zpt::http::basic_request&>(*this));
}

zpt::upnp::basic_request::basic_request(zpt::basic_message const& _request, bool)
  : basic_request{} {
    auto _req_headers = _request.headers();
    auto _headers = zpt::json::object();
    _headers["Content-Type"] = zpt::network::resolve_content_type(_request);
    _headers["Cache-Control"] =
      _req_headers("Cache-Control")->ok() ? _req_headers("Cache-Control") : "no-store";
    _headers["X-Conversation-ID"] =
      _req_headers("X-Conversation-ID")->ok() ? _req_headers("X-Conversation-ID") : "0";
    _headers["X-Version"] = _req_headers("X-Version")->ok() ? _req_headers("X-Version") : "1.0";

    this->__underlying           //
      << "uri" << _request.uri() //
      << "headers" << _headers;
}

auto zpt::upnp::basic_request::to_stream(std::ostream& _out) const -> zpt::basic_message const& {
    _out << this->__underlying("performative")->string() << " "
         << zpt::uri::to_string(this->__underlying("uri"));

    if (this->__underlying("uri")("params")->ok()) {
        _out << "?";
        for (auto const& [_, _name, _value] : this->__underlying("uri")("params")) {
            _out << _name << "=" << static_cast<std::string>(_value);
        }
    }

    _out << " UPNP/"
         << (this->__underlying("headers")("X-Version")->ok()
               ? this->__underlying("headers")("X-Version")->string()
               : "1.1")
         << CRLF;

    std::string _body{ "" };
    if (this->__underlying("body")->ok()) {
        if (this->__underlying("headers")("Content-Type") == "application/json") {
            _body.assign(static_cast<std::string>(this->__underlying("body")));
        }
        else { _body.assign(this->__underlying("body")->string()); }
    }

    for (auto [_, _name, _value] : this->__underlying("headers")) {
        _out << _name << ": " << static_cast<std::string>(_value) << CRLF;
    }
    _out << "Content-Length: " << _body.length() << CRLF;

    _out << CRLF << _body;

    return (*this);
}

auto zpt::upnp::basic_request::from_stream(std::istream& _in) -> zpt::basic_message& {
    static thread_local zpt::HTTPParser _p;
    _p.switchRoots(*static_cast<zpt::http::basic_request*>(this));
    _p.switchStreams(_in);
    try {
        _p.parse();
    }
    catch (zpt::SyntaxErrorException const& _e) {
        this->__underlying = zpt::undefined;
        throw;
    }
    catch (...) {
    }

    return (*this);
}

zpt::upnp::basic_reply::basic_reply() {
    this->__underlying["performative"] = zpt::ontology::to_str(zpt::Reply);
    zpt::init(static_cast<zpt::http::basic_reply&>(*this));
}

zpt::upnp::basic_reply::basic_reply(zpt::basic_message const& _request, bool)
  : basic_reply{} {
    auto _req_headers = _request.headers();
    auto _headers = zpt::json::object();
    _headers["Content-Type"] = zpt::network::resolve_content_type(_request);
    _headers["Cache-Control"] =
      _req_headers("Cache-Control")->ok() ? _req_headers("Cache-Control") : "no-store";
    _headers["X-Conversation-ID"] =
      _req_headers("X-Conversation-ID")->ok() ? _req_headers("X-Conversation-ID") : "0";
    _headers["X-Version"] = _req_headers("X-Version")->ok() ? _req_headers("X-Version") : "1.1";

    this->__underlying           //
      << "uri" << _request.uri() //
      << "headers" << _headers;
}

auto zpt::upnp::basic_reply::to_stream(std::ostream& _out) const -> zpt::basic_message const& {
    zpt::status _status = static_cast<int>(this->__underlying("status")) > 99
                            ? static_cast<int>(this->__underlying("status"))
                            : 100;
    std::string _version = this->__underlying("headers")("X-Version")->ok()
                             ? this->__underlying("headers")("X-Version")->string()
                             : "1.1";
    _out << "UPNP/" << _version << " " << std::to_string(_status);
    if (_version == "1.0") { _out << " " << zpt::http::status_names[_status]; }
    _out << CRLF;

    for (auto const& [_, _name, _value] : this->__underlying("headers")) {
        _out << _name << ": " << static_cast<std::string>(_value) << CRLF;
    }

    if (this->__underlying("body")->ok()) {
        if (this->__underlying("headers")("Content-Type") == "application/json") {
            _out << "Content-Length: " << this->__underlying("body")->string_length() << CRLF
                 << CRLF << this->__underlying("body");
        }
        else {
            _out << "Content-Length: " << this->__underlying("body")->string().length() << CRLF
                 << CRLF << this->__underlying("body")->string();
        }
    }
    else { _out << "Content-Length: 0" << CRLF << CRLF; }

    return (*this);
}

auto zpt::upnp::basic_reply::from_stream(std::istream& _in) -> zpt::basic_message& {
    static thread_local zpt::HTTPParser _p;
    _p.switchRoots(*this);
    _p.switchStreams(_in);
    try {
        _p.parse();
    }
    catch (zpt::SyntaxErrorException const& _e) {
        throw;
    }
    catch (...) {
    }

    return (*this);
}
