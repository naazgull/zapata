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

#include <mutex>
#include <zapata/ontology/message.h>
#include <zapata/uri/uri.h>
#include <zapata/uuid.h>

auto zpt::call_context::state() const -> int { return this->__state->load(); }

auto zpt::call_context::reply() const -> zpt::message { return this->__reply; }

auto zpt::call_context::reply(zpt::message _to_update) -> call_context& {
    this->__reply = _to_update;
    this->__state->store(_to_update->status() < 300 ? zpt::CALL_STATE_SUCCESS_REPLY
                                                    : zpt::CALL_STATE_FAILURE_REPLY);
    return (*this);
}

auto zpt::call_context::is_replied() const -> bool {
    return this->__state->load() > zpt::CALL_STATE_SENT;
}

auto zpt::call_context::has_error() const -> bool {
    return this->__state->load() == zpt::CALL_STATE_FAILURE_REPLY;
}

zpt::json_message::json_message()
  : __underlying{ zpt::json::object() } {
    auto _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    zpt::json _headers{ "Content-Type",
                        "application/json",
                        "Cache-Control",
                        "no-store",
                        "X-Conversation-ID",
                        zpt::uuid{}.to_string(),
                        "X-Version",
                        "1.1",
                        "Date",
                        std::string{ _buffer_date } };

    this->__underlying << "headers" << _headers;
}

zpt::json_message::json_message(zpt::json const& _other)
  : __underlying{ _other->clone() } {}

zpt::json_message::json_message(zpt::message _request, bool)
  : json_message{} {
    auto _req_headers = _request->headers();

    if (_req_headers("Cache-Control")->ok()) {
        this->__underlying["headers"]["Cache-Control"] = _req_headers("Cache-Control");
    }
    if (_req_headers("X-Conversation-ID")->ok()) {
        this->__underlying["headers"]["X-Conversation-ID"] = _req_headers("X-Conversation-ID");
    }
    if (_req_headers("X-Version")->ok()) {
        this->__underlying["headers"]["X-Version"] = _req_headers("X-Version");
    }

    this->__underlying                                       //
      << "performative" << zpt::ontology::to_str(zpt::Reply) //
      << "uri" << _request->uri();
}

auto zpt::json_message::clone() const -> zpt::message {
    auto _other = std::make_shared<zpt::json_message>();
    _other->__underlying = this->__underlying->clone();
    return _other;
}

auto zpt::json_message::performative() const -> zpt::performative {
    return zpt::ontology::from_str(this->__underlying("performative")->string());
}

auto zpt::json_message::status() const -> zpt::status {
    return this->__underlying("status")->integer();
}

auto zpt::json_message::uri() -> zpt::json& { return this->__underlying["uri"]; }

auto zpt::json_message::uri() const -> zpt::json const { return this->__underlying("uri"); }

auto zpt::json_message::version() const -> std::string {
    return this->__underlying("headers")("X-Version")->string();
}

auto zpt::json_message::scheme() const -> std::string {
    return this->__underlying("uri")("scheme")->string();
}

auto zpt::json_message::resource() const -> zpt::json const {
    return this->__underlying("uri")("raw_path");
}

auto zpt::json_message::parameters() const -> zpt::json const {
    return this->__underlying("uri")("params");
}

auto zpt::json_message::headers() -> zpt::json& { return this->__underlying["headers"]; }

auto zpt::json_message::headers() const -> zpt::json const { return this->__underlying("headers"); }

auto zpt::json_message::header(std::string const& _name, std::string const& _value)
  -> zpt::basic_message& {
    this->__underlying["headers"][zpt::r_prettify_header_name(_name)] = _value;
    return (*this);
}

auto zpt::json_message::body() -> zpt::json& { return this->__underlying["body"]; }

auto zpt::json_message::body() const -> zpt::json const { return this->__underlying("body"); }

auto zpt::json_message::keep_alive() const -> bool {
    return this->__underlying("headers")("Connection") == "keep-alive" ? true : false;
}

auto zpt::json_message::content_type() const -> std::string { return "application/json"; }

auto zpt::json_message::to_stream(std::ostream& _out) const -> zpt::basic_message const& {
    _out << this->__underlying << std::endl;
    return (*this);
}

auto zpt::json_message::from_stream(std::istream& _in) -> zpt::basic_message& {
    _in >> std::noskipws >> this->__underlying;
    return (*this);
}

auto zpt::json_message::performative(zpt::performative _performative) -> zpt::basic_message& {
    this->__underlying["performative"] = zpt::ontology::to_str(_performative);
    return (*this);
}

auto zpt::json_message::status(zpt::status _status) -> zpt::basic_message& {
    this->__underlying["status"] = _status;
    return (*this);
}

auto zpt::json_message::uri(std::string const& _s_uri) -> zpt::basic_message& {
    auto _uri = zpt::uri::parse(_s_uri);
    if (_uri("domain")->ok()) {
        this->__underlying["headers"]["Host"] = zpt::uri::address::to_string(_uri);
    }
    this->__underlying["uri"] = _uri;
    return (*this);
}

auto zpt::json_message::version(std::string const& _version) -> zpt::basic_message& {
    this->__underlying["headers"]["X-Version"] = _version;
    return (*this);
}

auto zpt::json_message::empty() const -> bool {
    return !this->__underlying->ok() || this->__underlying->stringify().length() == 0;
}

auto zpt::uri_to_string(zpt::json const& _uri) -> std::string { return zpt::uri::to_string(_uri); }

auto operator<<(std::ostream& _out, zpt::message _in) -> std::ostream& {
    _in->to_stream(_out);
    return _out;
}

auto operator>>(std::istream& _in, zpt::message _out) -> std::istream& {
    _out->from_stream(_in);
    return _in;
}
