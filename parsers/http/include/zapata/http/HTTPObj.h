#pragma once

#define DEBUG_JSON

#include <map>
#include <memory>
#include <ostream>
#include <vector>
#include <zapata/base/expect.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

#ifndef CRLF
#define CRLF "\r\n"
#endif

namespace zpt {
namespace http {
enum message_type { request, reply };

enum status {
    HTTP100 = 100,
    HTTP101 = 101,
    HTTP102 = 102,
    HTTP200 = 200,
    HTTP201 = 201,
    HTTP202 = 202,
    HTTP203 = 203,
    HTTP204 = 204,
    HTTP205 = 205,
    HTTP206 = 206,
    HTTP207 = 207,
    HTTP208 = 208,
    HTTP226 = 226,
    HTTP300 = 300,
    HTTP301 = 301,
    HTTP302 = 302,
    HTTP303 = 303,
    HTTP304 = 304,
    HTTP305 = 305,
    HTTP306 = 306,
    HTTP307 = 307,
    HTTP308 = 308,
    HTTP400 = 400,
    HTTP401 = 401,
    HTTP402 = 402,
    HTTP403 = 403,
    HTTP404 = 404,
    HTTP405 = 405,
    HTTP406 = 406,
    HTTP407 = 407,
    HTTP408 = 408,
    HTTP409 = 409,
    HTTP410 = 410,
    HTTP411 = 411,
    HTTP412 = 412,
    HTTP413 = 413,
    HTTP414 = 414,
    HTTP415 = 415,
    HTTP416 = 416,
    HTTP417 = 417,
    HTTP422 = 422,
    HTTP423 = 423,
    HTTP424 = 424,
    HTTP425 = 425,
    HTTP426 = 426,
    HTTP427 = 427,
    HTTP428 = 428,
    HTTP429 = 429,
    HTTP430 = 430,
    HTTP431 = 431,
    HTTP451 = 451,
    HTTP500 = 500,
    HTTP501 = 501,
    HTTP502 = 502,
    HTTP503 = 503,
    HTTP504 = 504,
    HTTP505 = 505,
    HTTP506 = 506,
    HTTP507 = 507,
    HTTP508 = 508,
    HTTP509 = 509,
    HTTP510 = 510,
    HTTP511 = 511
};

using header_map = std::map<std::string, std::string>;
using parameter_map = std::map<std::string, std::string>;
} // namespace http

class HTTPObj {
  public:
    HTTPObj();
    virtual ~HTTPObj();

    auto body() -> std::string&;
    auto body(std::string const& _body) -> void;
    auto headers() -> zpt::http::header_map&;
    auto header(std::string const& _name) -> std::string const&;
    auto header(std::string const& _name, std::string const& _value) -> void;
    auto version() -> std::string&;
    auto version(std::string const& _version) -> void;

    operator std::string();

    virtual auto to_string() -> std::string;
    virtual auto stringify(std::string& _out) -> void = 0;
    virtual auto stringify(std::ostream& _out) -> void = 0;

  protected:
    std::string __body{ "" };
    zpt::http::header_map __headers;
    std::string __version{ "1.1" };
};

using HTTPPtr = std::shared_ptr<HTTPObj>;

class HTTPReqT : public HTTPObj {
  public:
    HTTPReqT();
    virtual ~HTTPReqT();

    auto method() const -> zpt::performative;
    auto method(zpt::performative) -> void;
    auto url() const -> std::string const&;
    auto url(std::string const&) -> void;
    auto query() const -> std::string const&;
    auto query(std::string const&) -> void;
    auto params() const -> zpt::http::parameter_map const&;
    auto param(std::string const& _name) -> std::string const&;
    auto param(std::string const& _name, std::string const& _value) -> void;

    virtual auto stringify(std::string& _out) -> void;
    virtual auto stringify(std::ostream& _out) -> void;

  private:
    std::string __url{ "" };
    std::string __query{ "" };
    zpt::performative __method{ 0 };
    zpt::http::parameter_map __params;
};

class HTTPRepT : public HTTPObj {
  public:
    HTTPRepT();
    virtual ~HTTPRepT();

    auto status() -> zpt::http::status;
    auto status(zpt::http::status) -> void;

    virtual auto stringify(std::string& _out) -> void;
    virtual auto stringify(std::ostream& _out) -> void;

  private:
    zpt::http::status __status{ zpt::http::HTTP100 };
};

class HTTPReq {
  public:
    HTTPReq();
    virtual ~HTTPReq();

    auto operator*() -> zpt::HTTPReqT&;
    auto operator->() -> zpt::HTTPReqT*;
    operator std::string();

    virtual auto parse(std::istream& _in) -> void;

    friend auto operator<<(std::ostream& _out, HTTPReq& _in) -> std::ostream& {
        _in->stringify(_out);
        return _out;
    }
    friend auto operator>>(std::istream& _in, HTTPReq& _out) -> std::istream& {
        _out.parse(_in);
        return _in;
    }

  private:
    std::shared_ptr<zpt::HTTPReqT> __underlying{ nullptr };
};

class HTTPRep {
  public:
    HTTPRep();
    virtual ~HTTPRep();

    auto operator*() -> zpt::HTTPRepT&;
    auto operator->() -> zpt::HTTPRepT*;
    operator std::string();

    virtual void parse(std::istream& _in);

    friend auto operator<<(std::ostream& _out, HTTPRep& _in) -> std::ostream& {
        _in->stringify(_out);
        return _out;
    }

    friend auto operator>>(std::istream& _in, HTTPRep& _out) -> std::istream& {
        _out.parse(_in);
        return _in;
    }

  private:
    std::shared_ptr<zpt::HTTPRepT> __underlying{ nullptr };
};

namespace http {
extern std::string nil_header;
extern const char* method_names[];
extern const char* status_names[];

static inline const unsigned short Get = 0;
static inline const unsigned short Put = 1;
static inline const unsigned short Post = 2;
static inline const unsigned short Delete = 3;
static inline const unsigned short Head = 4;
static inline const unsigned short Options = 5;
static inline const unsigned short Patch = 6;
static inline const unsigned short Reply = 7;
static inline const unsigned short Msearch = 8;
static inline const unsigned short Notify = 9;
static inline const unsigned short Trace = 10;
static inline const unsigned short Connect = 11;

using req = zpt::HTTPReq;
using rep = zpt::HTTPRep;
} // namespace http

void
init(HTTPReq& _out);
void
init(HTTPRep& _out);
} // namespace zpt

auto operator"" _HTTP_REQUEST(const char* _string, size_t _length) -> zpt::http::req;
auto operator"" _HTTP_REPLY(const char* _string, size_t _length) -> zpt::http::rep;
