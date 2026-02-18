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
 * @file html.h
 * @brief HTML entity encoding and decoding utilities.
 *
 * Provides functions for escaping and unescaping HTML entities in strings.
 */

#pragma once

#include <algorithm>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <memory.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <wchar.h>
#include <wctype.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

namespace zpt {
namespace html {

/**
 * @brief Encodes HTML entities from a wide string to a stream.
 * @param s Wide string to encode.
 * @param out Output stream for encoded result.
 * @param quote Whether to encode quote characters.
 * @param tags Whether to encode tag characters (< and >).
 */
auto entities_encode(std::wstring s, std::ostream& out, bool quote = true, bool tags = false)
  -> void;

/**
 * @brief Encodes HTML entities in a string (in-place).
 * @param out String to encode (modified in place).
 * @param quote Whether to encode quote characters.
 * @param tags Whether to encode tag characters.
 *
 * Converts characters like &, <, >, " to their HTML entity equivalents
 * (&amp;, &lt;, &gt;, &quot;).
 */
auto entities_encode(std::string& out, bool quote = true, bool tags = false) -> void;

/**
 * @brief Decodes HTML entities in a string (in-place).
 * @param out String to decode (modified in place).
 *
 * Converts HTML entities like &amp;, &lt;, &gt; back to their
 * character equivalents.
 */
auto entities_decode(std::string& out) -> void;

/**
 * @brief Extracts the multipart boundary from a Content-Type header.
 * @param _in Content-Type header value.
 * @param _out Extracted boundary string.
 */
auto content_boundary(std::string& _in, std::string& _out) -> void;

} // namespace html
} // namespace zpt
