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

#include <stddef.h>
#include <vector>
#include <map>
#include <atomic>
#include <memory>
#include <cassert>

#include <zapata/atomics/padded_atomic.h>
#include <zapata/base/expect.h>

namespace zpt {
namespace mem {
class pool {
  public:
    using pointer_type = void*;

    pool(size_t _max_memory);
    virtual ~pool();

    auto allocate(size_t _n) -> pointer_type;
    auto deallocate(pointer_type _ptr, size_t _n) -> void;
    auto max_size() const -> size_t;
    auto allocated_size() const -> size_t;

  private:
    zpt::padded_atomic<size_t> __max_size{ 0 };
    zpt::padded_atomic<size_t> __allocated_size{ 0 };
};
} // namespace mem

template<typename T>
class allocator {
  public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = value_type const*;
    using void_pointer = void*;
    using const_void_pointer = void const*;
    using size_type = size_t;

    zpt::mem::pool& __pool;

    allocator(zpt::mem::pool& _pool);
    template<typename U>
    allocator(zpt::allocator<U> const& _rhs);
    allocator(zpt::allocator<T> const& _rhs);
    virtual ~allocator() = default;

    allocator(zpt::allocator<T>&& _rhs) = delete;
    auto operator=(zpt::allocator<T> const& _rhs) -> zpt::allocator<T>& = delete;
    auto operator=(zpt::allocator<T>&& _rhs) -> zpt::allocator<T>& = delete;

    /**
      Allocates storage suitable for an array object of type T[n] and creates
      the array, but does not construct array elements. May throw exceptions.
     */
    auto allocate(size_type _n) -> pointer;
    /**
      Deallocates storage pointed to p, which must be a value returned by a
      previous call to allocate that has not been invalidated by an intervening
      call to deallocate. n must match the value previously passed to
      allocate. Does not throw exceptions.
     */
    auto deallocate(pointer _to_deallocate, size_type _n) -> void;
    auto max_size() const -> size_type;
    template<class U, class... Args>
    auto construct(U* p, Args&&... args) -> void;
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
