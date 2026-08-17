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
 * @file safe_access.h
 * @brief Thread-safe wrapper around a value with per-instance mutex.
 *
 * Combines an object of type `T` with a mutex of type `M` into a single
 * RAII-managed unit. Provides pointer-like access (`operator->`,
 * `operator*`) to the value and a `mutex()` accessor for external
 * locking. Used for global state in language bridges (Lua, Prolog)
 * where a JSON object must be safely shared across threads.
 */

#pragma once

#include <utility>

namespace zpt {
template<typename T, typename M>
class safe_access {
  public:
    /**
     * @brief Constructs the underlying value with forwarded arguments.
     * @tparam Args Argument types for the underlying value.
     * @param _args Arguments forwarded to the underlying value constructor.
     */
    template<typename... Args>
    safe_access(Args... _args);
    /**
     * @brief Destroys the underlying value.
     * @return void (the underlying value is destroyed as part of this instance).
     */
    virtual ~safe_access();

    /**
     * @brief Dereferences to the underlying value.
     * @return Reference to the underlying value.
     */
    auto operator*() -> T&;
    /**
     * @brief Accesses the underlying value via pointer.
     * @return Pointer to the underlying value.
     */
    auto operator->() -> T*;
    /**
     * @brief Returns the per-instance mutex.
     * @return Reference to the mutex, for external locking.
     */
    auto mutex() -> M&;

  private:
    T __underlying;
    mutable M __underlying_mutex;
};

} // namespace zpt

template<typename T, typename M>
template<typename... Args>
zpt::safe_access<T, M>::safe_access(Args... _args)
  : __underlying{ std::forward<Args>(_args)... } {}

template<typename T, typename M>
zpt::safe_access<T, M>::~safe_access() {}

template<typename T, typename M>
auto zpt::safe_access<T, M>::operator*() -> T& {
    return this->__underlying;
}

template<typename T, typename M>
auto zpt::safe_access<T, M>::operator->() -> T* {
    return &this->__underlying;
}

template<typename T, typename M>
auto zpt::safe_access<T, M>::mutex() -> M& {
    return this->__underlying_mutex;
}
