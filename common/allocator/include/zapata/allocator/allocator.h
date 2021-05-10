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
#include <map>

namespace zpt {
class memory_pool {
  public:
    memory_pool(size_t _memory_size);
    virtual ~memory_pool();

    auto allocate(size_t _bytes) -> unsigned char*;
    auto dispose(unsigned char* _ptr, size_t bytes);

  private:
    unsigned char* __underlying{ nullptr };
};
template<typename T>
class allocator {
  public:
    using value_type = T;
    using pointer = value_type*;
    using size_type = size_t;

    allocator(zpt::memory_pool& _mem);
    allocator(zpt::allocator<T> const& _rhs);
    allocator(zpt::allocator<T>&& _rhs);
    virtual ~allocator() = default;

    auto operator=(zpt::allocator<T> const& _rhs) -> zpt::allocator<T>&;
    auto operator=(zpt::allocator<T>&& _rhs) -> zpt::allocator<T>&;

    /**
      Allocates storage suitable for an array object of type T[n] and creates
      the array, but does not construct array elements. May throw exceptions.
     */
    auto allocate(size_type _n) -> pointer;
    /**
      Same as a.allocate(n), but may use cvp (nullptr or a pointer obtained
      from a.allocate()) in unspecified manner to aid locality.
    */
    auto allocate(size_type _n, pointer const _adjacent_to) -> pointer;
    /**
      Deallocates storage pointed to p, which must be a value returned by a
      previous call to allocate that has not been invalidated by an intervening
      call to deallocate. n must match the value previously passed to
      allocate. Does not throw exceptions.
     */
    auto deallocate(pointer _to_deallocate, size_type _n) -> void;
    /**
      The largest value that can be passed to allocate().
     */
    auto max_size() -> size_type;
    /**
      Constructs an object of type X in previously-allocated storage at the
      address pointed to by xp, using args as the constructor arguments.
     */
    template<class C, typename... Args>
    auto construct(C* _object, Args... _args) -> void;
    /**
      Destructs an object of type X pointed to by xp, but does not
      deallocate any storage.
     */
    template<class C>
    auto destroy(C* _object) -> void;

  private:
    // std::map<T*, size_t> __allocated;
    zpt::memory_pool& __memory_pool;
};
} // namespace zpt

template<typename T>
zpt::allocator<T>::allocator(zpt::memory_pool& _mem)
  : __memory_pool{ _mem } {}

template<typename T>
zpt::allocator<T>::allocator(zpt::allocator<T> const& _rhs) {}

template<typename T>
zpt::allocator<T>::allocator(zpt::allocator<T>&& _rhs) {}

template<typename T>
auto
zpt::allocator<T>::operator=(zpt::allocator<T> const& _rhs) -> zpt::allocator<T>& {
    return (*this);
}

template<typename T>
auto
zpt::allocator<T>::operator=(zpt::allocator<T>&& _rhs) -> zpt::allocator<T>& {
    return (*this);
}

template<typename T>
auto
zpt::allocator<T>::allocate(size_type _n) -> pointer {
    return nullptr;
}

template<typename T>
auto
zpt::allocator<T>::allocate(size_type _n, pointer const _adjacent_to) -> pointer {
    return nullptr;
}

template<typename T>
auto
zpt::allocator<T>::deallocate(pointer _to_deallocate, size_type _n) -> void {}

template<typename T>
auto
zpt::allocator<T>::max_size() -> size_type {
    return 0;
}

template<typename T>
template<class C, typename... Args>
auto
zpt::allocator<T>::construct(C* _object, Args... _args) -> void {}

template<typename T>
template<class C>
auto
zpt::allocator<T>::destroy(C* _object) -> void {}
