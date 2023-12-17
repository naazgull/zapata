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

#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>
#include <zapata/uri/uri.h>

zpt::http::basic_reply::basic_reply() {
    this->__underlying["performative"] = zpt::ontology::to_str(zpt::Reply);
    zpt::init(*this);
}

zpt::http::basic_reply::basic_reply(zpt::basic_message const& _request, bool)
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

auto zpt::http::basic_reply::to_stream(std::ostream& _out) const -> void {
    zpt::status _status = static_cast<int>(this->__underlying("status")) > 99
                            ? static_cast<int>(this->__underlying("status"))
                            : 100;
    std::string _version = this->__underlying("headers")("X-Version")->ok()
                             ? this->__underlying("headers")("X-Version")->string()
                             : "1.1";
    _out << "HTTP/" << _version << " " << std::to_string(_status);
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
}

auto zpt::http::basic_reply::from_stream(std::istream& _in) -> void {
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
}

auto operator"" _HTTP_REPLY(const char* _string, size_t _length) -> zpt::message {
    std::istringstream _oss;
    auto _to_return = zpt::allocate_message<zpt::http::basic_reply>();
    _oss.str(std::string{ _string, _length });
    _oss >> _to_return;
    return _to_return;
}
