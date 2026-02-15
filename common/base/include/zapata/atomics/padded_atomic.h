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
 * @file padded_atomic.h
 * @brief Cache-line aligned atomic wrapper to prevent false sharing.
 *
 * Provides a wrapper around std::atomic that ensures proper alignment to
 * prevent false sharing in concurrent data structures.
 */

#pragma once

#include <atomic>
#include <cmath>
#include <cstddef>
#include <new>
#include <zapata/base/expect.h>

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
/** @brief L1 cache line size for constructive interference (co-location). */
constexpr std::size_t hardware_constructive_interference_size = 64;
/** @brief L1 cache line size for destructive interference (false sharing). */
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

namespace zpt {

/**
 * @brief Cache-line aligned atomic wrapper to prevent false sharing.
 * @tparam T The value type to wrap.
 *
 * In multi-threaded programs, false sharing occurs when threads on different
 * cores modify variables that share the same cache line. This wrapper aligns
 * the atomic variable to a cache line boundary, ensuring that each padded_atomic
 * occupies its own cache line.
 *
 * @par Example Usage
 * @code
 * struct counters {
 *     zpt::padded_atomic<size_t> counter1{0};  // Own cache line
 *     zpt::padded_atomic<size_t> counter2{0};  // Own cache line
 * };
 * @endcode
 *
 * @note This class is non-copyable but move-constructible.
 *
 * @see zpt::lf::queue
 * @see zpt::locks::spin_mutex
 */
template<typename T>
class padded_atomic {
  public:
    /** @brief Default constructor. */
    padded_atomic();

    /**
     * @brief Constructs with an initial value.
     * @param _value The initial value.
     */
    padded_atomic(T _value);

    padded_atomic(zpt::padded_atomic<T> const& _rhs) = delete;

    /**
     * @brief Move constructor.
     * @param _rhs The source to move from.
     */
    padded_atomic(zpt::padded_atomic<T>&& _rhs);

    virtual ~padded_atomic();

    auto operator=(zpt::padded_atomic<T> const& _rhs) -> zpt::padded_atomic<T>& = delete;
    auto operator=(zpt::padded_atomic<T>&& _rhs) -> zpt::padded_atomic<T>& = delete;

    /**
     * @brief Equality comparison with a value.
     * @param _rhs The value to compare against.
     * @return True if the stored value equals _rhs.
     */
    auto operator==(T const& _rhs) -> bool;

    /**
     * @brief Inequality comparison with a value.
     * @param _rhs The value to compare against.
     * @return True if the stored value does not equal _rhs.
     */
    auto operator!=(T const& _rhs) -> bool;

    /**
     * @brief Implicit conversion to the underlying type.
     * @return The current value (relaxed memory order).
     */
    operator T();

    /**
     * @brief Assigns a new value.
     * @param _rhs The value to store.
     * @return Reference to this object.
     */
    auto operator=(T const& _rhs) -> zpt::padded_atomic<T>&;

    /**
     * @brief Arrow operator for accessing std::atomic methods.
     * @return Pointer to the underlying atomic.
     */
    auto operator->() -> std::atomic<T>*;

    /** @copydoc operator->() */
    auto operator->() const -> std::atomic<T> const*;

    /**
     * @brief Dereference operator for accessing the std::atomic.
     * @return Reference to the underlying atomic.
     */
    auto operator*() -> std::atomic<T>&;

    /** @copydoc operator*() */
    auto operator*() const -> std::atomic<T> const&;

  private:
    /** @brief The underlying atomic, aligned to cache line boundary. */
    alignas(hardware_destructive_interference_size) std::atomic<T> __underlying;
};

} // namespace zpt

template<typename T>
zpt::padded_atomic<T>::padded_atomic()
  : __underlying{} {}

template<typename T>
zpt::padded_atomic<T>::padded_atomic(T _value)
  : __underlying{ _value } {}

template<typename T>
zpt::padded_atomic<T>::padded_atomic(zpt::padded_atomic<T>&& _rhs)
  : __underlying{ std::move(_rhs.__underlying) } {}

template<typename T>
zpt::padded_atomic<T>::~padded_atomic() {}

template<typename T>
auto zpt::padded_atomic<T>::operator==(T const& _rhs) -> bool {
    return (this->__underlying.load(std::memory_order_relaxed)) == _rhs;
}

template<typename T>
auto zpt::padded_atomic<T>::operator!=(T const& _rhs) -> bool {
    return !((*this) == _rhs);
}

template<typename T>
zpt::padded_atomic<T>::operator T() {
    return this->__underlying.load(std::memory_order_relaxed);
}

template<typename T>
auto zpt::padded_atomic<T>::operator=(T const& _rhs) -> zpt::padded_atomic<T>& {
    this->__underlying.store(_rhs);
    return (*this);
}

template<typename T>
auto zpt::padded_atomic<T>::operator->() -> std::atomic<T>* {
    return &this->__underlying;
}

template<typename T>
auto zpt::padded_atomic<T>::operator->() const -> std::atomic<T> const* {
    return &this->__underlying;
}

template<typename T>
auto zpt::padded_atomic<T>::operator*() -> std::atomic<T>& {
    return this->__underlying;
}

template<typename T>
auto zpt::padded_atomic<T>::operator*() const -> std::atomic<T> const& {
    return this->__underlying;
}
