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
 * @file HTTPTokenizer.h
 * @brief Hand-written wrapper around the bison-generated yyparse() for HTTP.y.
 *
 * Replaces the generated bisonc++ HTTPTokenizer/HTTPTokenizerBase pair.
 * Keeps the same public surface (default-constructible, `int parse()`) so
 * HTTPParser.h/.cpp need no changes.
 */

#pragma once

#include <zapata/http/HTTPTokenizerLexer.h>

namespace zpt {

class HTTPTokenizer {
  public:
    HTTPTokenizer() = default;

    /** @brief Parses one HTTP/UPnP message from d_scanner's current stream.
     * @return 0 on success, non-zero on error. */
    auto parse() -> int;

    HTTPTokenizerLexer d_scanner;
};

} // namespace zpt
