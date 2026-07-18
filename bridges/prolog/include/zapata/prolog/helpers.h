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
 * @file interface.h
 * @brief Prolog bridge helper classes.
 */

#pragma once

#include <SWI-Prolog.h>
#include <vector>
#include <zapata/atomics/padded_atomic.h>

namespace zpt {
namespace prolog {
class term {
  public:
    term();
    term(term_t _to_assign);
    term(std::string const& _to_parse);
    term(term const& _rhs);
    term(term&& _rhs);
    virtual ~term();

    operator term_t();
    auto operator*() -> term_t&;
    auto operator=(term_t _rhs) -> term&;
    auto operator=(term const& _rhs) -> term&;
    auto operator=(term&& _rhs) -> term&;
    auto operator==(term const& _rhs) -> bool;
    auto operator!=(term const& _rhs) -> bool;
    auto emplace() -> term;
    auto add(term const& _to_add) -> term&;
    auto to_string() const -> std::string;
    static auto null() -> term&;

    friend auto operator<<(std::ostream& _os, term const& _in) -> std::ostream& {
        _os << _in.to_string();
        return _os;
    }

  private:
    term_t __underlying{ 0 };
    std::vector<term> __children;
    std::shared_ptr<zpt::padded_atomic<size_t>> __references{ nullptr };
};

auto term_to_string(term_t _to_convert) -> std::string;
} // namespace prolog
} // namespace zpt
