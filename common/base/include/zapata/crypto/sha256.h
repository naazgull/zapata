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

#pragma once

#include <string>

#define SHA2_SHFR(x, n) (x >> n)
#define SHA2_ROTR(x, n) ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n) ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z) ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x, 2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x, 6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x, 7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x, 3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                                                                                          \
    {                                                                                                                  \
        *((str) + 3) = (std::uint8_t)((x));                                                                            \
        *((str) + 2) = (std::uint8_t)((x) >> 8);                                                                       \
        *((str) + 1) = (std::uint8_t)((x) >> 16);                                                                      \
        *((str) + 0) = (std::uint8_t)((x) >> 24);                                                                      \
    }
#define SHA2_PACK32(str, x)                                                                                            \
    {                                                                                                                  \
        *(x) = ((std::uint32_t) * ((str) + 3)) | ((std::uint32_t) * ((str) + 2) << 8) |                                \
               ((std::uint32_t) * ((str) + 1) << 16) | ((std::uint32_t) * ((str) + 0) << 24);                          \
    }

namespace zpt::crypto {
class SHA256 {
  protected:
    static const std::uint32_t sha256_k[];
    static constexpr unsigned int SHA224_256_BLOCK_SIZE = (512 / 8);

  public:
    void init();
    void update(const unsigned char* message, unsigned int len);
    void finalize(unsigned char* digest);
    static constexpr unsigned int DIGEST_SIZE = (256 / 8);

  protected:
    void transform(const unsigned char* message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
    std::uint32_t m_h[8];
};

std::string
sha256(std::string const& input);
} // namespace zpt::crypto
