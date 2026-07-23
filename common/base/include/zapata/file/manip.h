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
 * @brief Filesystem utilities for directory traversal and globbing.
 *
 * Provides functions for listing directory contents with pattern matching.
 */

#pragma once

#include <dirent.h>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <zapata/exceptions/SyntaxErrorException.h>

namespace zpt {
/**
 * @brief Lists files matching a regex pattern.
 * @param dir Directory to search in.
 * @param result Vector to receive matching file paths.
 * @param pattern Compiled regex pattern to match against filenames.
 * @param recursion Maximum recursion depth (0 = current directory only).
 * @return Number of matching files found.
 *
 * Searches the directory for files whose names match the regex pattern.
 */
auto globRegexp(std::string& dir,
                std::vector<std::string>& result,
                std::regex& pattern,
                short recursion = 0) -> int;

/**
 * @brief Lists files matching a glob pattern.
 * @param dir Directory to search in.
 * @param result Vector to receive matching file paths.
 * @param pattern Glob pattern (e.g., "*.txt", "*.{cpp,h}").
 * @param recursion Maximum recursion depth (0 = current directory only).
 * @return Number of matching files found.
 *
 * @par Example Usage
 * @code
 * std::vector<std::string> files;
 * zpt::glob("/src", files, "*.cpp", 3);  // Find .cpp files up to 3 levels deep
 * @endcode
 */
auto glob(std::string dir,
          std::vector<std::string>& result,
          std::string pattern,
          short recursion = 0) -> int;

} // namespace zpt
