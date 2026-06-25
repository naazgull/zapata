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
 * @file URITokenizer.h
 * @brief Hand-written wrapper around the bison-generated yyparse() for URI.y.
 *
 * Replaces the generated bisonc++ URITokenizer/URITokenizerBase pair.
 * Keeps the same public surface (default-constructible, `int parse()`) so
 * URIParser.h/.cpp need no changes beyond delegating to d_scanner.
 *
 * d_scanner is held by composition, not inheritance: this keeps the
 * bison-generated header include (in URITokenizer.cpp) confined to a single
 * .cpp file at file scope, so it never has to appear inside a namespace
 * block in any header.
 */

#pragma once

#include <zapata/uri/URITokenizerLexer.h>

namespace zpt {

class URITokenizer {
  public:
    URITokenizer() = default;

    auto parse() -> int;

    URITokenizerLexer d_scanner;
};

} // namespace zpt
