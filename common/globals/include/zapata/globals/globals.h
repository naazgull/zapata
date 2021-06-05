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
class globals {
  public:
    template<typename T, typename... Args>
    static auto alloc(ssize_t& _variable, Args... _args) -> T&;
    template<typename T, typename C = T>
    static auto cast(ssize_t& _variable, std::unique_ptr<C>&& _value) -> T&;
    template<typename T>
    static auto get(ssize_t _variable) -> T&;
    template<typename T>
    static auto dealloc(ssize_t _variable) -> void;
    static inline auto to_string() -> std::string;

  private:
    static inline std::map<size_t, std::vector<void*>> __variables{};
    static inline zpt::lf::spin_lock __variables_lock;
};
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::globals::alloc(ssize_t& _variable, Args... _args) -> T& {
    expect(_variable == -1,
           "variable already assigned with identifier " << _variable << "  for "
                                                        << typeid(T).name(),
           500,
           0);
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock,
                                       zpt::lf::spin_lock::exclusive };
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];

    T* _new = new T(_args...);
    _allocated.push_back(static_cast<void*>(_new));
    _variable = _allocated.size() - 1;
    return *_new;
}

template<typename T, typename C>
auto
zpt::globals::cast(ssize_t& _variable, std::unique_ptr<C>&& _value) -> T& {
    expect(_variable == -1,
           "variable already assigned with identifier " << _variable << " for " << typeid(T).name(),
           500,
           0);
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock,
                                       zpt::lf::spin_lock::exclusive };
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];

    T* _new = static_cast<T*>(_value.release());
    _allocated.push_back(static_cast<void*>(_new));
    _variable = _allocated.size() - 1;
    return *_new;
}

template<typename T>
auto
zpt::globals::get(ssize_t _variable) -> T& {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock, zpt::lf::spin_lock::shared };
    auto _found = zpt::globals::__variables.find(typeid(T).hash_code());
    expect(_found != zpt::globals::__variables.end(),
           "no such global variable for " << typeid(T).name(),
           500,
           0);
    expect(_found->second.size() > _variable,
           "no such global variable for " << typeid(T).name(),
           500,
           0);

    return *static_cast<T*>(_found->second[_variable]);
}

template<typename T>
auto
zpt::globals::dealloc(ssize_t _variable) -> void {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock,
                                       zpt::lf::spin_lock::exclusive };
    auto _found = zpt::globals::__variables.find(typeid(T).hash_code());
    expect(_found != zpt::globals::__variables.end(),
           "no such global variable for " << typeid(T).name(),
           500,
           0);
    expect(_found->second.size() > _variable,
           "no such global variable for " << typeid(T).name(),
           500,
           0);
    auto _ptr = static_cast<T*>(_found->second[_variable]);
    _found->second.erase(_found->second.begin() + _variable);
    delete _ptr;
}

auto
zpt::globals::to_string() -> std::string {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock, zpt::lf::spin_lock::shared };
    std::ostringstream _out;
    _out << "Global variables:" << std::endl;
    for (auto [_key, _value] : zpt::globals::__variables) {
        _out << _key << ":" << std::endl << std::flush;
        for (auto _variable : _value) { _out << "\t- " << _variable << std::endl << std::flush; }
    }
    return _out.str();
}
