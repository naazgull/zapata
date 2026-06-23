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
 * @file http.h
 * @brief HTTP message parsing and serialization.
 *
 * This is the aggregate header for the HTTP parser module. It provides
 * classes for parsing and serializing HTTP/1.1 requests and responses.
 *
 * Key types:
 * - `zpt::http::basic_request` - HTTP request message
 * - `zpt::http::basic_reply` - HTTP response message
 * - `zpt::http::status` - HTTP status codes
 *
 * @par Example
 * @code
 * #include <zapata/http.h>
 *
 * // Parse an HTTP request
 * auto req = "GET /api/users HTTP/1.1\r\nHost: example.com\r\n\r\n"_HTTP_REQUEST;
 *
 * // Access request properties
 * auto method = req->performative();  // zpt::Get
 * auto uri = req->uri();              // URI JSON object
 * auto headers = req->headers();      // Headers JSON object
 *
 * // Create and serialize a response
 * zpt::http::basic_reply reply;
 * reply.status(zpt::http::HTTP200);
 * reply.header("Content-Type", "application/json");
 * reply.body("{\"status\": \"ok\"}");
 * reply.to_stream(std::cout);
 * @endcode
 *
 * @see zpt::http::basic_request
 * @see zpt::http::basic_reply
 */

#pragma once

#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/http/HTTPObj.h>
#include <zapata/http/HTTPParser.h>
#include <zapata/http/HTTPTokenizer.h>
#include <zapata/http/HTTPTokenizerLexer.h>
#include <zapata/http/Re2cHTTPLexer.h>
