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

#include <iostream>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>
#include <zapata/json/json.h>
#include <zapata/log/log.h>
#include <zapata/upnp/UPNPObj.h>
#include <zapata/uri.h>
#include <zapata/uuid.h>

auto zpt::upnp::basic_request::to_stream(std::ostream& _out) const -> zpt::basic_message const& {
    auto _uri = this->__underlying("uri");
    _out << this->__underlying("performative")->string() << " " << zpt::uri::path::to_string(_uri)
         << zpt::uri::params::to_string(_uri);

    _out << " UPNP/"
         << (this->__underlying("headers")("X-Version")->ok()
               ? this->__underlying("headers")("X-Version")->string()
               : "1.1")
         << CRLF;

    std::string _body{ "" };
    if (this->__underlying("body")->ok()) {
        if (this->__underlying("headers")("Content-Type")->stringify().find("application/json") !=
            std::string::npos) {
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
        if (this->__underlying("headers")("Content-Type")->stringify().find("application/json") !=
            std::string::npos) {
            auto _body = this->__underlying("body")->stringify();
            _out << "Content-Length: " << _body.length() << CRLF << CRLF << _body;
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
