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
#include <cstdint>

#define SHA2_SHFR(x, n) (x >> n)
#define SHA2_ROTR(x, n) ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n) ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z) ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA512_F1(x) (SHA2_ROTR(x, 28) ^ SHA2_ROTR(x, 34) ^ SHA2_ROTR(x, 39))
#define SHA512_F2(x) (SHA2_ROTR(x, 14) ^ SHA2_ROTR(x, 18) ^ SHA2_ROTR(x, 41))
#define SHA512_F3(x) (SHA2_ROTR(x, 1) ^ SHA2_ROTR(x, 8) ^ SHA2_SHFR(x, 7))
#define SHA512_F4(x) (SHA2_ROTR(x, 19) ^ SHA2_ROTR(x, 61) ^ SHA2_SHFR(x, 6))
#define SHA2_UNPACK32(x, str)                                                                      \
    {                                                                                              \
        *((str) + 3) = (std::uint8_t)((x));                                                        \
        *((str) + 2) = (std::uint8_t)((x) >> 8);                                                   \
        *((str) + 1) = (std::uint8_t)((x) >> 16);                                                  \
        *((str) + 0) = (std::uint8_t)((x) >> 24);                                                  \
    }
#define SHA2_UNPACK64(x, str)                                                                      \
    {                                                                                              \
        *((str) + 7) = (std::uint8_t)((x));                                                        \
        *((str) + 6) = (std::uint8_t)((x) >> 8);                                                   \
        *((str) + 5) = (std::uint8_t)((x) >> 16);                                                  \
        *((str) + 4) = (std::uint8_t)((x) >> 24);                                                  \
        *((str) + 3) = (std::uint8_t)((x) >> 32);                                                  \
        *((str) + 2) = (std::uint8_t)((x) >> 40);                                                  \
        *((str) + 1) = (std::uint8_t)((x) >> 48);                                                  \
        *((str) + 0) = (std::uint8_t)((x) >> 56);                                                  \
    }
#define SHA2_PACK64(str, x)                                                                        \
    {                                                                                              \
        *(x) = ((std::uint64_t) * ((str) + 7)) | ((std::uint64_t) * ((str) + 6) << 8) |            \
               ((std::uint64_t) * ((str) + 5) << 16) | ((std::uint64_t) * ((str) + 4) << 24) |     \
               ((std::uint64_t) * ((str) + 3) << 32) | ((std::uint64_t) * ((str) + 2) << 40) |     \
               ((std::uint64_t) * ((str) + 1) << 48) | ((std::uint64_t) * ((str) + 0) << 56);      \
    }

namespace zpt::crypto {
class SHA512 {
  protected:
    static const std::uint64_t sha512_k[];
    static constexpr unsigned int SHA384_512_BLOCK_SIZE = (1024 / 8);

  public:
    void init();
    void update(const unsigned char* message, unsigned int len);
    void finalize(unsigned char* digest);
    static constexpr unsigned int DIGEST_SIZE = (512 / 8);

  protected:
    void transform(const unsigned char* message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA384_512_BLOCK_SIZE];
    std::uint64_t m_h[8];
};

std::string sha512(std::string const& input);
} // namespace zpt::crypto
