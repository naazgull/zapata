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

namespace zpt {
template<typename T>
class ref_ptr {
  public:
    ref_ptr() = default;
    ref_ptr(T& _target);
    ref_ptr(zpt::ref_ptr<T> const& _rhs);
    ref_ptr(zpt::ref_ptr<T>&& _rhs);
    virtual ~ref_ptr() = default;

    auto operator=(zpt::ref_ptr<T> const& _rhs) -> zpt::ref_ptr<T>&;
    auto operator=(zpt::ref_ptr<T>&& _rhs) -> zpt::ref_ptr<T>&;

    auto operator-> () -> T*;
    auto operator*() -> T&;

    auto get() -> T*;
    auto reset() -> T&;

  private:
    T* __underlying{ nullptr };
};
} // namespace zpt

template<typename T>
zpt::ref_ptr<T>::ref_ptr(T& _target)
  : __underlying{ &_target } {}

template<typename T>
zpt::ref_ptr<T>::ref_ptr(zpt::ref_ptr<T> const& _rhs)
  : __underlying{ _rhs.__underlying } {}

template<typename T>
zpt::ref_ptr<T>::ref_ptr(zpt::ref_ptr<T>&& _rhs)
  : __underlying{ _rhs.__underlying } {
    _rhs.reset();
}

template<typename T>
auto
zpt::ref_ptr<T>::operator=(zpt::ref_ptr<T> const& _rhs) -> zpt::ref_ptr<T>& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

template<typename T>
auto
zpt::ref_ptr<T>::operator=(zpt::ref_ptr<T>&& _rhs) -> zpt::ref_ptr<T>& {
    this->__underlying = _rhs.__underlying;
    _rhs.reset();
    return (*this);
}

template<typename T>
auto zpt::ref_ptr<T>::operator-> () -> T* {
    return this->__underlying;
}

template<typename T>
auto zpt::ref_ptr<T>::operator*() -> T& {
    return *this->__underlying;
}

template<typename T>
auto
zpt::ref_ptr<T>::get() -> T* {
    this->__underlying;
}

template<typename T>
auto
zpt::ref_ptr<T>::reset() -> T& {
    auto& _return = this->__underlying;
    this->__underlying = nullptr;
    return _return;
}
