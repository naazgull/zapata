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
 * @file Exception.h
 * @brief Base exception class for the Zapata framework.
 *
 * Provides the root exception type from which all Zapata-specific exceptions derive.
 * Extends std::exception with string-based error messages.
 */

#pragma once

#include <exception>
#include <iostream>
#include <string>
#include <zapata/log/log.h>

namespace zpt {

/**
 * @brief Base exception class for all Zapata framework exceptions.
 *
 * Provides a simple exception type that stores an error message string.
 * All framework-specific exceptions should derive from this class.
 *
 * @par Example Usage
 * @code
 * throw zpt::exception("Something went wrong");
 * @endcode
 *
 * @see zpt::ExpectationException
 * @see zpt::SyntaxErrorException
 */
class exception : public std::exception {
  public:
    /**
     * @brief Constructs an exception with the given error message.
     * @param _what The error message describing what went wrong.
     */
    exception(std::string const& _what);

    /**
     * @brief Destructor.
     */
    virtual ~exception() throw();

    /**
     * @brief Returns the error message.
     * @return Null-terminated string describing the error.
     */
    virtual auto what() const noexcept -> const char* override;

    /**
     * @brief Stream output operator for printing exceptions.
     * @param _out Output stream.
     * @param _in Exception to output.
     * @return Reference to the output stream.
     */
    friend auto operator<<(std::ostream& _out, zpt::exception const& _in) -> std::ostream& {
        _out << _in.what() << std::flush;
        return _out;
    }

  private:
    std::string __what; ///< The error message.
};

} // namespace zpt
