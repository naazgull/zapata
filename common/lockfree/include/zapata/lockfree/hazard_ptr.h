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

#include <zapata/base/expect.h>
#include <zapata/log/log.h>
#include <zapata/globals/thread_local_variable.h>

namespace zpt {
namespace lf {

template<typename T>
class hazard_ptr {
  public:
    using size_type = size_t;
    using hp_type = zpt::padded_atomic<T*>;
    using thr_slot_type = zpt::padded_atomic<bool>;
    using pending_list = std::map<T*, T*>;

    class guard {
      public:
        friend class zpt::lf::hazard_ptr<T>;

        guard(T* _target, zpt::lf::hazard_ptr<T>& _parent);
        virtual ~guard();

        auto retire() -> guard&;
        auto target() const -> T*;

      private:
        T* __target{ nullptr };
        long __target_idx{ -1 };
        zpt::lf::hazard_ptr<T>& __parent;
        bool __retire{ false };
    };

    hazard_ptr(long _max_threads, long _ptr_per_thread = 2);
    hazard_ptr(const hazard_ptr<T>& _rhs) = delete;
    hazard_ptr(hazard_ptr<T>&& _rhs) = delete;
    virtual ~hazard_ptr();

    auto operator=(const hazard_ptr<T>& _rhs) -> hazard_ptr<T>& = delete;
    auto operator=(hazard_ptr<T>&& _rhs) -> hazard_ptr<T>& = delete;

    auto operator[](size_t _idx) -> hp_type&;
    auto at(size_t _idx) -> hp_type&;

    auto acquire(T* _ptr) -> long;
    auto release(long _idx) -> hazard_ptr<T>&;
    auto retire(T* _ptr) -> hazard_ptr<T>&;
    auto clean() -> hazard_ptr<T>&;
    auto clear() -> hazard_ptr<T>&;
    auto clear_thread_context() -> hazard_ptr<T>&;

    auto get_thread_dangling_count() -> size_t;
    auto get_thread_held_count() -> size_t;

    friend auto operator<<(std::ostream& _out, zpt::lf::hazard_ptr<T>& _in) -> std::ostream& {
        _out << "hazard_ptr(" << std::hex << &_in << std::dec << ") -> P = " << _in.P
             << " | K = " << _in.K << " | N = " << _in.N << " | R = " << _in.R << std::dec
             << std::flush;
        return _out;
    }

    friend class guard;

  private:
    long P{ 0 };
    long K{ 0 };
    long N{ 0 };
    long R{ 0 };
    hp_type* __hp{ nullptr };
    thr_slot_type* __next_thr_slot{ nullptr };
    zpt::thread_local_variable<pending_list> __thread_retired;
    zpt::thread_local_variable<int> __thread_idx{ -1 };

    auto init() -> zpt::lf::hazard_ptr<T>&;
    auto get_retired() -> pending_list&;
    auto get_this_thread_slot() -> int;
    auto get_next_available_thread_slot() -> int;
    auto release_this_thread_slot() -> zpt::lf::hazard_ptr<T>&;
    auto is_thread_slot_taken(size_t _slot) -> bool;
};

template<typename T>
zpt::lf::hazard_ptr<T>::guard::guard(T* _ptr, zpt::lf::hazard_ptr<T>& _parent)
  : __target{ _ptr }
  , __parent{ _parent } {
    this->__target_idx = this->__parent.acquire(this->__target);
}

template<typename T>
zpt::lf::hazard_ptr<T>::guard::~guard() {
    this->__parent.release(this->__target_idx);
    if (this->__retire) this->__parent.retire(this->__target);
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
  , K{ std::max(2L, _ptr_per_thread) }
  , N{ P * K }
  , R{ N * 2 }
  , __hp{ new hp_type[N] }
  , __next_thr_slot{ new thr_slot_type[P] } {
    expect(_ptr_per_thread > 0, "`_ptr_per_thread` expected to be higher than 0", 500, 0);
    this->init();
}

template<typename T>
zpt::lf::hazard_ptr<T>::~hazard_ptr() {
    this->clear();
    delete[] this->__hp;
    delete[] this->__next_thr_slot;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::operator[](size_t _idx) -> hp_type& {
    return this->__hp[_idx];
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::at(size_t _idx) -> hp_type& {
    return this->__hp[_idx];
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::acquire(T* _ptr) -> long {
    auto _idx = this->get_this_thread_slot();

    for (auto _k = _idx * this->K; _k != ((_idx + 1) * this->K); ++_k) {
        T* _null{ nullptr };
        if (this->__hp[_k]->compare_exchange_strong(_null, _ptr, std::memory_order_release)) {
            return _k;
        }
    }

    expect(
      false,
      "No more hazard-pointer slots available for this thread, release some before continuing.",
      500,
      0);
    return -1;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::release(long _idx) -> zpt::lf::hazard_ptr<T>& {
    if (_idx == -1) { return (*this); }
    this->__hp[_idx] = nullptr;
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::retire(T* _ptr) -> zpt::lf::hazard_ptr<T>& {
    auto& _retired = this->get_retired();
    _retired.insert(std::make_pair(_ptr, _ptr));
    if (_retired.size() == static_cast<size_t>(R)) { this->clean(); }
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::clean() -> zpt::lf::hazard_ptr<T>& {
    auto& _retired = this->get_retired();

    std::map<T*, bool> _to_process;
    for (long _slot = 0; _slot != this->P; ++_slot) {
        if (this->is_thread_slot_taken(_slot)) {
            for (auto _idx = _slot * this->K; _idx != ((_slot + 1) * this->K); ++_idx) {
                T* _ptr = this->__hp[_idx]->load();
                if (_ptr != nullptr) { _to_process.insert(std::make_pair(_ptr, true)); }
            }
        }
    }

    for (auto _it = _retired.begin(); _it != _retired.end();) {
        if (_to_process.find(_it->first) == _to_process.end()) {
            delete _it->first;
            _it = _retired.erase(_it);
        }
        else { ++_it; }
    }

    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::clear() -> zpt::lf::hazard_ptr<T>& {
    std::map<T*, bool> _to_process;
    for (auto _idx = 0; _idx != this->N; ++_idx) {
        T* _ptr = this->__hp[_idx]->load();
        if (_ptr != nullptr) {
            this->__hp[_idx] = nullptr;
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
zpt::lf::hazard_ptr<T>::clear_thread_context() -> zpt::lf::hazard_ptr<T>& {
    this->release_this_thread_slot();
    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_thread_dangling_count() -> size_t {
    return this->get_retired().size();
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_thread_held_count() -> size_t {
    auto _idx = this->get_this_thread_slot();
    auto _held{ 0 };
    for (auto _k = _idx * this->K; _k != ((_idx + 1) * this->K); ++_k) {
        if (this->__hp[_k] != nullptr) { ++_held; }
    }
    return _held;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::release_this_thread_slot() -> zpt::lf::hazard_ptr<T>& {
    auto& _idx = *this->__thread_idx;
    if (_idx == -1) { return (*this); }

    for (auto _k = _idx * this->K; _k != ((_idx + 1) * this->K); ++_k) { this->__hp[_k] = nullptr; }
    this->__next_thr_slot[_idx] = false;

    return (*this);
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_retired() -> zpt::lf::hazard_ptr<T>::pending_list& {
    return *this->__thread_retired;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_next_available_thread_slot() -> int {
    auto _idx{ 0 };
    for (; _idx != this->P; ++_idx) {
        auto _acquired{ false };
        if (this->__next_thr_slot[_idx]->compare_exchange_strong(_acquired, true)) { return _idx; }
    }
    expect(_idx != this->P,
           "No more thread space available for " << std::this_thread::get_id() << " in domain "
                                                 << typeid(T).name() << " and instance " << std::hex
                                                 << this,
           500,
           0);
    return -1;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::get_this_thread_slot() -> int {
    auto& _idx = *this->__thread_idx;
    if (_idx == -1) { _idx = this->get_next_available_thread_slot(); }
    return _idx;
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::is_thread_slot_taken(size_t _slot) -> bool {
    return this->__next_thr_slot[_slot];
}

template<typename T>
auto
zpt::lf::hazard_ptr<T>::init() -> zpt::lf::hazard_ptr<T>& {
    for (auto _idx = 0; _idx != this->N; ++_idx) { this->__hp[_idx] = nullptr; }
    for (auto _idx = 0; _idx != this->P; ++_idx) { this->__next_thr_slot[_idx] = false; }
    return (*this);
}
} // namespace lf
} // namespace zpt
