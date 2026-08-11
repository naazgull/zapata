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
 * @file URIParser.h
 * @brief URI parser (re2c/bison replacement for the generated flexc++/bisonc++
 * URILexer/URITokenizerBase).
 *
 * Parses URI strings into structured JSON components (scheme, host,
 * path, query parameters, fragment).
 *
 * @see zpt::uri::parse
 */

#pragma once

#include <zapata/uri/URITokenizer.h>

namespace zpt {

/**
 * @brief URI parser.
 *
 * Wrapper around the re2c/bison generated parser for URIs.
 * Parses URI strings into structured JSON with components: scheme, host, path, query, fragment.
 *
 * Example JSON output format:
 * @code
 * {
 *   "scheme": "http",
 *   "host": "example.com",
 *   "path": ["api", "users", "123"],
 *   "query": { "page": "1", "limit": "10" },
 *   "fragment": "results"
 * }
 * @endcode
 */
class URIParser : public URITokenizer {
  public:
    /** @brief Constructs a parser with the given I/O streams.
     * @param _in Input stream to read URI strings from (default: stdin)
     * @param _out Output stream for parser logging (default: stdout)
     */
    URIParser(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    /** @brief Destructor.
     * @return void (destructors implicitly clean up the object). */
    virtual ~URIParser();

    /** @brief Sets the JSON root node to populate during parsing.
     * The parser will write URI components into this JSON object.
     * @param _root JSON object to populate with URI components
     * @return void (internal __root set).
     */
    auto switchRoots(zpt::json& _root) -> void;
    /** @brief Switches the input/output streams for parsing.
     * Allows reusing the parser instance with different streams.
     * @param _in Input stream to read URI strings from
     * @param _out Output stream for parser logging
     * @return void (internal streams replaced).
     */
    auto switchStreams(std::istream& _in = std::cin, std::ostream& _out = std::cout) -> void;
    /** @brief Clears the internal structures after parsing.
     * Resets parser state for reuse with a new URI string.
     * @return Reference to this parser for chaining.
     */
    auto clear() -> void;
};
} // namespace zpt
