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

#include <atomic>
#include <cstddef>
#include <cmath>
#include <zapata/base/expect.h>

namespace zpt {
template<typename T>
class padded_atomic {
  public:
    padded_atomic();
    padded_atomic(T _value);
    padded_atomic(zpt::padded_atomic<T> const& _rhs) = delete;
    padded_atomic(zpt::padded_atomic<T>&& _rhs);
    virtual ~padded_atomic();

    auto operator=(zpt::padded_atomic<T> const& _rhs) -> zpt::padded_atomic<T>& = delete;
    auto operator=(zpt::padded_atomic<T>&& _rhs) -> zpt::padded_atomic<T>&;

    auto operator==(T const& _rhs) -> bool;
    auto operator!=(T const& _rhs) -> bool;

    operator T();

    auto operator->() -> std::atomic<T>*;
    auto operator*() -> std::atomic<T>&;

  private:
    alignas(std::atomic<T>) std::byte* __storage{ nullptr };
    std::atomic<T>* __underlying{ nullptr };
};
} // namespace zpt

template<typename T>
zpt::padded_atomic<T>::padded_atomic()
  : __storage{ new std::byte[static_cast<size_t>(
      zpt::cache_line_size() * std::ceil(static_cast<double>(sizeof(std::atomic<T>)) /
                                         static_cast<double>(zpt::cache_line_size())))] }
  , __underlying{ new (this->__storage) std::atomic<T>() } {}

template<typename T>
zpt::padded_atomic<T>::padded_atomic(T _value)
  : zpt::padded_atomic<T>() {
    this->__underlying->store(_value);
}

template<typename T>
zpt::padded_atomic<T>::padded_atomic(zpt::padded_atomic<T>&& _rhs)
  : __storage{ _rhs.__storage }
  , __underlying{ _rhs.__underlying } {
    _rhs.__storage = nullptr;
    _rhs.__underlying = nullptr;
}

template<typename T>
zpt::padded_atomic<T>::~padded_atomic() {
    if (this->__underlying != nullptr) {
        this->__underlying->~atomic();
        this->__underlying = nullptr;
    }
    delete[] this->__storage;
    this->__storage = nullptr;
}

template<typename T>
auto
zpt::padded_atomic<T>::operator=(zpt::padded_atomic<T>&& _rhs) -> zpt::padded_atomic<T>& {
    this->__storage = _rhs.__storage;
    this->__underlying = _rhs.__underlying;
    _rhs.__storage = nullptr;
    _rhs.__underlying = nullptr;
    return (*this);
}

template<typename T>
auto
zpt::padded_atomic<T>::operator==(T const& _rhs) -> bool {
    return (this->__underlying->load()) == _rhs;
}

template<typename T>
auto
zpt::padded_atomic<T>::operator!=(T const& _rhs) -> bool {
    return !((*this) == _rhs);
}

template<typename T>
zpt::padded_atomic<T>::operator T() {
    return this->__underlying->load();
}

template<typename T>
auto
zpt::padded_atomic<T>::operator->() -> std::atomic<T>* {
    expect(this->__underlying != nullptr, "there is no underlying memory allocated", 500, 0);
    return this->__underlying;
}

template<typename T>
auto
zpt::padded_atomic<T>::operator*() -> std::atomic<T>& {
    expect(this->__underlying != nullptr, "there is no underlying memory allocated", 500, 0);
    return *this->__underlying;
}
