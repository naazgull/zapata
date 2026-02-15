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
 * @file sha1.h
 * @brief SHA-1 cryptographic hash implementation.
 *
 * Provides a pure C++ implementation of the SHA-1 hash algorithm.
 * Produces a 160-bit (20-byte) hash digest.
 *
 * @warning SHA-1 is considered cryptographically weak. Use SHA-256 or
 *          SHA-512 for security-sensitive applications.
 */

#pragma once

#include <cinttypes>
#include <cstdint>
#include <iostream>
#include <string>

namespace zpt::crypto {

/**
 * @brief SHA-1 hash algorithm implementation.
 *
 * Implements the SHA-1 cryptographic hash function. Supports incremental
 * hashing via multiple update() calls.
 *
 * @par Example Usage
 * @code
 * zpt::crypto::SHA1 hasher;
 * hasher.update("Hello, ");
 * hasher.update("World!");
 * std::string hash = hasher.finalize();
 * @endcode
 *
 * @see zpt::crypto::sha1() for a simpler one-shot interface.
 * @see zpt::crypto::SHA256 for a more secure alternative.
 */
class SHA1 {
  public:
    /**
     * @brief Constructs and initializes a SHA1 hasher.
     */
    SHA1();

    /**
     * @brief Updates the hash with a string.
     * @param s String data to hash.
     */
    void update(const std::string& s);

    /**
     * @brief Updates the hash with data from a stream.
     * @param is Input stream to read from.
     */
    void update(std::istream& is);

    /**
     * @brief Finalizes and returns the hash.
     * @return Hexadecimal string representation of the 160-bit hash.
     */
    std::string finalize();

    /**
     * @brief Computes SHA-1 hash of a file.
     * @param filename Path to the file.
     * @return Hexadecimal hash string.
     */
    static std::string from_file(const std::string& filename);

  private:
    static constexpr unsigned int DIGEST_INTS = 5;  ///< 32-bit integers per digest.
    static constexpr unsigned int BLOCK_INTS = 16;  ///< 32-bit integers per block.
    static constexpr unsigned int BLOCK_BYTES = BLOCK_INTS * 4; ///< Bytes per block.

    std::uint32_t digest[DIGEST_INTS]; ///< Current hash state.
    std::string buffer;                ///< Pending data buffer.
    std::uint64_t transforms;          ///< Number of transforms performed.

    void reset();
    void transform(std::uint32_t block[BLOCK_BYTES]);
    static void buffer_to_block(const std::string& buffer, std::uint32_t block[BLOCK_BYTES]);
    static void read(std::istream& is, std::string& s, int max);
};

/**
 * @brief Computes SHA-1 hash of a string.
 * @param string The string to hash.
 * @return Hexadecimal string representation of the 160-bit hash.
 *
 * @par Example Usage
 * @code
 * std::string hash = zpt::crypto::sha1("Hello, World!");
 * @endcode
 */
std::string sha1(const std::string& string);

} // namespace zpt::crypto
