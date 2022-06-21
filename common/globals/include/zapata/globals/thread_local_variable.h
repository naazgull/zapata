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

#include <zapata/globals/globals.h>

namespace zpt {
template<typename T>
class thread_local_variable {
    static_assert(std::is_copy_constructible<T>::value,
                  "Type `T` in `zpt:thread_local_variable<T>` must be copy constuctible.");

  public:
    using type = T;
    using pointer = type*;
    using reference = type&;
    using const_pointer = type const*;
    using const_reference = type const&;

    template<typename... Args>
    thread_local_variable(Args... _args);
    virtual ~thread_local_variable();

    thread_local_variable(thread_local_variable const&) = delete;
    thread_local_variable(thread_local_variable&&) = delete;
    auto operator=(thread_local_variable const&) -> thread_local_variable& = delete;
    auto operator=(thread_local_variable&&) -> thread_local_variable& = delete;

    operator reference();
    operator const_reference() const;
    auto operator*() -> reference;
    auto operator*() const -> const_reference;
    template<typename D = T, std::enable_if_t<std::is_class<D>::value, bool> = true>
    auto operator->() -> pointer;
    template<typename D = T, std::enable_if_t<std::is_class<D>::value, bool> = true>
    auto operator->() const -> const_pointer;
    auto dispose_local_image() -> void;

  private:
    type __initial_value;

    auto get() -> reference;
};
} // namespace zpt

template<typename T>
template<typename... Args>
zpt::thread_local_variable<T>::thread_local_variable(Args... _args)
  : __initial_value{ std::forward<Args>(_args)... } {}

template<typename T>
zpt::thread_local_variable<T>::~thread_local_variable() {}

template<typename T>
zpt::thread_local_variable<T>::operator reference() {
    return this->get();
}

template<typename T>
zpt::thread_local_variable<T>::operator const_reference() const {
    return this->get();
}

template<typename T>
auto
zpt::thread_local_variable<T>::operator*() -> reference {
    return this->get();
}

template<typename T>
auto
zpt::thread_local_variable<T>::operator*() const -> const_reference {
    return this->get();
}

template<typename T>
template<typename D, std::enable_if_t<std::is_class<D>::value, bool>>
auto
zpt::thread_local_variable<T>::operator->() -> pointer {
    return &this->get();
}

template<typename T>
template<typename D, std::enable_if_t<std::is_class<D>::value, bool>>
auto
zpt::thread_local_variable<T>::operator->() const -> const_pointer {
    return &this->get();
}

template<typename T>
auto
zpt::thread_local_variable<T>::dispose_local_image() -> void {
    zpt::thread_local_table::dealloc<zpt::thread_local_variable<T>, T>(*this);
}

template<typename T>
auto
zpt::thread_local_variable<T>::get() -> reference {
    try {
        return zpt::thread_local_table::get<zpt::thread_local_variable<T>, T>(*this);
    }
    catch (...) {
        return zpt::thread_local_table::alloc<zpt::thread_local_variable<T>, T>(
          *this, this->__initial_value);
    }
}
