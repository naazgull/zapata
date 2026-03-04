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
 * @file manip.h
 * @brief String manipulation utilities.
 *
 * Provides common string operations: trimming whitespace, replacing substrings,
 * normalizing paths, and formatting header names.
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdio.h>
#include <string.h>

namespace zpt {

/**
 * @brief Trims leading whitespace from a string (in-place).
 * @param _in_out String to modify.
 */
void ltrim(std::string& _in_out);

/**
 * @brief Trims trailing whitespace from a string (in-place).
 * @param _in_out String to modify.
 */
void rtrim(std::string& _in_out);

/**
 * @brief Trims leading and trailing whitespace from a string (in-place).
 * @param _in_out String to modify.
 */
void trim(std::string& _in_out);

/**
 * @brief Replaces all occurrences of a substring (in-place).
 * @param str String to modify.
 * @param find Substring to search for.
 * @param replace Replacement string.
 */
auto replace(std::string& str, std::string const& find, std::string const& replace) -> void;

/**
 * @brief Replaces all occurrences of a list of substrings (in-place).
 * @param str String to modify.
 * @param find List of substrings to search for.
 * @param replace List of replacements string.
 */
auto replace_multiple(std::string& _str,
                      std::vector<std::string> const& _find,
                      std::vector<std::string> const& _replace) -> void;

/**
 * @brief Normalizes a file path (in-place).
 * @param _in_out Path to normalize.
 * @param _with_trailing Whether to include trailing slash.
 *
 * Removes duplicate slashes and optionally ensures trailing slash.
 */
void normalize_path(std::string& _in_out, bool _with_trailing);

/**
 * @brief Converts a header name to title case (in-place).
 * @param name Header name to format.
 *
 * Converts "content-type" to "Content-Type".
 */
void prettify_header_name(std::string& name);

/**
 * @brief Returns a copy with leading whitespace removed.
 * @param _in_out Input string.
 * @return Trimmed copy.
 */
std::string r_ltrim(std::string const& _in_out);

/**
 * @brief Returns a copy with trailing whitespace removed.
 * @param _in_out Input string.
 * @return Trimmed copy.
 */
std::string r_rtrim(std::string const& _in_out);

/**
 * @brief Returns a copy with leading and trailing whitespace removed.
 * @param _in_out Input string.
 * @return Trimmed copy.
 */
std::string r_trim(std::string const& _in_out);

/**
 * @brief Returns a copy with all occurrences of a substring replaced.
 * @param str Input string.
 * @param find Substring to search for.
 * @param replace Replacement string.
 * @return Modified copy.
 */
std::string r_replace(std::string const& str, std::string const& find, std::string const& replace);

/**
 * @brief Returns a copy with all occurrences of a list of substrings replaced.
 * @param str Input string.
 * @param find List of substrings to search for.
 * @param replace List of replacement strings.
 * @return Modified copy.
 */
auto r_replace_multiple(std::string const& _str,
                        std::vector<std::string> const& _find,
                        std::vector<std::string> const& _replace) -> std::string;

/**
 * @brief Returns a normalized copy of a file path.
 * @param _in_out Input path.
 * @param _with_trailing Whether to include trailing slash.
 * @return Normalized path.
 */
std::string r_normalize_path(std::string const& _in_out, bool _with_trailing);

/**
 * @brief Returns a copy with header name in title case.
 * @param name Input header name.
 * @return Formatted header name.
 */
std::string r_prettify_header_name(std::string const& name);

} // namespace zpt

namespace {
#pragma GCC diagnostic ignored "-Wunused-function"
auto ___format(std::istringstream& _in, std::ostringstream& _out) -> void {
    char _c{ '\0' };
    do {
        _c = _in.get();
        if (!_in.eof()) { _out << _c; }
    } while (_in.good());
}
#pragma GCC diagnostic pop
template<typename P, typename... Args>
auto ___format(std::istringstream& _in, std::ostringstream& _out, P _param, Args... _rest) -> void {
    char _c{ '\0' };
    do {
        _c = _in.get();
        if (!_in.eof()) {
            if (_c == '{') {
                _c = _in.peek();
                if (_c == '}') {
                    _in.get();
                    _out << _param;
                    ::___format(_in, _out, _rest...);
                    return;
                }
                else { _out << '{'; }
            }
            else { _out << _c; }
        }
    } while (_in.good());
}
} // namespace
