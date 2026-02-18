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

/**
 * @file hazard_ptr.h
 * @brief Hazard pointer implementation for safe memory reclamation in lock-free structures.
 *
 * Hazard pointers provide a mechanism for safe memory reclamation in lock-free
 * data structures. They solve the ABA problem and prevent use-after-free bugs
 * by tracking which memory locations are currently being accessed by threads.
 *
 * @par How It Works
 * 1. Before accessing a shared pointer, a thread "acquires" it by publishing
 *    the pointer value in a hazard pointer slot
 * 2. When done, the thread "releases" the hazard pointer slot
 * 3. When a node is removed, it's placed in a "retired" list
 * 4. Periodically, retired nodes are scanned and deleted only if no hazard
 *    pointer references them
 *
 * @par Thread Requirements
 * - Each thread must call `clear_thread_context()` before exiting
 * - Maximum threads must be specified at construction time
 * - Each thread gets K hazard pointer slots (default: 2)
 *
 * @see zpt::lf::queue
 */

#pragma once

#include <zapata/base/expect.h>
#include <zapata/globals/thread_local_variable.h>
#include <zapata/log/log.h>

namespace zpt {
/**
 * @brief Lock-free data structures namespace.
 */
namespace lf {

/**
 * @brief Hazard pointer domain for safe memory reclamation.
 *
 * Manages a pool of hazard pointer slots that threads use to protect
 * memory locations from being freed while still in use. This is essential
 * for implementing lock-free data structures safely.
 *
 * @tparam T Type of pointers being protected.
 *
 * @par Configuration Parameters
 * - P: Maximum number of threads (set at construction)
 * - K: Hazard pointers per thread (default: 2)
 * - N: Total hazard pointers (P * K)
 * - R: Reclamation threshold (N * 2)
 *
 * @par Example
 * @code
 * zpt::lf::hazard_ptr<MyNode> hp(16);  // Support up to 16 threads
 *
 * // Protect a pointer during access
 * MyNode* node = shared_ptr.load();
 * long slot = hp.acquire(node);
 * // ... use node safely ...
 * hp.release(slot);
 *
 * // Or use RAII guard
 * {
 *     zpt::lf::hazard_ptr<MyNode>::guard guard(node, hp);
 *     // ... use guard.target() safely ...
 *     guard.retire();  // Mark for deletion when guard destructs
 * }
 * @endcode
 */
template<typename T>
class hazard_ptr {
  public:
    using size_type = size_t;
    /** @brief Type for hazard pointer storage. */
    using hp_type = zpt::padded_atomic<T*>;
    /** @brief Type for thread slot availability tracking. */
    using thr_slot_type = zpt::padded_atomic<bool>;
    /** @brief Type for retired pointer pending list. */
    using pending_list = std::map<T*, T*>;

    /**
     * @brief RAII guard for hazard pointer acquisition/release.
     *
     * Automatically acquires a hazard pointer slot on construction and
     * releases it on destruction. Optionally retires the protected pointer.
     */
    class guard {
      public:
        friend class zpt::lf::hazard_ptr<T>;

        /**
         * @brief Acquires a hazard pointer for the target.
         * @param _target Pointer to protect.
         * @param _parent Hazard pointer domain.
         */
        guard(T* _target, zpt::lf::hazard_ptr<T>& _parent);
        /** @brief Releases the hazard pointer, optionally retiring the target. */
        virtual ~guard();

        /** @brief Marks the protected pointer for retirement on destruction. */
        auto retire() -> guard&;
        /** @brief Returns the protected pointer. */
        auto target() const -> T*;

      private:
        T* __target{ nullptr };
        long __target_idx{ -1 };
        zpt::lf::hazard_ptr<T>& __parent;
        bool __retire{ false };
    };

    /**
     * @brief Constructs a hazard pointer domain.
     * @param _max_threads Maximum number of concurrent threads.
     * @param _ptr_per_thread Hazard pointer slots per thread (min: 2).
     */
    hazard_ptr(long _max_threads, long _ptr_per_thread = 2);
    hazard_ptr(const hazard_ptr<T>& _rhs) = delete;
    hazard_ptr(hazard_ptr<T>&& _rhs) = delete;
    virtual ~hazard_ptr();

    auto operator=(const hazard_ptr<T>& _rhs) -> hazard_ptr<T>& = delete;
    auto operator=(hazard_ptr<T>&& _rhs) -> hazard_ptr<T>& = delete;

    /** @brief Access hazard pointer slot by index. */
    auto operator[](size_t _idx) -> hp_type&;
    /** @brief Access hazard pointer slot by index. */
    auto at(size_t _idx) -> hp_type&;

    /**
     * @brief Acquires a hazard pointer slot for a pointer.
     * @param _ptr Pointer to protect.
     * @return Slot index (use with release()).
     * @throws zpt::ExpectationException If no slots available.
     */
    auto acquire(T* _ptr) -> long;
    /**
     * @brief Releases a hazard pointer slot.
     * @param _idx Slot index from acquire().
     */
    auto release(long _idx) -> hazard_ptr<T>&;
    /**
     * @brief Marks a pointer for deferred deletion.
     * @param _ptr Pointer to retire.
     */
    auto retire(T* _ptr) -> hazard_ptr<T>&;
    /** @brief Scans retired list and deletes unreferenced pointers. */
    auto clean() -> hazard_ptr<T>&;
    /** @brief Cleans up thread-local state (call before thread exit). */
    auto clear_thread_context() -> hazard_ptr<T>&;

    /** @brief Returns count of retired pointers not yet deleted. */
    auto get_thread_dangling_count() -> size_t;
    /** @brief Returns count of hazard pointers held by this thread. */
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
    auto clean_this_thread_retired() -> zpt::lf::hazard_ptr<T>&;
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
auto zpt::lf::hazard_ptr<T>::guard::retire() -> guard& {
    this->__retire = true;
    return (*this);
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::guard::target() const -> T* {
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
    expect(_ptr_per_thread > 0, "`_ptr_per_thread` expected to be higher than 0");
    this->init();
}

template<typename T>
zpt::lf::hazard_ptr<T>::~hazard_ptr() {
    delete[] this->__hp;
    delete[] this->__next_thr_slot;
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::operator[](size_t _idx) -> hp_type& {
    return this->__hp[_idx];
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::at(size_t _idx) -> hp_type& {
    return this->__hp[_idx];
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::acquire(T* _ptr) -> long {
    auto _idx = this->get_this_thread_slot();

    for (auto _k = _idx * this->K; _k != ((_idx + 1) * this->K); ++_k) {
        T* _null{ nullptr };
        if (this->__hp[_k]->compare_exchange_strong(_null, _ptr, std::memory_order_release)) {
            return _k;
        }
    }

    expect(
      false,
      "No more hazard-pointer slots available for this thread, release some before continuing.");
    return -1;
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::release(long _idx) -> zpt::lf::hazard_ptr<T>& {
    if (_idx == -1) { return (*this); }
    this->__hp[_idx] = nullptr;
    return (*this);
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::retire(T* _ptr) -> zpt::lf::hazard_ptr<T>& {
    auto& _retired = this->get_retired();
    _retired.insert(std::make_pair(_ptr, _ptr));
    if (_retired.size() == static_cast<size_t>(R)) { this->clean(); }
    return (*this);
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::clean() -> zpt::lf::hazard_ptr<T>& {
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
auto zpt::lf::hazard_ptr<T>::clear_thread_context() -> zpt::lf::hazard_ptr<T>& {
    this //
      ->release_this_thread_slot()
      .clean_this_thread_retired();
    return (*this);
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::get_thread_dangling_count() -> size_t {
    return this->get_retired().size();
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::get_thread_held_count() -> size_t {
    auto _idx = this->get_this_thread_slot();
    auto _held{ 0 };
    for (auto _k = _idx * this->K; _k != ((_idx + 1) * this->K); ++_k) {
        if (this->__hp[_k] != nullptr) { ++_held; }
    }
    return _held;
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::release_this_thread_slot() -> zpt::lf::hazard_ptr<T>& {
    auto& _idx = *this->__thread_idx;
    if (_idx == -1) { return (*this); }

    for (auto _k = _idx * this->K; _k != ((_idx + 1) * this->K); ++_k) { this->__hp[_k] = nullptr; }
    this->__next_thr_slot[_idx] = false;

    return (*this);
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::clean_this_thread_retired() -> zpt::lf::hazard_ptr<T>& {
    auto& _retired = this->get_retired();
    for (auto [key, _] : _retired) { delete key; }
    return (*this);
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::get_retired() -> zpt::lf::hazard_ptr<T>::pending_list& {
    return *this->__thread_retired;
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::get_next_available_thread_slot() -> int {
    auto _idx{ 0 };
    for (; _idx != this->P; ++_idx) {
        auto _acquired{ false };
        if (this->__next_thr_slot[_idx]->compare_exchange_strong(_acquired, true)) { return _idx; }
    }
    expect(_idx != this->P,
           "No more thread space available for " << std::this_thread::get_id() << " in domain "
                                                 << typeid(T).name() << " and instance " << std::hex
                                                 << this);
    return -1;
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::get_this_thread_slot() -> int {
    auto& _idx = *this->__thread_idx;
    if (_idx == -1) { _idx = this->get_next_available_thread_slot(); }
    return _idx;
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::is_thread_slot_taken(size_t _slot) -> bool {
    return this->__next_thr_slot[_slot];
}

template<typename T>
auto zpt::lf::hazard_ptr<T>::init() -> zpt::lf::hazard_ptr<T>& {
    for (auto _idx = 0; _idx != this->N; ++_idx) { this->__hp[_idx] = nullptr; }
    for (auto _idx = 0; _idx != this->P; ++_idx) { this->__next_thr_slot[_idx] = false; }
    return (*this);
}
} // namespace lf
} // namespace zpt
