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
#include <zapata/locks/spin_lock.h>
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
    static inline zpt::locks::spin_lock __variables_lock;
};
class thread_local_table_entry {
  public:
    virtual ~thread_local_table_entry() = default;
    virtual auto is_null() const -> bool = 0;
};
class thread_local_table {
  public:
    using entry_type = std::unique_ptr<thread_local_table_entry>;
    using map_type = std::map<std::uintptr_t, entry_type>;

    template<typename T>
    class entry : public thread_local_table_entry {
      public:
        template<typename... Args>
        entry(Args... _args);
        virtual ~entry() override;

        auto operator*() -> T&;
        auto operator->() -> T*;
        auto is_null() const -> bool override;

      private:
        T __underlying;
    };

    template<typename P, typename T, typename... Args>
    static auto alloc(P const& _member_variable, Args... _args) -> T&;
    template<typename P, typename T>
    static auto get(P const& _member_variable) -> T&;
    template<typename P, typename T>
    static auto dealloc(P const& _member_variable) -> void;
    static inline auto to_string() -> std::string;

  private:
    static thread_local inline map_type __variables{};
};
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::globals::alloc(ssize_t& _variable, Args... _args) -> T& {
    expect(_variable == -1,
           "variable already assigned with identifier " << _variable << "  for "
                                                        << typeid(T).name());
    zpt::globals::__variables_lock.acquire_exclusive();
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];
    zpt::globals::__variables_lock.release_exclusive();

    T* _new = new T(_args...);
    zpt::globals::__variables_lock.acquire_exclusive();
    _allocated.push_back(static_cast<void*>(_new));
    _variable = _allocated.size() - 1;
    zpt::globals::__variables_lock.release_exclusive();
    return *_new;
}

template<typename T, typename C>
auto
zpt::globals::cast(ssize_t& _variable, std::unique_ptr<C>&& _value) -> T& {
    expect(_variable == -1,
           "variable already assigned with identifier " << _variable << " for "
                                                        << typeid(T).name());
    zpt::globals::__variables_lock.acquire_exclusive();
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];
    zpt::globals::__variables_lock.release_exclusive();

    T* _new = static_cast<T*>(_value.release());
    zpt::globals::__variables_lock.acquire_exclusive();
    _allocated.push_back(static_cast<void*>(_new));
    _variable = _allocated.size() - 1;
    zpt::globals::__variables_lock.release_exclusive();
    return *_new;
}

template<typename T>
auto
zpt::globals::get(ssize_t _variable) -> T& {
    zpt::locks::spin_lock::guard _sentry{ zpt::globals::__variables_lock,
                                          zpt::locks::spin_lock::shared };
    auto _found = zpt::globals::__variables.find(typeid(T).hash_code());
    expect(_found != zpt::globals::__variables.end(),
           "no such global variable for " << typeid(T).name());
    expect(static_cast<ssize_t>(_found->second.size()) > _variable,
           "no such global variable for " << typeid(T).name());

    return *static_cast<T*>(_found->second[_variable]);
}

template<typename T>
auto
zpt::globals::dealloc(ssize_t _variable) -> void {
    zpt::globals::__variables_lock.acquire_exclusive();
    auto _found = zpt::globals::__variables.find(typeid(T).hash_code());
    expect(_found != zpt::globals::__variables.end(),
           "no such global variable for " << typeid(T).name());
    expect(static_cast<ssize_t>(_found->second.size()) > _variable,
           "no such global variable for " << typeid(T).name());
    auto _ptr = static_cast<T*>(_found->second[_variable]);
    _found->second.erase(_found->second.begin() + _variable);
    zpt::globals::__variables_lock.release_exclusive();
    delete _ptr;
}

auto
zpt::globals::to_string() -> std::string {
    zpt::locks::spin_lock::guard _sentry{ zpt::globals::__variables_lock,
                                          zpt::locks::spin_lock::shared };
    std::ostringstream _out;
    _out << "Global variables:" << std::endl;
    for (auto [_key, _value] : zpt::globals::__variables) {
        _out << _key << ":" << std::endl << std::flush;
        for (auto _variable : _value) { _out << "\t- " << _variable << std::endl << std::flush; }
    }
    return _out.str();
}

template<typename T>
template<typename... Args>
zpt::thread_local_table::entry<T>::entry(Args... _args)
  : __underlying{ _args... } {}

template<typename T>
zpt::thread_local_table::entry<T>::~entry() {}

template<typename T>
auto
zpt::thread_local_table::entry<T>::operator*() -> T& {
    return this->__underlying;
}

template<typename T>
auto
zpt::thread_local_table::entry<T>::operator->() -> T* {
    return &this->__underlying;
}

template<typename T>
auto
zpt::thread_local_table::entry<T>::is_null() const -> bool {
    return false;
}

template<typename P, typename T, typename... Args>
auto
zpt::thread_local_table::alloc(P const& _member_variable, Args... _args) -> T& {
    entry<T>* _entry = new entry<T>(_args...);
    zpt::thread_local_table::__variables.emplace(
      reinterpret_cast<std::uintptr_t>(&_member_variable), std::move(entry_type(_entry)));
    return **_entry;
}

template<typename P, typename T>
auto
zpt::thread_local_table::get(P const& _member_variable) -> T& {
    auto _found = zpt::thread_local_table::__variables.find(
      reinterpret_cast<std::uintptr_t>(&_member_variable));
    expect(_found != zpt::thread_local_table::__variables.end(),
           "no such global variable for " << reinterpret_cast<std::uintptr_t>(&_member_variable));
    return *static_cast<entry<T>&>(*_found->second.get());
}

template<typename P, typename T>
auto
zpt::thread_local_table::dealloc(P const& _member_variable) -> void {
    auto _found = zpt::thread_local_table::__variables.find(
      reinterpret_cast<std::uintptr_t>(&_member_variable));
    if (_found == zpt::thread_local_table::__variables.end()) { return; }
    zpt::thread_local_table::__variables.erase(_found);
}

auto
zpt::thread_local_table::to_string() -> std::string {
    std::ostringstream _out;
    _out << "Thread local members:" << std::endl;
    for (auto& [_key, _value] : zpt::thread_local_table::__variables) {
        _out << _key << ": " << _value << std::endl << std::flush;
    }
    return _out.str();
}
