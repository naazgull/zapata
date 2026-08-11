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
 * @file ExpectationException.h
 * @brief Exception thrown when expect() macro fails.
 *
 * Provides an exception type that captures the failed condition, source
 * location, and error message from the expect() assertion macro.
 */

#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <zapata/exceptions/Exception.h>

namespace zpt {

/**
 * @brief Exception thrown when an expect() assertion fails.
 *
 * Captures detailed information about the failed assertion including the
 * condition that failed, the source file and line number, and a custom
 * error message.
 *
 * @see expect()
 */
class ExpectationException : public zpt::exception {
  public:
    /**
     * @brief Constructs an ExpectationException.
     * @param _what Error message describing the failure.
     * @param _desc The stringified condition that failed.
     * @param _line Source line number where the failure occurred.
     * @param _file Source file where the failure occurred.
     * @param _code Error code for the failure.
     * @return void (constructors implicitly initialize the object).
     * @throws ExpectationException Always (the constructor itself throws).
     */
    ExpectationException(std::string const& _what,
                         std::string _desc,
                         int _line = 0,
                         std::string _file = "",
                         int _code = 500);
    virtual ~ExpectationException() = default;

    /**
     * @brief Returns the failed condition description.
     * @return The stringified condition that failed.
     */
    virtual auto description() const -> const char*;

    /**
     * @brief Returns the error code for the failure.
     * @return The error code for the failure.
     */
    virtual auto code() const -> int;

    /**
     * @brief Stream output operator.
     * @param _out Output stream.
     * @param _in Exception to output.
     * @return Reference to the output stream.
     */
    friend auto operator<<(std::ostream& _out, zpt::ExpectationException const& _in)
      -> std::ostream& {
        _out << _in.what() << ": " << _in.description() << ")";
        return _out;
    }

  private:
    std::string __description; ///< The failed condition string.
    int __line;                ///< Source line number.
    std::string __file;        ///< Source file name.
    int __code;                ///< The error code for the failure.
};

/** @brief Type alias for ExpectationException. */
using failed_expectation = ExpectationException;
} // namespace zpt
