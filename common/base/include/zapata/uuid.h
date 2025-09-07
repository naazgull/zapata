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

namespace zpt {
class uuid {
  public:
    uuid();
    uuid(std::string const& _str);
    uuid(uuid const& _rhs);
    uuid(uuid&& _rhs);
    ~uuid() = default;

    auto operator=(uuid const& _rhs) -> uuid&;
    auto operator=(uuid&& _rhs) -> uuid&;

    auto operator==(uuid const& _rhs) -> bool;
    auto operator!=(uuid const& _rhs) -> bool;

    auto to_string() const -> std::string;
    auto from_string(std::string const& _str) -> uuid&;
    auto to_stream(std::ostream& _out) const -> uuid const&;
    auto from_stream(std::istream& _in) -> uuid&;

    friend auto operator<<(std::ostream& _out, zpt::uuid const& _uuid) -> std::ostream& {
        _uuid.to_stream(_out);
        return _out;
    }

    friend auto operator>>(std::istream& _in, zpt::uuid& _uuid) -> std::istream& {
        _uuid.from_stream(_in);
        return _in;
    }

  private:
    __uint128_t __base;
};
} // namespace zpt
