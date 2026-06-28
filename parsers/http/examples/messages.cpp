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

/*
 * Headless smoke tests for the re2c/bison HTTP parser. Feeds canned
 * request/reply fixtures through basic_request::from_stream/
 * basic_reply::from_stream on an std::istringstream and checks both the
 * populated fields AND the istream's remaining content, since the latter is
 * the direct regression test for the historical trailing-byte bug (the
 * stream's read position must land exactly at the first byte after the
 * message - no more, no less).
 */

#include <iostream>
#include <zapata/http.h>

namespace {
int failures = 0;

auto check(bool _condition, std::string const& _what) -> void {
    if (!_condition) {
        std::cerr << "FAIL: " << _what << std::endl;
        ++failures;
    }
}

auto remaining(std::istringstream& _iss) -> std::string {
    std::string _rest;
    char _c;
    while (_iss.get(_c)) { _rest.push_back(_c); }
    return _rest;
}

auto test_simple_get() -> void {
    std::istringstream _iss;
    _iss.str("GET /foo/bar?x=1 HTTP/1.1\r\nHost: example.com\r\n\r\nTRAILING");
    auto _req = zpt::allocate_message<zpt::http::basic_request>();
    _iss >> std::noskipws >> _req;

    check(_req->performative() == zpt::Get, "simple_get: performative is GET");
    check(_req->version() == "1.1", "simple_get: version is 1.1");
    check(static_cast<std::string>(_req->headers()("Host")) == "example.com",
          "simple_get: Host header");
    check(remaining(_iss) == "TRAILING", "simple_get: exactly the trailing bytes remain");
}

auto test_post_with_body() -> void {
    std::string _body = "{\"a\":1}";
    std::ostringstream _oss;
    _oss << "POST /items HTTP/1.1\r\n"
         << "Host: example.com\r\n"
         << "Content-Length: " << _body.length() << "\r\n"
         << "\r\n"
         << _body << "NEXTMSG";
    std::istringstream _iss;
    _iss.str(_oss.str());
    auto _req = zpt::allocate_message<zpt::http::basic_request>();
    _iss >> std::noskipws >> _req;

    check(_req->performative() == zpt::Post, "post_with_body: performative is POST");
    check(static_cast<std::string>(_req->body()) == _body, "post_with_body: body matches");
    check(remaining(_iss) == "NEXTMSG", "post_with_body: exactly the trailing bytes remain");
}

auto test_chunked_body_with_trailer() -> void {
    // "Wiki" + "pedia" chunked, with a declared trailer header and its value.
    std::ostringstream _oss;
    _oss << "POST /items HTTP/1.1\r\n"
         << "Host: example.com\r\n"
         << "Content-Type: text/plain\r\n"
         << "Transfer-Encoding: chunked\r\n"
         << "Trailer: X-Checksum\r\n"
         << "\r\n"
         << "4\r\n"
         << "Wiki\r\n"
         << "5\r\n"
         << "pedia\r\n"
         << "0\r\n"
         << "abc123\r\n"
         << "\r\n"
         << "AFTER";
    std::istringstream _iss;
    _iss.str(_oss.str());
    auto _req = zpt::allocate_message<zpt::http::basic_request>();
    _iss >> std::noskipws >> _req;

    check(static_cast<std::string>(_req->body()) == "Wikipedia",
          "chunked_body: decoded chunk data matches");
    check(remaining(_iss) == "AFTER", "chunked_body: exactly the trailing bytes remain");
}

auto test_no_body_reply_at_stream_eof() -> void {
    // No trailing bytes at all after the headers' blank line - this is the
    // exact shape of the original bug report (a WebSocket upgrade reply),
    // and the case that requires yyfill()'s zero-padding-at-EOF to match.
    std::istringstream _iss;
    _iss.str(
      "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: upgrade\r\n\r\n");
    auto _rep = zpt::allocate_message<zpt::http::basic_reply>();
    _iss >> std::noskipws >> _rep;

    check(static_cast<int>(_rep->status()) == 101, "no_body_reply: status is 101");
    check(remaining(_iss).empty(), "no_body_reply: nothing left in the stream");
}

auto test_chunked_reply() -> void {
    // Simulates a typical llama.cpp non-streaming reply:
    // HTTP/1.1 200 with Transfer-Encoding: chunked and no Content-Length.
    std::ostringstream _oss;
    _oss << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: application/json\r\n"
         << "Transfer-Encoding: chunked\r\n"
         << "\r\n"
         << "7\r\n"
         << "{\"a\":1}\r\n"
         << "0\r\n"
         << "\r\n"
         << "AFTER";
    std::istringstream _iss;
    _iss.str(_oss.str());
    auto _rep = zpt::allocate_message<zpt::http::basic_reply>();
    _iss >> std::noskipws >> _rep;

    check(static_cast<int>(_rep->status()) == 200, "chunked_reply: status is 200");
    check(static_cast<std::string>(_rep->body()) == "{\"a\":1}",
          "chunked_reply: body matches decoded chunk data");
    check(remaining(_iss) == "AFTER", "chunked_reply: exactly the trailing bytes remain");
}

auto test_consecutive_messages_on_one_stream() -> void {
    // Two back-to-back requests on the same stream, as would happen on a
    // persistent (keep-alive) connection - the second from_stream() call
    // must start exactly where the first one left off.
    std::istringstream _iss;
    _iss.str("GET /a HTTP/1.1\r\nHost: x\r\n\r\nGET /b HTTP/1.1\r\nHost: x\r\n\r\n");

    auto _req1 = zpt::allocate_message<zpt::http::basic_request>();
    _iss >> std::noskipws >> _req1;
    check(static_cast<std::string>(_req1->resource()) == "/a",
          "consecutive: first request's URI is /a");

    auto _req2 = zpt::allocate_message<zpt::http::basic_request>();
    _iss >> std::noskipws >> _req2;
    check(static_cast<std::string>(_req2->resource()) == "/b",
          "consecutive: second request's URI is /b");

    check(remaining(_iss).empty(), "consecutive: nothing left after both messages");
}
} // namespace

auto main() -> int {
    test_simple_get();
    test_post_with_body();
    test_chunked_body_with_trailer();
    test_chunked_reply();
    test_no_body_reply_at_stream_eof();
    test_consecutive_messages_on_one_stream();

    if (failures == 0) {
        std::cout << "All HTTP parser smoke tests passed." << std::endl;
        return 0;
    }
    std::cerr << failures << " smoke test(s) failed." << std::endl;
    return 1;
}
