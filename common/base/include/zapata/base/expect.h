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
 * @file expect.h
 * @brief Assertion macros and utility functions.
 *
 * Provides the expect() macro for runtime assertion checking that throws
 * ExpectationException on failure, along with timezone and thread utilities.
 */

#pragma once

#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <zapata/exceptions/ExpectationException.h>

/**
 * @def expect(x, y)
 * @brief Runtime assertion macro that throws ExpectationException on failure.
 * @param x Boolean expression to validate.
 * @param y Error message (can be a stream expression).
 *
 * If the expression @p x evaluates to false, throws zpt::ExpectationException
 * with the provided error message, the stringified expression, and source location.
 *
 * @par Example Usage
 * @code
 * expect(ptr != nullptr, "Pointer must not be null");
 * expect(value > 0, "Value must be positive, got: " << value);
 * @endcode
 *
 * @throws zpt::ExpectationException When the condition is false.
 */
#define expect(x, y)                                                                               \
    if (!(x)) {                                                                                    \
        std::ostringstream __OSS__;                                                                \
        __OSS__ << y << std::flush;                                                                \
        throw zpt::ExpectationException(__OSS__.str(), #x, __LINE__, __FILE__);                    \
    }

#define expect_c(x, y, c)                                                                          \
    if (!(x)) {                                                                                    \
        std::ostringstream __OSS__;                                                                \
        __OSS__ << y << std::flush;                                                                \
        throw zpt::ExpectationException(__OSS__.str(), #x, __LINE__, __FILE__, c);                 \
    }

namespace zpt {

/**
 * @brief Returns the system timezone string.
 * @return Reference to the timezone string (e.g., "UTC", "EST").
 */
auto get_tz() -> std::string const&;

/**
 * @brief Shared pointer to a tm structure.
 */
using tm_ptr = std::shared_ptr<std::tm>;

/**
 * @brief Converts a time_t to a thread-safe tm structure.
 * @param _t The time value to convert.
 * @return Shared pointer to the tm structure.
 *
 * Unlike localtime(), this function is thread-safe and returns
 * a shared pointer to avoid dangling pointer issues.
 */
auto get_time(time_t _t) -> zpt::tm_ptr;

namespace this_thread {
/**
 * @brief Sets the name of the current thread for debugging purposes.
 * @param _name The name to assign to the current thread.
 *
 * Thread names appear in debuggers and profilers, making it easier
 * to identify threads during development.
 */
auto name(std::string const& _name) -> void;
auto name() -> std::string;
} // namespace this_thread

} // namespace zpt
