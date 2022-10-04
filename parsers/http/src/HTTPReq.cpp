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

auto
zpt::http::basic_request::to_stream(std::ostream& _out) const -> void {
    _out << this->__underlying("performative")->string() << " "
         << zpt::uri::to_string(this->__underlying("uri"));

    if (this->__underlying("uri")("params")->ok()) {
        _out << "?";
        for (auto [_, _name, _value] : this->__underlying("params")) {
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

auto
zpt::http::basic_request::from_stream(std::istream& _in) -> void {
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

auto
zpt::http::basic_request::make_reply() -> zpt::message {
    return zpt::make_message<zpt::http::basic_request>(*this, true);
}

auto operator"" _HTTP_REQUEST(const char* _string, size_t _length) -> zpt::message {
    std::istringstream _oss;
    auto _to_return = zpt::make_message<zpt::http::basic_request>();
    _oss.str(std::string{ _string, _length });
    _oss >> _to_return;
    return _to_return;
}
