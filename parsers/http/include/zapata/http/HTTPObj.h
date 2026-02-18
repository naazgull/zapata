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

/**
 * @file HTTPObj.h
 * @brief HTTP message types and status code definitions.
 *
 * Defines the HTTP message classes used for request/response handling
 * in the Zapata framework. These classes implement the `zpt::basic_message`
 * interface for use with transport layers.
 *
 * @see zpt::http::basic_request
 * @see zpt::http::basic_reply
 * @see zpt::http::status
 */

#pragma once

#define DEBUG_JSON

#include <map>
#include <memory>
#include <ostream>
#include <vector>
#include <zapata/base/expect.h>
#include <zapata/json/JSONClass.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>
#include <zapata/transport.h>

#ifndef CRLF
#define CRLF "\r\n"
#endif

namespace zpt {
/**
 * @brief HTTP protocol parsing and message handling.
 */
namespace http {

/**
 * @brief HTTP status codes enumeration.
 *
 * All standard HTTP/1.1 status codes from 1xx (informational) through
 * 5xx (server errors). Use these with `basic_reply::status()`.
 */
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

/** @brief HTTP method name strings for serialization. */
inline const char* method_names[] = { "GET",   "PUT",   "POST",     "DELETE", "HEAD",  "OPTIONS",
                                      "PATCH", "REPLY", "M-SEARCH", "NOTIFY", "TRACE", "CONNECT" };

inline const char* status_names[] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
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

/**
 * @brief Base class for HTTP messages (requests and responses).
 *
 * Implements the `zpt::basic_message` interface for HTTP protocol.
 * Provides access to headers, body, URI, and other message properties.
 */
class basic_message : public zpt::basic_message {
  public:
    /** @brief Default constructor. */
    basic_message() = default;
    /** @brief Destructor. */
    virtual ~basic_message() = default;

    /** @brief Returns the HTTP method (GET, POST, etc.). */
    virtual auto performative() const -> zpt::performative override;
    /** @brief Returns the HTTP status code. */
    virtual auto status() const -> zpt::status override;
    /** @brief Returns reference to the request URI. */
    virtual auto uri() -> zpt::json& override;
    /** @brief Returns the request URI (const). */
    virtual auto uri() const -> zpt::json const override;
    /** @brief Returns the HTTP version (e.g., "1.1"). */
    virtual auto version() const -> std::string override;
    /** @brief Returns the URI scheme (http, https). */
    virtual auto scheme() const -> std::string override;
    /** @brief Returns the resource path. */
    virtual auto resource() const -> zpt::json const override;
    /** @brief Returns query parameters. */
    virtual auto parameters() const -> zpt::json const override;
    /** @brief Returns reference to headers object. */
    virtual auto headers() -> zpt::json& override;
    /** @brief Returns headers (const). */
    virtual auto headers() const -> zpt::json const override;
    /** @brief Returns reference to body. */
    virtual auto body() -> zpt::json& override;
    /** @brief Returns body (const). */
    virtual auto body() const -> zpt::json const override;
    /** @brief Checks if connection should be kept alive. */
    virtual auto keep_alive() const -> bool override;
    /** @brief Returns the Content-Type header value. */
    virtual auto content_type() const -> std::string override;
    /** @brief Sets the HTTP method. */
    virtual auto performative(zpt::performative _performative) -> zpt::basic_message& override;
    /** @brief Sets the HTTP status code. */
    virtual auto status(zpt::status _status) -> zpt::basic_message& override;
    /** @brief Sets the request URI from string. */
    virtual auto uri(std::string const& _uri) -> zpt::basic_message& override;
    /** @brief Sets the HTTP version. */
    virtual auto version(std::string const& _uri) -> zpt::basic_message& override;
    /** @brief Checks if the message is empty/uninitialized. */
    virtual auto empty() const -> bool override;

    /** @brief Returns the URI fragment/anchor. */
    virtual auto anchor() const -> std::string;
    /**
     * @brief Sets the message body from a string.
     * @param _body Body content.
     * @return Reference for chaining.
     */
    virtual auto body(std::string const& _body) -> zpt::basic_message&;
    /**
     * @brief Adds or updates a header.
     * @param _name Header name.
     * @param _value Header value.
     * @return Reference for chaining.
     */
    virtual auto header(std::string const& _name, std::string const& _value) -> zpt::basic_message&;

  protected:
    zpt::json __underlying; ///< Internal JSON storage for message data
};

/**
 * @brief HTTP request message.
 *
 * Represents an HTTP request with method, URI, headers, and optional body.
 * Can be parsed from streams or constructed programmatically.
 *
 * @par Example
 * @code
 * zpt::http::basic_request req;
 * req.performative(zpt::Get);
 * req.uri("/api/users");
 * req.header("Accept", "application/json");
 * req.to_stream(socket);
 * @endcode
 */
class basic_request : public zpt::http::basic_message {
  public:
    /** @brief Default constructor. */
    basic_request();
    /**
     * @brief Constructs from a generic message.
     * @param _request Source message.
     * @param Unused compatibility parameter.
     */
    basic_request(zpt::basic_message const& _request, bool);
    /** @brief Destructor. */
    virtual ~basic_request() = default;

    /** @brief Serializes the request to an output stream. */
    virtual auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    /** @brief Parses a request from an input stream. */
    virtual auto from_stream(std::istream& _in) -> zpt::basic_message& override;
};

/** @brief Shared pointer type for HTTP requests. */
using request = std::shared_ptr<basic_request>;

/**
 * @brief HTTP response message.
 *
 * Represents an HTTP response with status code, headers, and optional body.
 *
 * @par Example
 * @code
 * zpt::http::basic_reply reply;
 * reply.status(zpt::http::HTTP200);
 * reply.header("Content-Type", "application/json");
 * reply.body(R"({"status": "ok"})");
 * reply.to_stream(socket);
 * @endcode
 */
class basic_reply : public zpt::http::basic_message {
  public:
    /** @brief Default constructor. */
    basic_reply();
    /**
     * @brief Constructs a reply from a request (for response generation).
     * @param _request Original request message.
     * @param Unused compatibility parameter.
     */
    basic_reply(zpt::basic_message const& _request, bool);
    /** @brief Destructor. */
    virtual ~basic_reply() = default;

    /** @brief Serializes the response to an output stream. */
    virtual auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    /** @brief Parses a response from an input stream. */
    virtual auto from_stream(std::istream& _in) -> zpt::basic_message& override;
};

/** @brief Shared pointer type for HTTP responses. */
using reply = std::shared_ptr<basic_reply>;

} // namespace http

/** @brief Initializes an HTTP request with default values. */
void init(zpt::http::basic_request& _out);
/** @brief Initializes an HTTP response with default values. */
void init(zpt::http::basic_reply& _out);
} // namespace zpt

/**
 * @brief User-defined literal for parsing HTTP requests.
 * @param _string Raw HTTP request text.
 * @param _length String length.
 * @return Parsed request as zpt::message.
 *
 * @par Example
 * @code
 * auto req = "GET /api HTTP/1.1\r\nHost: example.com\r\n\r\n"_HTTP_REQUEST;
 * @endcode
 */
auto operator"" _HTTP_REQUEST(const char* _string, size_t _length) -> zpt::message;

/**
 * @brief User-defined literal for parsing HTTP responses.
 * @param _string Raw HTTP response text.
 * @param _length String length.
 * @return Parsed response as zpt::message.
 */
auto operator"" _HTTP_REPLY(const char* _string, size_t _length) -> zpt::message;
