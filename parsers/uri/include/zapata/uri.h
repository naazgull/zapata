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
 * @file uri.h
 * @brief URI parsing and manipulation utilities.
 *
 * This is the aggregate header for the URI parser module. It provides
 * functions to parse URIs into structured JSON objects and convert them
 * back to strings.
 *
 * The URI is decomposed into components:
 * - scheme (e.g., "http", "https")
 * - authority (host, port, user info)
 * - path segments
 * - query parameters
 * - fragment
 *
 * @par Example
 * @code
 * #include <zapata/uri.h>
 *
 * // Parse a URI
 * auto uri = zpt::uri::parse("https://user:pass@example.com:8080/path?q=1#frag");
 *
 * // Access components
 * std::string scheme = uri["scheme"];   // "https"
 * std::string host = uri["host"];       // "example.com"
 * int port = uri["port"];               // 8080
 * auto params = uri["params"];          // {"q": "1"}
 *
 * // Convert back to string
 * std::string url = zpt::uri::to_string(uri);
 * @endcode
 *
 * @see zpt::uri::parse
 * @see zpt::uri::to_string
 */

#pragma once

#include <zapata/exceptions/NoHeaderNameException.h>
#include <zapata/uri/URILexer.h>
#include <zapata/uri/URILexerbase.h>
#include <zapata/uri/URILexerimpl.h>
#include <zapata/uri/URIParser.h>
#include <zapata/uri/URITokenizer.h>
#include <zapata/uri/URITokenizerLexer.h>
#include <zapata/uri/URITokenizerbase.h>
#include <zapata/uri/URITokenizerimpl.h>
#include <zapata/uri/URIinc.h>
#include <zapata/uri/uri.h>
