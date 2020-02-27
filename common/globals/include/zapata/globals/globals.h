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
#include <zapata/lockfree.h>
#include <typeinfo>

namespace zpt {
template<typename T>
auto
copy(T const& _from, T& _to) -> void;

class globals {
  public:
    template<typename T, typename... Args>
    static auto alloc(size_t& _variable, Args... _args) -> T&;
    template<typename T>
    static auto get(size_t _variable) -> T&;
    template<typename T, typename... Args>
    static auto dealloc(size_t _variable) -> void;

    template<typename T>
    class cached {
      public:
        template<typename... Args>
        cached(Args... _args);
        virtual ~cached();

        auto invalidate() -> cached<T>&;
        auto invalidate(T const& _new_value) -> cached<T>&;

        auto operator-> () -> T*;
        auto operator*() -> T&;

      private:
        T __underlying;
        zpt::lf::spin_lock __cache_guard{};
        std::map<std::atomic<bool>*, bool> __cache_validation{};
        zpt::lf::spin_lock __cache_validation_guard{};

        auto instance() -> T&;

        class thread_exit_guard {
          public:
            friend class zpt::globals::cached<T>;

            thread_exit_guard(zpt::globals::cached<T>& _parent);
            virtual ~thread_exit_guard();

            zpt::globals::cached<T>& __parent;
            std::atomic<bool> __cache_invalid{ true };
        };
        friend class zpt::globals::cached<T>::thread_exit_guard;
    };

  private:
    static inline std::map<size_t, std::vector<void*>> __variables{};
    static inline zpt::lf::spin_lock __variables_lock;
};
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::globals::alloc(size_t& _variable, Args... _args) -> T& {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock,
                                       zpt::lf::spin_lock::exclusive };
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];

    T* _new = new T(_args...);
    _allocated.push_back(static_cast<void*>(_new));
    _variable = _allocated.size() - 1;
    return *_new;
}

template<typename T>
auto
zpt::globals::get(size_t _variable) -> T& {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock, zpt::lf::spin_lock::shared };
    auto _found = zpt::globals::__variables.find(typeid(T).hash_code());
    expect(_found != zpt::globals::__variables.end() && _found->second.size() > _variable,
           "no such global variable",
           500,
           0);

    return *static_cast<T*>(_found->second[_variable]);
}

template<typename T, typename... Args>
auto
zpt::globals::dealloc(size_t _variable) -> void {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock,
                                       zpt::lf::spin_lock::exclusive };
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];
    _allocated.erase(_allocated.begin() + _variable);
}

template<typename T>
template<typename... Args>
zpt::globals::cached<T>::cached(Args... _args)
  : __underlying{ _args... } {}

template<typename T>
zpt::globals::cached<T>::~cached() {}

template<typename T>
auto
zpt::globals::cached<T>::invalidate() -> zpt::globals::cached<T>& {
    return this->invalidate(this->instance());
}

template<typename T>
auto
zpt::globals::cached<T>::invalidate(T const& _new_value) -> zpt::globals::cached<T>& {
    {
        zpt::lf::spin_lock::guard _sentry{ this->__cache_guard, zpt::lf::spin_lock::exclusive };
        zpt::copy(_new_value, this->__underlying);
    }
    {
        zpt::lf::spin_lock::guard _sentry{ this->__cache_validation_guard,
                                           zpt::lf::spin_lock::shared };
        for (auto [_atomic, _] : this->__cache_validation) {
            _atomic->store(true);
        }
    }
    return (*this);
}

template<typename T>
auto zpt::globals::cached<T>::operator-> () -> T* {
    return &this->instance();
}

template<typename T>
auto zpt::globals::cached<T>::operator*() -> T& {
    return this->instance();
}

template<typename T>
auto
zpt::globals::cached<T>::instance() -> T& {
    static thread_local zpt::globals::cached<T>::thread_exit_guard _invalidate_cache{ *this };
    static thread_local T _local_copy;
    if (_invalidate_cache.__cache_invalid.exchange(false)) {
        zpt::lf::spin_lock::guard _sentry{ this->__cache_guard, zpt::lf::spin_lock::shared };
        zpt::copy(this->__underlying, _local_copy);
    }
    return _local_copy;
}

template<typename T>
zpt::globals::cached<T>::thread_exit_guard::thread_exit_guard(zpt::globals::cached<T>& _parent)
  : __parent{ _parent } {
    zpt::lf::spin_lock::guard _sentry{ this->__parent.__cache_validation_guard,
                                       zpt::lf::spin_lock::exclusive };
    if (!this->__parent.__cache_validation[&this->__cache_invalid]) {
        this->__parent.__cache_validation[&this->__cache_invalid] = true;
    }
}

template<typename T>
zpt::globals::cached<T>::thread_exit_guard::~thread_exit_guard() {
    zpt::lf::spin_lock::guard _sentry{ this->__parent.__cache_validation_guard,
                                       zpt::lf::spin_lock::exclusive };
    this->__parent.__cache_validation.erase(&this->__cache_invalid);
}
