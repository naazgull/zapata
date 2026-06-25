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
 * @brief URI parser (re2c/bison replacement for the generated flexc++/bisonc++ URILexer/URITokenizerBase).
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
 * Parses URI strings into structured JSON.
 */
class URIParser : public URITokenizer {
  public:
    /** @brief Constructs a parser with the given I/O streams. */
    URIParser(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~URIParser();

    /** @brief Sets the JSON root node to populate during parsing. */
    auto switchRoots(zpt::json& _root) -> void;
    /** @brief Switches the input/output streams. */
    auto switchStreams(std::istream& _in = std::cin, std::ostream& _out = std::cout) -> void;
    /** @brief Clears the internal structures. */
    auto clear() -> void;
};
} // namespace zpt
