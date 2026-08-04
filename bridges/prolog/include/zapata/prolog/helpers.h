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

/**
 * @brief Wrapper around SWI-Prolog `term_t` with shared ownership.
 *
 * Manages Prolog term references with automatic cleanup and reference counting.
 * Supports construction from raw term_t, from Prolog string expressions,
 * and from existing term instances.
 */
class term {
  public:
    /** @brief Constructs an empty term reference using `PL_new_term_ref`. */
    term();
    /** @brief Constructs a term from an existing SWI-Prolog term reference. */
    term(term_t _to_assign);
    /** @brief Parses a Prolog string expression (e.g. "foo(1, 2)") into a new term. */
    term(std::string const& _to_parse);
    /** @brief Copies a term, incrementing the shared reference count. */
    term(term const& _rhs);
    /** @brief Moves a term, transferring ownership of the underlying reference. */
    term(term&& _rhs);
    /** @brief Frees the underlying term reference when the reference count reaches zero. */
    virtual ~term();

    /** @brief Implicit conversion to the raw SWI-Prolog `term_t` type. */
    operator term_t();
    /** @brief Returns a reference to the underlying `term_t` pointer. */
    auto operator*() -> term_t&;
    /** @brief Assigns a raw `term_t` to this term. Must not be currently shared. */
    auto operator=(term_t _rhs) -> term&;
    /** @brief Assigns from another term, sharing the underlying reference. */
    auto operator=(term const& _rhs) -> term&;
    /** @brief Moves assignment from another term. */
    auto operator=(term&& _rhs) -> term&;
    /** @brief Compares underlying term references for equality. */
    auto operator==(term const& _rhs) -> bool;
    /** @brief Compares underlying term references for inequality. */
    auto operator!=(term const& _rhs) -> bool;
    /** @brief Emplaces a new child term and returns a reference to it. */
    auto emplace() -> term;
    /** @brief Appends a child term to this term's children list. */
    auto add(term const& _to_add) -> term&;
    /** @brief Serializes the underlying Prolog term to a string representation. */
    auto to_string() const -> std::string;
    /** @brief Returns a static null term (underlying pointer is 0). */
    static auto null() -> term&;

    /** @brief Streams the term's string representation into an output stream. */
    friend auto operator<<(std::ostream& _os, term const& _in) -> std::ostream& {
        _os << _in.to_string();
        return _os;
    }

  private:
    term_t __underlying{ 0 };     ///< Raw SWI-Prolog term reference
    std::vector<term> __children; ///< Child terms
    std::shared_ptr<zpt::padded_atomic<size_t>> __references{ nullptr }; ///< Shared reference count
};

/** @brief Converts a raw SWI-Prolog term to a string representation using `PL_write_term`. */
auto term_to_string(term_t _to_convert) -> std::string;
} // namespace prolog
} // namespace zpt
