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
 * @file usage.h
 * @brief Memory usage monitoring utilities.
 *
 * Provides functions for querying the current process's memory usage.
 */

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace zpt {

/**
 * @brief Type trait to detect pointer types.
 * @tparam T Type to check.
 */
template<typename T>
struct is_pointer {
    static const bool value = false; ///< False for non-pointer types.
};

/**
 * @brief Specialization for pointer types.
 * @tparam T Pointed-to type.
 */
template<typename T>
struct is_pointer<T*> {
    static const bool value = true; ///< True for pointer types.
};

/**
 * @brief Queries the current process's memory usage.
 * @param vm_usage Output: virtual memory size in KB.
 * @param resident_set Output: resident set size (physical memory) in KB.
 *
 * Reads from /proc/self/stat on Linux to obtain memory statistics.
 *
 * @par Example Usage
 * @code
 * double vm, rss;
 * zpt::process_mem_usage(vm, rss);
 * std::cout << "VM: " << vm << " KB, RSS: " << rss << " KB\n";
 * @endcode
 */
void process_mem_usage(double& vm_usage, double& resident_set);

} // namespace zpt
