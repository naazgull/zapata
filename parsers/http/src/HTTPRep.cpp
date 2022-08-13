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
    _headers["X-Version"] = _req_headers("X-Version")->ok() ? _req_headers("X-Version") : "1.0";

    this->__underlying           //
      << "uri" << _request.uri() //
      << "headers" << _headers;
}

auto
zpt::http::basic_reply::to_stream(std::ostream& _out) const -> void {
    zpt::status _status = static_cast<int>(this->__underlying("status")) > 99
                            ? static_cast<int>(this->__underlying("status"))
                            : 100;
    std::string _version = this->__underlying("headers")("X-Version")->ok()
                             ? this->__underlying("headers")("X-Version")->string()
                             : "1.1";
    _out << "HTTP/" << _version << " " << std::to_string(_status);
    if (_version == "1.0") { _out << " " << zpt::http::status_names[_status]; }
    _out << CRLF;

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
zpt::http::basic_reply::from_stream(std::istream& _in) -> void {
    static thread_local zpt::HTTPParser _p;
    _p.switchRoots(*this);
    _p.switchStreams(_in);
    _p.parse();
}

auto operator"" _HTTP_REPLY(const char* _string, size_t _length) -> zpt::message {
    std::istringstream _oss;
    auto _to_return = zpt::make_message<zpt::http::basic_reply>();
    _oss.str(std::string{ _string, _length });
    _oss >> _to_return;
    return _to_return;
}
