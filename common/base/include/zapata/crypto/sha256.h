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
 * @file sha256.h
 * @brief SHA-256 cryptographic hash implementation.
 *
 * Provides a pure C++ implementation of the SHA-256 hash algorithm.
 * Produces a 256-bit (32-byte) hash digest.
 */

#pragma once

#include <cinttypes>
#include <cstdint>
#include <string>

/** @name SHA-2 Helper Macros
 * Internal macros for SHA-2 algorithm operations.
 * @{
 */
#define SHA2_SHFR(x, n) (x >> n)
#define SHA2_ROTR(x, n) ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n) ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z) ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x, 2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x, 6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x, 7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x, 3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                                                                      \
    {                                                                                              \
        *((str) + 3) = (std::uint8_t)((x));                                                        \
        *((str) + 2) = (std::uint8_t)((x) >> 8);                                                   \
        *((str) + 1) = (std::uint8_t)((x) >> 16);                                                  \
        *((str) + 0) = (std::uint8_t)((x) >> 24);                                                  \
    }
#define SHA2_PACK32(str, x)                                                                        \
    {                                                                                              \
        *(x) = ((std::uint32_t)*((str) + 3)) | ((std::uint32_t)*((str) + 2) << 8) |                \
               ((std::uint32_t)*((str) + 1) << 16) | ((std::uint32_t)*((str) + 0) << 24);          \
    }
/** @} */

namespace zpt::crypto {

/**
 * @brief SHA-256 hash algorithm implementation.
 *
 * Implements the SHA-256 cryptographic hash function as defined in FIPS 180-4.
 * Produces a 256-bit (32-byte) digest.
 *
 * @par Example Usage (Low-level API)
 * @code
 * zpt::crypto::SHA256 hasher;
 * hasher.init();
 * hasher.update(reinterpret_cast<const unsigned char*>(data), len);
 * unsigned char digest[SHA256::DIGEST_SIZE];
 * hasher.finalize(digest);
 * @endcode
 *
 * @see zpt::crypto::sha256() for a simpler string-based interface.
 */
class SHA256 {
  protected:
    static const std::uint32_t sha256_k[];                           ///< Round constants.
    static constexpr unsigned int SHA224_256_BLOCK_SIZE = (512 / 8); ///< Block size in bytes.

  public:
    /** @brief Size of the output digest in bytes (32). */
    static constexpr unsigned int DIGEST_SIZE = (256 / 8);

    /**
     * @brief Initializes the hash state.
     *
     * Must be called before update() to reset the hasher.
     * @return void
     */
    void init();

    /**
     * @brief Updates the hash with additional data.
     * @param message Pointer to the data to hash.
     * @param len Length of the data in bytes.
     *
     * Can be called multiple times to hash data incrementally.
     * @return void
     */
    void update(const unsigned char* message, unsigned int len);

    /**
     * @brief Finalizes the hash and outputs the digest.
     * @param digest Buffer to receive the 32-byte hash digest.
     * @return none
     *
     * After calling finalize(), call init() before hashing new data.
     */
    void finalize(unsigned char* digest);

  protected:
    /**
     * @brief Processes a block of data.
     * @param message Pointer to the message block.
     * @param block_nb Number of blocks to process.
     * @return void
     */
    void transform(const unsigned char* message, unsigned int block_nb);

    unsigned int m_tot_len;                           ///< Total message length.
    unsigned int m_len;                               ///< Current block length.
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE]; ///< Message block buffer.
    std::uint32_t m_h[8];                             ///< Hash state.
};

/**
 * @brief Computes SHA-256 hash of a string.
 * @param input The string to hash.
 * @return Hexadecimal string representation of the 256-bit hash.
 *
 * @par Example Usage
 * @code
 * std::string hash = zpt::crypto::sha256("Hello, World!");
 * // hash = "dffd6021bb2bd5b0af676290809ec3a53191dd81c7f70a4b28688a362182986f"
 * @endcode
 */
std::string sha256(std::string const& input);

} // namespace zpt::crypto
