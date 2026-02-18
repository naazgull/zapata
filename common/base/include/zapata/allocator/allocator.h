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
 * @file allocator.h
 * @brief Memory pool and STL-compatible allocator.
 *
 * Provides a bounded memory pool and a custom allocator that can be used
 * with STL containers to limit memory usage.
 */

#pragma once

#include <atomic>
#include <cassert>
#include <map>
#include <memory>
#include <stddef.h>
#include <vector>
#include <zapata/atomics/padded_atomic.h>
#include <zapata/base/expect.h>

namespace zpt {
namespace mem {

/**
 * @brief Bounded memory pool with atomic tracking.
 *
 * A simple memory pool that tracks allocations against a maximum limit.
 * Uses atomic counters for thread-safe allocation tracking.
 *
 * @par Thread Safety
 * All methods are thread-safe.
 */
class pool {
  public:
    using pointer_type = void*; ///< Generic pointer type.

    /**
     * @brief Creates a pool with the specified maximum memory.
     * @param _max_memory Maximum bytes this pool can allocate.
     */
    pool(size_t _max_memory);

    virtual ~pool();

    /**
     * @brief Allocates memory from the pool.
     * @param _n Number of bytes to allocate.
     * @return Pointer to allocated memory.
     * @throws zpt::failed_expectation If allocation would exceed max_size.
     */
    auto allocate(size_t _n) -> pointer_type;

    /**
     * @brief Returns memory to the pool.
     * @param _ptr Pointer previously returned by allocate().
     * @param _n Size that was originally allocated.
     */
    auto deallocate(pointer_type _ptr, size_t _n) -> void;

    /**
     * @brief Returns the maximum pool size.
     * @return Maximum bytes this pool can allocate.
     */
    auto max_size() const -> size_t;

    /**
     * @brief Returns the currently allocated size.
     * @return Bytes currently allocated from this pool.
     */
    auto allocated_size() const -> size_t;

  private:
    zpt::padded_atomic<size_t> __max_size{ 0 };       ///< Maximum allowed allocation.
    zpt::padded_atomic<size_t> __allocated_size{ 0 }; ///< Current allocation.
};

} // namespace mem

/**
 * @brief Returns the global memory pool singleton.
 * @param _max_mem If non-zero on first call, sets the pool's maximum size.
 * @return Reference to the global memory pool.
 */
auto MEM_POOL(std::uint64_t _max_mem = 0) -> zpt::mem::pool&;

/**
 * @brief STL-compatible allocator backed by a memory pool.
 * @tparam T The type to allocate.
 *
 * This allocator can be used with STL containers to bound their memory usage.
 * It delegates allocation to a zpt::mem::pool instance.
 *
 * @par Example Usage
 * @code
 * zpt::mem::pool my_pool{1024 * 1024};  // 1MB pool
 * zpt::allocator<int> alloc{my_pool};
 * std::vector<int, zpt::allocator<int>> vec{alloc};
 * @endcode
 *
 * @note The allocator is non-copyable by assignment but copy-constructible
 *       (required for container rebinding).
 */
template<typename T>
class allocator {
  public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = value_type const*;
    using void_pointer = void*;
    using const_void_pointer = void const*;
    using size_type = size_t;

    zpt::mem::pool& __pool; ///< Reference to the backing memory pool.

    /**
     * @brief Constructs an allocator using the given pool.
     * @param _pool The memory pool to allocate from.
     */
    allocator(zpt::mem::pool& _pool);

    /**
     * @brief Rebinding copy constructor.
     * @tparam U Source allocator's value type.
     * @param _rhs Source allocator.
     */
    template<typename U>
    allocator(zpt::allocator<U> const& _rhs);

    /**
     * @brief Copy constructor.
     * @param _rhs Source allocator.
     */
    allocator(zpt::allocator<T> const& _rhs);

    virtual ~allocator() = default;

    allocator(zpt::allocator<T>&& _rhs) = delete;
    auto operator=(zpt::allocator<T> const& _rhs) -> zpt::allocator<T>& = delete;
    auto operator=(zpt::allocator<T>&& _rhs) -> zpt::allocator<T>& = delete;

    /**
     * @brief Allocates storage for n objects of type T.
     * @param _n Number of objects to allocate space for.
     * @return Pointer to the allocated storage.
     * @throws std::bad_alloc If pool limit would be exceeded.
     *
     * Allocates storage suitable for an array object of type T[n] and creates
     * the array, but does not construct array elements.
     */
    auto allocate(size_type _n) -> pointer;

    /**
     * @brief Deallocates storage.
     * @param _to_deallocate Pointer returned by a previous allocate() call.
     * @param _n Size that was originally passed to allocate().
     *
     * Deallocates storage pointed to by _to_deallocate, which must be a value
     * returned by a previous call to allocate() that has not been invalidated
     * by an intervening call to deallocate(). Does not throw exceptions.
     */
    auto deallocate(pointer _to_deallocate, size_type _n) -> void;

    /**
     * @brief Returns the maximum allocation size.
     * @return Maximum number of objects that can be allocated.
     */
    auto max_size() const -> size_type;

    /**
     * @brief Constructs an object in allocated storage.
     * @tparam U Type to construct.
     * @tparam Args Constructor argument types.
     * @param p Pointer to storage.
     * @param args Constructor arguments.
     */
    template<class U, class... Args>
    auto construct(U* p, Args&&... args) -> void;

    /**
     * @brief Destroys an object without deallocating storage.
     * @param p Pointer to the object to destroy.
     */
    auto destroy(pointer p) -> void;
};

} // namespace zpt

template<class T>
bool operator==(zpt::allocator<T> const& a1, zpt::allocator<T> const& a2) {
    return true;
}

template<class T>
bool operator!=(zpt::allocator<T> const& a1, zpt::allocator<T> const& a2) {
    return false;
}

template<typename T>
zpt::allocator<T>::allocator(zpt::mem::pool& _pool)
  : __pool{ _pool } {}

template<typename T>
template<typename U>
zpt::allocator<T>::allocator(zpt::allocator<U> const& _rhs)
  : __pool{ _rhs.__pool } {}

template<typename T>
zpt::allocator<T>::allocator(zpt::allocator<T> const& _rhs)
  : __pool{ _rhs.__pool } {}

template<typename T>
auto zpt::allocator<T>::allocate(size_type _n) -> pointer {
    try {
        return reinterpret_cast<pointer>(this->__pool.allocate(sizeof(T) * _n));
    }
    catch (zpt::failed_expectation const& _e) {
    }
    throw std::bad_alloc{};
}

template<typename T>
auto zpt::allocator<T>::deallocate(pointer _to_deallocate, size_type _n) -> void {
    this->__pool.deallocate(reinterpret_cast<zpt::mem::pool::pointer_type>(_to_deallocate),
                            sizeof(T) * _n);
}

template<typename T>
auto zpt::allocator<T>::max_size() const -> size_type {
    return this->__pool.max_size();
}

template<typename T>
template<class U, class... Args>
auto zpt::allocator<T>::construct(U* p, Args&&... args) -> void {
    expect(p != nullptr, "can't initialize an object instance in unallocated memory");
    ::new ((void*)p) U(std::forward<Args>(args)...);
}

template<typename T>
auto zpt::allocator<T>::destroy(pointer p) -> void {
    expect(p != nullptr, "can't destroy an object instance from unallocated memory");
    p->~T();
}
