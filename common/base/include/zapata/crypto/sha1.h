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

#include <iostream>
#include <string>
#include <cstdint>

namespace zpt::crypto {
class SHA1 {
  public:
    SHA1();
    void update(const std::string& s);
    void update(std::istream& is);
    std::string finalize();
    static std::string from_file(const std::string& filename);

  private:
    static constexpr unsigned int DIGEST_INTS = 5; /* number of 32bit integers per SHA1 digest */
    static constexpr unsigned int BLOCK_INTS = 16; /* number of 32bit integers per SHA1 block */
    static constexpr unsigned int BLOCK_BYTES = BLOCK_INTS * 4;

    std::uint32_t digest[DIGEST_INTS];
    std::string buffer;
    std::uint64_t transforms;

    void reset();
    void transform(std::uint32_t block[BLOCK_BYTES]);

    static void buffer_to_block(const std::string& buffer, std::uint32_t block[BLOCK_BYTES]);
    static void read(std::istream& is, std::string& s, int max);
};

std::string sha1(const std::string& string);
} // namespace zpt::crypto
