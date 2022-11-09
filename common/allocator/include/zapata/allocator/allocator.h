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

#include <zapata/base/expect.h>

namespace zpt {
template<typename T>
class object_pool {
  public:
    using pointer_type = unsigned char*;

    object_pool(size_t _max_memory);
    virtual ~object_pool();

    auto acquire(size_t _n = 1) -> pointer_type;
    auto restore(pointer_type _ptr, size_t _n = 1) -> void;
    auto restore(T* _ptr) -> void;
    auto count() const -> unsigned long long;
    auto to_string() const -> std::string;

  private:
    std::map<size_t, size_t> __available;
    alignas(T) std::unique_ptr<unsigned char[]> __underlying{ nullptr };
    size_t __max_size{ 0 };
    size_t __current_addr{ 0 };
    std::atomic<unsigned long long> __object_count{ 0 };

    auto normalize(std::map<size_t, size_t>::iterator& _added) -> void;
};

template<typename T>
class allocator {
  public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = value_type const*;
    using size_type = size_t;

    allocator(size_t _max_memory);
    template<class U>
    constexpr allocator(zpt::allocator<U> const&) noexcept;
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
      Deallocates storage pointed to p, which must be a value returned by a
      previous call to allocate that has not been invalidated by an intervening
      call to deallocate. n must match the value previously passed to
      allocate. Does not throw exceptions.
     */
    auto deallocate(pointer _to_deallocate, size_type _n) -> void;
    auto max_memory_size() const -> size_type;
    auto to_string() const -> std::string;

    friend auto operator<<(std::ostream& _out, zpt::allocator<T> const& _in) -> std::ostream& {
        _out << _in.to_string() << std::flush;
        return _out;
    }

  private:
    size_t __max_size{ 0 };

    auto get_pool() -> zpt::object_pool<T>&;
    auto get_pool() const -> zpt::object_pool<T> const&;
    static auto get_pool(size_t _max_memory) -> zpt::object_pool<T>&;
};
} // namespace zpt

template<typename T>
zpt::object_pool<T>::object_pool(size_t _max_memory)
  : __underlying{ std::make_unique<unsigned char[]>(_max_memory) }
  , __max_size{ _max_memory }
  , __current_addr{ 0 }
  , __object_count{ 0 } {}

template<typename T>
zpt::object_pool<T>::~object_pool() {}

template<typename T>
auto zpt::object_pool<T>::acquire(size_t _n) -> pointer_type {
    size_t _byte_size = _n * sizeof(T);
    if (this->__available.size() != 0) {
        for (auto [_addr, _size] : this->__available) {
            if (_size >= _byte_size) {
                auto _return = &this->__underlying[_addr];
                this->__available.erase(_addr);
                if (_size - _byte_size != 0) {
                    this->__available.insert(std::make_pair(_addr + _byte_size, _size - _byte_size));
                }
                this->__object_count += _n;
                return _return;
            }
        }
    }
    expect(this->__current_addr + _byte_size < this->__max_size, "no more memory available");
    auto _return = &this->__underlying[this->__current_addr];
    this->__current_addr += _byte_size;
    this->__object_count += _n;
    return _return;
}

template<typename T>
auto zpt::object_pool<T>::restore(pointer_type _ptr, size_t _n) -> void {
    size_t _addr = _ptr - &this->__underlying[0];
    auto [_iterator, _inserted] = this->__available.insert(std::make_pair(_addr, _n * sizeof(T)));
    if (_inserted) { this->normalize(_iterator); }
    this->__object_count -= _n;
}

template<typename T>
auto zpt::object_pool<T>::count() const -> unsigned long long {
    return this->__object_count.load();
}

template<typename T>
auto zpt::object_pool<T>::to_string() const -> std::string {
    std::ostringstream _oss;
    _oss << "- main chunk: (" << this->__current_addr << "/" << this->__max_size << ")";
    if (this->__available.size() != 0) {
        _oss << std::endl << "- previously allocated and available chunks:";
        for (auto [_addr, _size] : this->__available) {
            _oss << " (" << _addr << ", " << _size << ")" << std::flush;
        }
    }
    return _oss.str();
}

template<typename T>
auto zpt::object_pool<T>::normalize(std::map<size_t, size_t>::iterator& _added) -> void {
    auto _prev = _added;
    --_prev;
    if (_prev != _added) {
        if (_prev->first + _prev->second == _added->first) {
            _prev->second += _added->second;
            this->__available.erase(_added);
            _added = _prev;
        }
    }
    auto _next = _added;
    ++_next;
    if (_next != this->__available.end()) {
        if (_added->first + _added->second == _next->first) {
            _added->second += _next->second;
            this->__available.erase(_next);
        }
    }
}

template<typename T>
zpt::allocator<T>::allocator(size_t _max_memory)
  : __max_size{ _max_memory } {}

template<typename T>
template<class U>
constexpr zpt::allocator<T>::allocator(zpt::allocator<U> const& _rhs) noexcept
  : __max_size{ _rhs.max_memory_size() } {}

template<typename T>
zpt::allocator<T>::allocator(zpt::allocator<T> const& _rhs)
  : __max_size{ _rhs.__max_size } {}

template<typename T>
zpt::allocator<T>::allocator(zpt::allocator<T>&& _rhs)
  : __max_size{ _rhs.__max_size } {
    _rhs.__max_size = 0;
}

template<typename T>
auto zpt::allocator<T>::operator=(zpt::allocator<T> const& _rhs) -> zpt::allocator<T>& {
    this->__max_size = _rhs.__max_size;
    return (*this);
}

template<typename T>
auto zpt::allocator<T>::operator=(zpt::allocator<T>&& _rhs) -> zpt::allocator<T>& {
    this->__max_size = _rhs.__max_size;
    _rhs.__max_size = 0;
    return (*this);
}

template<typename T>
auto zpt::allocator<T>::allocate(size_type _n) -> pointer {
    try {
        return reinterpret_cast<pointer>(this->get_pool().acquire(_n));
    }
    catch (zpt::failed_expectation const& _e) {
    }
    throw std::bad_alloc{};
}

template<typename T>
auto zpt::allocator<T>::deallocate(pointer _to_deallocate, size_type _n) -> void {
    this->get_pool().restore(reinterpret_cast<zpt::object_pool<T>::pointer_type>(_to_deallocate), _n);
}

template<typename T>
auto zpt::allocator<T>::max_memory_size() const -> size_type {
    return this->__max_size;
}

template<typename T>
auto zpt::allocator<T>::to_string() const -> std::string {
    unsigned long long _count = this->get_pool().count();
    std::ostringstream _oss;
    _oss << "zpt::allocator<" << typeid(T).name() << ">:" << std::endl
         << "- max of " << this->__max_size << " bytes of allocatable memory per thread" << std::endl
         << "- holds " << _count << " objects in a total of " << (_count * sizeof(T))
         << " bytes allocated by the current thread" << std::endl
         << this->get_pool().to_string() << std::flush;
    return _oss.str();
}

template<typename T>
auto zpt::allocator<T>::get_pool() -> zpt::object_pool<T>& {
    expect(this->__max_size != 0, "can't allocate in a memory chunk of zero bytes");
    return zpt::allocator<T>::get_pool(this->__max_size);
}

template<typename T>
auto zpt::allocator<T>::get_pool() const -> zpt::object_pool<T> const& {
    expect(this->__max_size != 0, "can't allocate in a memory chunk of zero bytes");
    return zpt::allocator<T>::get_pool(this->__max_size);
}

template<typename T>
auto zpt::allocator<T>::get_pool(size_t _max_memory) -> zpt::object_pool<T>& {
    static thread_local zpt::object_pool<T> _object_pool{ _max_memory };
    return _object_pool;
}
