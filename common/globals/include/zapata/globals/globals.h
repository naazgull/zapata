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
 * @file globals.h
 * @brief Thread-local variable storage infrastructure.
 *
 * Provides a thread-local table for managing per-thread copies of variables,
 * keyed by memory address. Used internally by `zpt::thread_local_variable`
 * to provide safe per-thread variable instances.
 *
 * @see zpt::thread_local_variable
 */

#pragma once

#include <shared_mutex>
#include <typeinfo>
#include <zapata/base.h>
#include <zapata/locks/spin_mutex.h>

namespace zpt {

/**
 * @brief Abstract base class for type-erased thread-local table entries.
 */
class thread_local_table_entry {
  public:
    virtual ~thread_local_table_entry() = default;

    /** @brief Returns true if the entry holds no value. */
    virtual auto is_null() const -> bool = 0;
};

/**
 * @brief Thread-local storage table for per-thread variable instances.
 *
 * Maintains a `thread_local` map from variable addresses to type-erased
 * entries. Each thread gets its own map, allowing lock-free reads of
 * thread-local copies of shared variables.
 *
 * @note This is a low-level mechanism. Prefer `zpt::thread_local_variable<T>`
 *       for a higher-level interface.
 *
 * @see zpt::thread_local_variable
 */
class thread_local_table {
  public:
    using entry_type = std::unique_ptr<thread_local_table_entry>;
    using map_type = std::map<std::uintptr_t, entry_type>;

    /**
     * @brief Typed entry holding the actual per-thread value.
     * @tparam T Value type stored in the entry.
     */
    template<typename T>
    class entry : public thread_local_table_entry {
      public:
        /** @brief Constructs an entry, forwarding args to T's constructor. */
        template<typename... Args>
        entry(Args... _args);
        virtual ~entry() override;

        /** @brief Dereferences to the stored value. */
        auto operator*() -> T&;
        /** @brief Member access to the stored value. */
        auto operator->() -> T*;
        auto is_null() const -> bool override;

      private:
        T __underlying;
    };

    /**
     * @brief Allocates a thread-local copy of a variable.
     * @tparam P Variable pointer type (used as key).
     * @tparam T Value type to store.
     * @tparam Args Constructor argument types.
     * @param _member_variable Reference to the variable (address used as key).
     * @param _args Arguments forwarded to T's constructor.
     * @return Reference to the newly allocated value.
     */
    template<typename P, typename T, typename... Args>
    static auto alloc(P const& _member_variable, Args... _args) -> T&;

    /**
     * @brief Retrieves the thread-local copy of a variable.
     * @tparam P Variable pointer type.
     * @tparam T Expected value type.
     * @param _member_variable Reference to the variable (address used as key).
     * @return Reference to the thread-local value.
     * @throws zpt::ExpectationException If no entry exists for this variable.
     */
    template<typename P, typename T>
    static auto get(P const& _member_variable) -> T&;

    /**
     * @brief Deallocates the thread-local copy of a variable.
     * @tparam P Variable pointer type.
     * @tparam T Value type.
     * @param _member_variable Reference to the variable (address used as key).
     */
    template<typename P, typename T>
    static auto dealloc(P const& _member_variable) -> void;

    /** @brief Returns a debug string representation of the table. */
    static auto to_string() -> std::string;

  private:
    static thread_local inline map_type __variables{};
};
} // namespace zpt

template<typename T>
template<typename... Args>
zpt::thread_local_table::entry<T>::entry(Args... _args)
  : __underlying{ _args... } {}

template<typename T>
zpt::thread_local_table::entry<T>::~entry() {}

template<typename T>
auto zpt::thread_local_table::entry<T>::operator*() -> T& {
    return this->__underlying;
}

template<typename T>
auto zpt::thread_local_table::entry<T>::operator->() -> T* {
    return &this->__underlying;
}

template<typename T>
auto zpt::thread_local_table::entry<T>::is_null() const -> bool {
    return false;
}

template<typename P, typename T, typename... Args>
auto zpt::thread_local_table::alloc(P const& _member_variable, Args... _args) -> T& {
    entry<T>* _entry = new entry<T>(_args...);
    zpt::thread_local_table::__variables.emplace(
      reinterpret_cast<std::uintptr_t>(&_member_variable), std::move(entry_type{ _entry }));
    return **_entry;
}

template<typename P, typename T>
auto zpt::thread_local_table::get(P const& _member_variable) -> T& {
    auto _found = zpt::thread_local_table::__variables.find(
      reinterpret_cast<std::uintptr_t>(&_member_variable));
    expect(_found != zpt::thread_local_table::__variables.end(),
           "no such global variable for " << reinterpret_cast<std::uintptr_t>(&_member_variable));
    return *static_cast<entry<T>&>(*_found->second.get());
}

template<typename P, typename T>
auto zpt::thread_local_table::dealloc(P const& _member_variable) -> void {
    auto _found = zpt::thread_local_table::__variables.find(
      reinterpret_cast<std::uintptr_t>(&_member_variable));
    if (_found == zpt::thread_local_table::__variables.end()) { return; }
    zpt::thread_local_table::__variables.erase(_found);
}
