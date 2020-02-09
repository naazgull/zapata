#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/exceptions/CastException.h>

auto
zpt::init(zpt::HTTPReq& _req) -> void {
    time_t _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    char _buffer_expires[80];
    _ptm.tm_hour += 1;
    strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

    std::string _url(_req->url());
    if (_url != "") {
        size_t _b = _url.find("://") + 3;
        size_t _e = _url.find("/", _b);
        std::string _domain(_url.substr(_b, _e - _b));
        std::string _path(_url.substr(_e));
        _req->header("Host", _domain);
        _req->url(_path);
    }

    _req->method(zpt::http::Get);
    _req->header("User-Agent", "zapata rest-ful server");
    _req->header("Cache-Control", "max-age=3600");
    _req->header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
    _req->header("Date", std::string(_buffer_date));
}

auto
zpt::init(zpt::HTTPRep& _rep) -> void {
    time_t _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    char _buffer_expires[80];
    _ptm.tm_hour += 1;
    strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

    _rep->status(zpt::http::HTTP404);
    _rep->header("Server", "zapata");
    _rep->header("Cache-Control", "max-age=3600");
    _rep->header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
    _rep->header("Date", std::string(_buffer_date));
    _rep->header("Expires", std::string(_buffer_expires));
}

zpt::HTTPObj::HTTPObj() {}

zpt::HTTPObj::~HTTPObj() {}

auto
zpt::HTTPObj::body() -> std::string& {
    return this->__body;
}

auto
zpt::HTTPObj::body(std::string _body) -> void {
    this->__headers["Content-Length"] = std::to_string(_body.length());
    this->__body.assign(_body.data());
}

auto
zpt::HTTPObj::headers() -> zpt::http::header_map& {
    return this->__headers;
}

auto
zpt::HTTPObj::header(const char* _idx) -> std::string {
    std::string _name(_idx);
    zpt::prettify_header_name(_name);
    return this->__headers[_name];
}

auto
zpt::HTTPObj::header(const char* _name, const char* _value) -> void {
    std::string _n(_name);
    zpt::prettify_header_name(_n);
    this->__headers[_n] = _value;
}

auto
zpt::HTTPObj::header(const char* _name, std::string _value) -> void {
    std::string _n(_name);
    zpt::prettify_header_name(_n);
    this->__headers[_n] = _value;
}

auto
zpt::HTTPObj::header(std::string _name, std::string _value) -> void {
    std::string _n(_name);
    zpt::prettify_header_name(_n);
    this->__headers[_n] = _value;
}

auto
zpt::HTTPObj::version() -> std::string& {
    return this->__version;
}

auto
zpt::HTTPObj::version(std::string _version) -> void {
    this->__version.assign(_version);
}

zpt::HTTPObj::operator std::string() {
    return this->to_string();
}

auto
zpt::HTTPObj::to_string() -> std::string {
    std::string _return;
    this->stringify(_return);
    return _return;
}

namespace zpt {
namespace http {
std::string nil_header = "";

const char* method_names[] = { "GET",   "PUT",   "POST",     "DELETE", "HEAD",  "OPTIONS",
                               "PATCH", "REPLY", "M-SEARCH", "NOTIFY", "TRACE", "CONNECT" };

const char* status_names[] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "Continue ",
    "Switching Protocols ",
    "Processing ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "OK ",
    "Created ",
    "Accepted ",
    "Non-Authoritative Information ",
    "No Content ",
    "Reset Content ",
    "Partial Content ",
    "Multi-Status ",
    "Already Reported ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "IM Used ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "Multiple Choices ",
    "Moved Permanently ",
    "Found ",
    "See Other ",
    "Not Modified ",
    "Use Proxy ",
    "(Unused) ",
    "Temporary Redirect ",
    "Permanent Redirect ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "Bad Request ",
    "Unauthorized ",
    "Payment Required ",
    "Forbidden ",
    "Not Found ",
    "Method Not Allowed ",
    "Not Acceptable ",
    "Proxy Authentication Required ",
    "Request Timeout ",
    "Conflict ",
    "Gone ",
    "Length Required ",
    "Precondition Failed ",
    "Payload Too Large ",
    "URI Too Long ",
    "Unsupported Media Type ",
    "Requested Range Not Satisfiable ",
    "Expectation Failed ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "Unprocessable Entity ",
    "Locked ",
    "Failed Dependency ",
    "Unassigned ",
    "Upgrade Required ",
    "Unassigned ",
    "Precondition Required ",
    "Too Many Requests ",
    "Unassigned ",
    "Request Header Fields Too Large ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "Unavailable For Legal Reasons",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "Internal Server Error ",
    "Not Implemented ",
    "Bad Gateway ",
    "Service Unavailable ",
    "Gateway Timeout ",
    "HTTP Version Not Supported ",
    "Variant Also Negotiates (Experimental) ",
    "Insufficient Storage ",
    "Loop Detected ",
    "Unassigned ",
    "Not Extended ",
    "Network Authentication Required ",
};
} // namespace http
} // namespace zpt
