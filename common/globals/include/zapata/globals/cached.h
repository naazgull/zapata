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

#include <zapata/base.h>
#include <typeinfo>

namespace zpt {
template<typename T>
class cached {
  public:
    cached() = default;
    template<typename... Args>
    cached(Args... _args);
    virtual ~cached();

    auto commit() -> cached<T>&;
    auto commit(T const& _new_value) -> cached<T>&;

    auto version() -> unsigned long long;

    auto operator->() -> T*;
    auto operator*() -> T&;

  private:
    T __underlying;
    zpt::lf::spin_lock __repository_lock{};
    zpt::padded_atomic<unsigned long long> __cache_version{ 0 };

    auto instance() -> T&;
};
} // namespace zpt

template<typename T>
template<typename... Args>
zpt::cached<T>::cached(Args... _args)
  : __underlying{ _args... } {}

template<typename T>
zpt::cached<T>::~cached() {}

template<typename T>
auto
zpt::cached<T>::commit() -> zpt::cached<T>& {
    zpt::lf::spin_lock::guard _sentry{ this->__repository_lock, zpt::lf::spin_lock::exclusive };
    this->__underlying = this->instance();
    ++(*this->__cache_version);
    return (*this);
}

template<typename T>
auto
zpt::cached<T>::commit(T const& _new_value) -> zpt::cached<T>& {
    zpt::lf::spin_lock::guard _sentry{ this->__repository_lock, zpt::lf::spin_lock::exclusive };
    this->__underlying = _new_value;
    ++(*this->__cache_version);
    return (*this);
}

template<typename T>
auto
zpt::cached<T>::version() -> unsigned long long {
    return this->__cache_version->load();
}

template<typename T>
auto
zpt::cached<T>::operator->() -> T* {
    return &this->instance();
}

template<typename T>
auto
zpt::cached<T>::operator*() -> T& {
    return this->instance();
}

template<typename T>
auto
zpt::cached<T>::instance() -> T& {
    static thread_local std::map<zpt::cached<T>*, std::tuple<T, unsigned long long>> _local;
    auto& [_local_copy, _local_version] = _local[this];
    if (this->__cache_version->load() > _local_version) {
        if (this->__repository_lock.count_exclusive_acquired_by_thread() == 0) {
            zpt::lf::spin_lock::guard _sentry{ this->__repository_lock,
                                               zpt::lf::spin_lock::shared };
            _local_copy = this->__underlying;
            _local_version = this->__cache_version->load();
        }
        else {
            _local_copy = this->__underlying;
            _local_version = this->__cache_version->load();
        }
    }
    return _local_copy;
}
