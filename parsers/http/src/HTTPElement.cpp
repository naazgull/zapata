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
#include <zapata/http/HTTPObj.h>
#include <zapata/uri/uri.h>
#include <zapata/uuid.h>

auto zpt::http::basic_message::performative() const -> zpt::performative {
    return zpt::ontology::from_str(this->__underlying("performative")->string());
}

auto zpt::http::basic_message::status() const -> zpt::status {
    return this->__underlying("status")->integer();
}

auto zpt::http::basic_message::uri() -> zpt::json& { return this->__underlying["uri"]; }

auto zpt::http::basic_message::uri() const -> zpt::json const { return this->__underlying("uri"); }

auto zpt::http::basic_message::version() const -> std::string {
    return this->__underlying("headers")("X-Version")->string();
}

auto zpt::http::basic_message::scheme() const -> std::string {
    return this->__underlying("uri")("scheme")->string();
}

auto zpt::http::basic_message::resource() const -> zpt::json const {
    return this->__underlying("uri")("raw_path");
}

auto zpt::http::basic_message::parameters() const -> zpt::json const {
    return this->__underlying("uri")("params");
}

auto zpt::http::basic_message::headers() -> zpt::json& { return this->__underlying["headers"]; }

auto zpt::http::basic_message::headers() const -> zpt::json const {
    return this->__underlying("headers");
}

auto zpt::http::basic_message::body() -> zpt::json& { return this->__underlying["body"]; }

auto zpt::http::basic_message::body() const -> zpt::json const {
    return this->__underlying("body");
}

auto zpt::http::basic_message::keep_alive() const -> bool {
    return this->__underlying("headers")("Connection") == "keep-alive" ? true : false;
}

auto zpt::http::basic_message::content_type() const -> std::string {
    auto ct = this->__underlying("headers")("Content-Type");
    return ct->ok() ? ct->string() : "text/plain";
}

auto zpt::http::basic_message::performative(zpt::performative _performative)
  -> zpt::basic_message& {
    this->__underlying["performative"] = zpt::ontology::to_str(_performative);
    return (*this);
}

auto zpt::http::basic_message::status(zpt::status _status) -> zpt::basic_message& {
    this->__underlying["status"] = _status;
    return (*this);
}

auto zpt::http::basic_message::uri(std::string const& _s_uri) -> zpt::basic_message& {
    auto _uri = zpt::uri::parse(_s_uri);
    if (_uri("domain")->ok()) {
        this->__underlying["headers"]["Host"] = zpt::uri::address::to_string(_uri);
    }
    this->__underlying["uri"] = _uri;
    return (*this);
}

auto zpt::http::basic_message::version(std::string const& _version) -> zpt::basic_message& {
    this->__underlying["headers"]["X-Version"] = _version;
    return (*this);
}

auto zpt::http::basic_message::empty() const -> bool {
    return !this->__underlying->ok() || this->__underlying->stringify().length() == 0;
}

auto zpt::http::basic_message::anchor() const -> std::string {
    return this->__underlying("uri")("anchor")->string();
}

auto zpt::http::basic_message::body(std::string const& _body) -> zpt::basic_message& {
    if (this->content_type().find("application/json") != std::string::npos) {
        this->__underlying["body"] = zpt::json::parse_json_str(_body);
    }
    else { this->__underlying["body"] = _body; }
    return (*this);
}

auto zpt::http::basic_message::header(std::string const& _name, std::string const& _value)
  -> zpt::basic_message& {
    this->__underlying["headers"][_name] = _value;
    return (*this);
}

auto zpt::init(zpt::http::basic_request& _req) -> void {
    time_t _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    _req.performative(zpt::Get);
    _req.header("User-Agent", "zapata");
    _req.header("Date", std::string(_buffer_date));
    _req.header("Content-Type", "application/json");
    _req.header("Cache-Control", "no-store");
    _req.header("X-Conversation-ID", zpt::uuid{}.to_string());
    _req.header("X-Version", "1.1");
}

auto zpt::init(zpt::http::basic_reply& _rep) -> void {
    time_t _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    _rep.status(zpt::http::HTTP404);
    _rep.header("Server", "zapata");
    _rep.header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
    _rep.header("Date", std::string(_buffer_date));
}
