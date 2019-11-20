#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/exceptions/CastException.h>

void
zpt::init(HTTPReq& _req) {
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

void
zpt::init(HTTPRep& _rep) {
    time_t _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    char _buffer_expires[80];
    _ptm.tm_hour += 1;
    strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

    _rep->status(zpt::http::HTTP404);
    _rep->header("User-Agent", "zapata rest-ful server");
    _rep->header("Cache-Control", "max-age=3600");
    _rep->header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
    _rep->header("Date", std::string(_buffer_date));
    _rep->header("Expires", std::string(_buffer_expires));
}

zpt::HTTPObj::HTTPObj() {}

zpt::HTTPObj::~HTTPObj() {}

std::string&
zpt::HTTPObj::body() {
    return this->__body;
}

void
zpt::HTTPObj::body(std::string _body) {
    this->__body.assign(_body.data());
}

zpt::http::header_map&
zpt::HTTPObj::headers() {
    return this->__headers;
}

std::string
zpt::HTTPObj::header(const char* _idx) {
    std::string _name(_idx);
    zpt::prettify_header_name(_name);
    auto _found = this->__headers.find(_name);
    if (_found != this->__headers.end()) {
        return _found->second;
    }
    return "";
}

void
zpt::HTTPObj::header(const char* _name, const char* _value) {
    std::string _n(_name);
    zpt::prettify_header_name(_n);
    auto _found = this->__headers.find(_n);
    if (_found != this->__headers.end()) {
        this->__headers.erase(_found);
    }
    this->__headers.insert(std::make_pair(_n, _value));
}

void
zpt::HTTPObj::header(const char* _name, std::string _value) {
    std::string _n(_name);
    zpt::prettify_header_name(_n);
    auto _found = this->__headers.find(_n);
    if (_found != this->__headers.end()) {
        this->__headers.erase(_found);
    }
    this->__headers.insert(std::make_pair(_n, _value));
}

void
zpt::HTTPObj::header(std::string _name, std::string _value) {
    std::string _n(_name);
    zpt::prettify_header_name(_n);
    auto _found = this->__headers.find(_n);
    if (_found != this->__headers.end()) {
        this->__headers.erase(_found);
    }
    this->__headers.insert(std::make_pair(_n, _value));
}

zpt::HTTPObj::operator std::string() {
    return this->to_string();
}

std::string
zpt::HTTPObj::to_string() {
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
    "100 Continue ",
    "101 Switching Protocols ",
    "102 Processing ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "200 OK ",
    "201 Created ",
    "202 Accepted ",
    "203 Non-Authoritative Information ",
    "204 No Content ",
    "205 Reset Content ",
    "206 Partial Content ",
    "207 Multi-Status ",
    "208 Already Reported ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "226 IM Used ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "300 Multiple Choices ",
    "301 Moved Permanently ",
    "302 Found ",
    "303 See Other ",
    "304 Not Modified ",
    "305 Use Proxy ",
    "306 (Unused) ",
    "307 Temporary Redirect ",
    "308 Permanent Redirect ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "400 Bad Request ",
    "401 Unauthorized ",
    "402 Payment Required ",
    "403 Forbidden ",
    "404 Not Found ",
    "405 Method Not Allowed ",
    "406 Not Acceptable ",
    "407 Proxy Authentication Required ",
    "408 Request Timeout ",
    "409 Conflict ",
    "410 Gone ",
    "411 Length Required ",
    "412 Precondition Failed ",
    "413 Payload Too Large ",
    "414 URI Too Long ",
    "415 Unsupported Media Type ",
    "416 Requested Range Not Satisfiable ",
    "417 Expectation Failed ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "422 Unprocessable Entity ",
    "423 Locked ",
    "424 Failed Dependency ",
    "425 Unassigned ",
    "426 Upgrade Required ",
    "427 Unassigned ",
    "428 Precondition Required ",
    "429 Too Many Requests ",
    "430 Unassigned ",
    "431 Request Header Fields Too Large ",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "451 Unavailable For Legal Reasons",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "500 Internal Server Error ",
    "501 Not Implemented ",
    "502 Bad Gateway ",
    "503 Service Unavailable ",
    "504 Gateway Timeout ",
    "505 HTTP Version Not Supported ",
    "506 Variant Also Negotiates (Experimental) ",
    "507 Insufficient Storage ",
    "508 Loop Detected ",
    "509 Unassigned ",
    "510 Not Extended ",
    "511 Network Authentication Required ",
};
} // namespace http
} // namespace zpt
