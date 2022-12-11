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
#include <zapata/log/log.h>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPParser.h>
#include <zapata/json/json.h>
#include <zapata/uri.h>

zpt::http::basic_request::basic_request() {
    this->__underlying["performative"] = zpt::ontology::to_str(zpt::Get);
    zpt::init(*this);
}

zpt::http::basic_request::basic_request(zpt::basic_message const& _request, bool)
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

auto zpt::http::basic_request::to_stream(std::ostream& _out) const -> void {
    _out << this->__underlying("performative")->string() << " "
         << static_cast<std::string>(this->__underlying("uri")("raw_path"));

    if (this->__underlying("uri")("params")->ok()) {
        _out << "?";
        for (auto const& [_, _name, _value] : this->__underlying("params")) {
            _out << _name << "=" << _value;
        }
    }

    _out << " HTTP/"
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
}

auto zpt::http::basic_request::from_stream(std::istream& _in) -> void {
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

auto operator"" _HTTP_REQUEST(const char* _string, size_t _length) -> zpt::message {
    std::istringstream _oss;
    auto _to_return = zpt::make_message<zpt::http::basic_request>();
    _oss.str(std::string{ _string, _length });
    _oss >> _to_return;
    return _to_return;
}
