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
#include <iostream>
#include <iterator>
#include <map>
#include <math.h>
#include <memory>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <zapata/base/expect.h>
#include <zapata/log/log.h>

namespace zpt {
namespace lf {

template<typename T>
class hazard_ptr {
  public:
    using size_type = size_t;
    static constexpr size_t element_size = sizeof(std::atomic<T*>);
    static inline std::atomic_flag __initialized = ATOMIC_FLAG_INIT;

    class pending_list {
      public:
        friend class zpt::lf::hazard_ptr<T>;

        pending_list(zpt::lf::hazard_ptr<T>& _parent);
        virtual ~pending_list();

        auto operator->() -> std::map<T*, T*>*;
        auto operator*() -> std::map<T*, T*>&;

      private:
        std::map<T*, T*> __underlying;
        zpt::lf::hazard_ptr<T>& __parent;
    };

    class guard {
      public:
        friend class zpt::lf::hazard_ptr<T>;

        guard(T* _target, zpt::lf::hazard_ptr<T>& _parent);
        guard(std::atomic<T*>& _target, zpt::lf::hazard_ptr<T>& _parent);
        virtual ~guard();

        auto is_acquired() -> bool;
        auto retire() -> guard&;
        auto target() const -> T*;

      private:
        T* __target{ nullptr };
        zpt::lf::hazard_ptr<T>& __parent;
        bool __retire{ false };
    };

    hazard_ptr(long _max_threads, long _ptr_per_thread);
    hazard_ptr(const hazard_ptr<T>& _rhs) = delete;
    hazard_ptr(hazard_ptr<T>&& _rhs) = delete;
    virtual ~hazard_ptr();

    auto operator=(const hazard_ptr<T>& _rhs) -> hazard_ptr<T>& = delete;
    auto operator=(hazard_ptr<T>&& _rhs) -> hazard_ptr<T>& = delete;

    auto operator[](size_t _idx) -> zpt::padded_atomic<T*>&;
    auto at(size_t _idx) -> zpt::padded_atomic<T*>&;

    auto acquire(T* _ptr) -> T*;
    auto acquire(std::atomic<T*>& _ptr) -> T*;
    auto release(T* _ptr) -> hazard_ptr<T>&;
    auto retire(T* _ptr) -> hazard_ptr<T>&;
    auto clean() -> hazard_ptr<T>&;
    auto clear() -> hazard_ptr<T>&;

    auto get_thread_dangling_count() -> size_t;
    auto get_thread_held_count() -> size_t;
    auto get_retired() -> pending_list&;
    auto get_this_thread_idx() -> int;
    auto get_next_available_idx() -> int;
    auto release_thread_idx() -> zpt::lf::hazard_ptr<T>&;

    friend auto operator<<(std::ostream& _out, zpt::lf::hazard_ptr<T>& _in) -> std::ostream& {
        _out << "zpt::lf::hazard_ptr(" << std::hex << &_in << ") for `" << typeid(T).name() << "` "
             << std::dec << " -> P = " << _in.P << " | K = " << _in.K << " | N = " << _in.N
             << " | R = " << _in.R << std::dec << std::flush;
        return _out;
    }

    friend class guard;

  private:
    std::unique_ptr<zpt::padded_atomic<T*>[]> __hp { nullptr };
    std::unique_ptr<zpt::padded_atomic<bool>[]> __next_thr_idx { nullptr };
    long P{ 0 };
    long K{ 0 };
    long N{ 0 };
    long R{ 0 };

    auto init() -> zpt::lf::hazard_ptr<T>&;
};

template<typename T>
zpt::lf::hazard_ptr<T>::pending_list::pending_list(zpt::lf::hazard_ptr<T>& _parent)
  : __parent{ _parent } {}

template<typename T>
zpt::lf::hazard_ptr<T>::pending_list::~pending_list() {
    this->__parent.clean();
    this->__parent.release_thread_idx();
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::pending_list::operator->() -> std::map<T*, T*>* {
    return &this->__underlying;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::pending_list::operator*() -> std::map<T*, T*>& {
    return this->__underlying;
}

template<typename T>
zpt::lf::hazard_ptr<T>::guard::guard(T* _ptr, zpt::lf::hazard_ptr<T>& _parent)
  : __target{ _ptr }
  , __parent{ _parent } {
    this->__parent.acquire(this->__target);
}

template<typename T>
zpt::lf::hazard_ptr<T>::guard::guard(std::atomic<T*>& _atomic, zpt::lf::hazard_ptr<T>& _parent)
  : __parent{ _parent } {
    this->__target = this->__parent.acquire(_atomic);
}

template<typename T>
zpt::lf::hazard_ptr<T>::guard::~guard() {
    this->__parent.release(this->__target);
    if (this->__retire) this->__parent.retire(this->__target);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::guard::is_acquired() -> bool {
    return this->__target != nullptr;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::guard::retire() -> guard& {
    this->__retire = true;
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::guard::target() const -> T* {
    return this->__target;
}

template<typename T>
zpt::lf::hazard_ptr<T>::hazard_ptr(long _max_threads, long _ptr_per_thread)
  : P{ _max_threads }
  , K{ _ptr_per_thread }
  , N{ P * K }
  , R{ N } {
    expect(!zpt::lf::hazard_ptr<T>::__initialized.test_and_set(),
           "Hazard domain for type `" << typeid(T).name() << "` already initialized",
           500,
           0);
    expect(_max_threads > 0,
           "Hazard pointer domain for the given template type has not been initialized",
           500,
           0);
    expect(_ptr_per_thread > 0, "`_ptr_per_thread` expected to be higher than 0", 500, 0);
    this->__hp = std::make_unique<zpt::padded_atomic<T*>[]>(this->N);
    this->__next_thr_idx = std::make_unique<zpt::padded_atomic<bool>[]>(this->P);
    this->init();
}

template<typename T>
zpt::lf::hazard_ptr<T>::~hazard_ptr() {
    this->clear();
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::operator[](size_t _idx) -> zpt::padded_atomic<T*>& {
    return this->__hp[_idx];
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::at(size_t _idx) -> zpt::padded_atomic<T*>& {
    return this->__hp[_idx];
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::acquire(T* _ptr) -> T* {
    size_t _idx = this->get_this_thread_idx();

    for (size_t _k = _idx * K; _k != ((_idx + 1) * K); ++_k) {
        auto& _current = this->__hp[_k];
        if (_current->load(std::memory_order_acquire) == nullptr) {
            T* _exchange = _ptr;
            T* _null{ nullptr };
            if (_current->compare_exchange_strong(_null, _exchange, std::memory_order_release)) {
                return _ptr;
            }
        }
    }

    expect(
      false,
      "No more hazard-pointer slots available for this thread, release some before continuing.",
      500,
      0);
    return nullptr;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::acquire(std::atomic<T*>& _atomic) -> T* {
    auto _ptr = _atomic.load();
    this->acquire(_ptr);
    if (_ptr != _atomic.load(std::memory_order_acquire)) {
        this->release(_ptr);
        return nullptr;
    }
    return _ptr;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::release(T* _ptr) -> zpt::lf::hazard_ptr<T>& {
    if (_ptr == nullptr) { return (*this); }

    size_t _idx = this->get_this_thread_idx();
    for (size_t _k = _idx * K; _k != ((_idx + 1) * K); ++_k) {
        T* _exchange = _ptr;
        if (this->__hp[_k]->compare_exchange_strong(_exchange, nullptr)) { return (*this); }
    }
    expect(false, "Pointer not found in this thread's list.", 500, 0);
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::retire(T* _ptr) -> zpt::lf::hazard_ptr<T>& {
    auto& _retired = zpt::lf::hazard_ptr<T>::get_retired();
    _retired->insert(std::make_pair(_ptr, _ptr));
    if (_retired->size() == static_cast<size_t>(R)) { this->clean(); }
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::clean() -> zpt::lf::hazard_ptr<T>& {
    auto& _retired = zpt::lf::hazard_ptr<T>::get_retired();

    std::map<T*, bool> _to_process;
    for (long _idx = 0; _idx != this->N; ++_idx) {
        auto& _item = this->__hp[_idx];
        T* _ptr = _item->load();
        if (_ptr != nullptr) { _to_process.insert(std::make_pair(_ptr, true)); }
    }

    for (auto _it = _retired->begin(); _it != _retired->end();) {
        if (_to_process.find(_it->first) == _to_process.end()) {
            delete _it->first;
            _it = _retired->erase(_it);
        }
        else {
            ++_it;
        }
    }

    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::clear() -> zpt::lf::hazard_ptr<T>& {
    std::map<T*, bool> _to_process;
    for (long _idx = 0; _idx != this->N; ++_idx) {
        auto& _item = this->__hp[_idx];
        T* _ptr = _item->load();
        if (_ptr != nullptr) {
            _item->store(nullptr);
            _to_process.insert(std::make_pair(_ptr, true));
        }
    }
    std::for_each(_to_process.begin(),
                  _to_process.end(),
                  [&](const std::pair<T*, bool>& _item) -> void { delete _item.first; });
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_thread_dangling_count() -> size_t {
    auto& _retired = zpt::lf::hazard_ptr<T>::get_retired();
    return _retired->size();
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_thread_held_count() -> size_t {
    int _idx = this->get_this_thread_idx();
    size_t _held{ 0 };
    for (long _k = _idx * K; _k != ((_idx + 1) * K); ++_k) {
        if (this->__hp[_k]->load() != nullptr) { ++_held; }
    }
    return _held;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::release_thread_idx() -> zpt::lf::hazard_ptr<T>& {
    int _idx = this->get_this_thread_idx();

    for (long _k = _idx * K; _k != ((_idx + 1) * K); ++_k) { this->__hp[_k]->store(nullptr); }
    this->__next_thr_idx[_idx] = false;

    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::init() -> zpt::lf::hazard_ptr<T>& {
    for (long _idx = 0; _idx != this->N; ++_idx) { this->__hp[_idx]->store(nullptr); }
    for (long _idx = 0; _idx != this->P; ++_idx) { this->__next_thr_idx[_idx]->store(false); }
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_retired() -> zpt::lf::hazard_ptr<T>::pending_list& {
    thread_local static pending_list _retired{ *this };
    return _retired;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_next_available_idx() -> int {
    long _idx{ 0 };
    for (; _idx != this->P; ++_idx) {
        bool _acquired{ false };
        if (this->__next_thr_idx[_idx]->compare_exchange_strong(_acquired, true)) { return _idx; }
    }
    expect(_idx == this->P,
           "No more thread space available for " << std::this_thread::get_id() << " in domain "
                                                 << typeid(T).name() << " and instance " << std::hex
                                                 << this,
           500,
           0);
    return -1;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_this_thread_idx() -> int {
    static thread_local int _idx{ -1 };
    if (_idx == -1) { _idx = this->get_next_available_idx(); }
    return _idx;
}
} // namespace lf
} // namespace zpt
