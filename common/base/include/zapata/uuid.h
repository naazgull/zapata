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
 * @file uuid.h
 * @brief UUID (Universally Unique Identifier) class.
 *
 * Provides a UUID type for generating and manipulating 128-bit unique identifiers.
 */

#pragma once

#include <string>

namespace zpt {

/**
 * @brief Universally Unique Identifier (UUID) class.
 *
 * Represents a 128-bit UUID with support for generation, parsing, and
 * string conversion. Uses the standard 8-4-4-4-12 hexadecimal format.
 *
 * @par Example Usage
 * @code
 * zpt::uuid id;                    // Generate new UUID
 * std::cout << id << std::endl;    // e.g., "550e8400-e29b-41d4-a716-446655440000"
 *
 * zpt::uuid parsed("550e8400-e29b-41d4-a716-446655440000");
 * @endcode
 */
class uuid {
  public:
    /**
     * @brief Generates a new random UUID.
     */
    uuid();

    /**
     * @brief Constructs a UUID from a string.
     * @param _str UUID string in 8-4-4-4-12 format.
     */
    uuid(std::string const& _str);

    /**
     * @brief Constructs a UUID from a 16 byte array.
     * @param _str UUID stored in an unsigned 128-bit integer.
     */
    uuid(__uint128_t _bytes);

    /**
     * @brief Copy constructor.
     * @param _rhs UUID to copy.
     */
    uuid(uuid const& _rhs);

    /**
     * @brief Move constructor.
     * @param _rhs UUID to move from.
     */
    uuid(uuid&& _rhs);

    ~uuid() = default;

    /**
     * @brief Copy assignment operator.
     * @param _rhs UUID to copy.
     * @return Reference to this UUID.
     */
    auto operator=(__uint128_t _rhs) -> uuid&;

    /**
     * @brief Copy assignment operator.
     * @param _rhs UUID to copy.
     * @return Reference to this UUID.
     */
    auto operator=(uuid const& _rhs) -> uuid&;

    /**
     * @brief Move assignment operator.
     * @param _rhs UUID to move from.
     * @return Reference to this UUID.
     */
    auto operator=(uuid&& _rhs) -> uuid&;

    /**
     * @brief Equality comparison.
     * @param _rhs UUID to compare with.
     * @return True if UUIDs are equal.
     */
    auto operator==(uuid const& _rhs) -> bool;

    /**
     * @brief Inequality comparison.
     * @param _rhs UUID to compare with.
     * @return True if UUIDs are not equal.
     */
    auto operator!=(uuid const& _rhs) -> bool;

    /** @brief Converts to string. */
    operator std::string();

    /**
     * @brief Converts the UUID to a string.
     * @return UUID in 8-4-4-4-12 hexadecimal format.
     */
    auto to_string() const -> std::string;

    /**
     * @brief Converts the UUID to a 22 byte base-64 encoded string.
     * @return UUID in URL safe 22 bytes base-64 encoded string.
     */
    auto to_base64_string() const -> std::string;

    /**
     * @brief Converts the UUID to the textual representation of the underlying 128-bit integer.
     * @return UUID as the textual representation of the underlying 128-bit integer.
     */
    auto to_128bit_string() const -> std::string;

    /**
     * @brief Parses a UUID from a string.
     * @param _str UUID string to parse.
     * @return Reference to this UUID.
     */
    auto from_string(std::string const& _str) -> uuid&;

    /**
     * @brief Parses a UUID from a 22 byte base-64 encoded string.
     * @param _str UUID string to parse.
     * @return Reference to this UUID.
     */
    auto from_base64_string(std::string const& _str) -> uuid&;

    /**
     * @brief Writes the UUID to an output stream.
     * @param _out Output stream.
     * @return Reference to this UUID.
     */
    auto to_stream(std::ostream& _out) const -> uuid const&;

    /**
     * @brief Reads a UUID from an input stream.
     * @param _in Input stream.
     * @return Reference to this UUID.
     */
    auto from_stream(std::istream& _in) -> uuid&;

    /**
     * @brief Stream output operator.
     * @param _out Output stream.
     * @param _uuid UUID to output.
     * @return Reference to output stream.
     */
    friend auto operator<<(std::ostream& _out, zpt::uuid const& _uuid) -> std::ostream& {
        _uuid.to_stream(_out);
        return _out;
    }

    /**
     * @brief Stream input operator.
     * @param _in Input stream.
     * @param _uuid UUID to read into.
     * @return Reference to input stream.
     */
    friend auto operator>>(std::istream& _in, zpt::uuid& _uuid) -> std::istream& {
        _uuid.from_stream(_in);
        return _in;
    }

  private:
    __uint128_t __base; ///< 128-bit UUID value.
};

} // namespace zpt
