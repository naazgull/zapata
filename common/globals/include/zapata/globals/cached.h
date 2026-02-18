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
 * @file cached.h
 * @brief Thread-safe cached value with copy-on-read semantics.
 *
 * Provides a value wrapper where writes are protected by a spin mutex and
 * reads use thread-local copies that are refreshed when the version changes.
 *
 * @see zpt::locks::spin_mutex
 * @see zpt::thread_local_variable
 */

#pragma once

#include <typeinfo>
#include <zapata/base.h>

namespace zpt {

/**
 * @brief Thread-safe cached value with per-thread read copies.
 *
 * Stores a shared value that can be updated atomically via `commit()`.
 * Each reader thread maintains a local copy that is refreshed only when
 * the version number changes, minimizing lock contention for read-heavy
 * workloads.
 *
 * @tparam T Value type (must be copy-assignable).
 *
 * @par Example Usage
 * @code
 * zpt::cached<zpt::json> config;
 * config.commit(zpt::json{ "key", "value" });  // Writer thread
 *
 * auto& local = *config;  // Reader thread gets local copy
 * @endcode
 *
 * @par Thread Safety
 * - `commit()`: Thread-safe (exclusive lock)
 * - `operator*()`, `operator->()`: Thread-safe (shared lock on version check)
 */
template<typename T>
class cached {
  public:
    /** @brief Default constructor. */
    cached() = default;
    /** @brief Constructs with an initial value. */
    template<typename... Args>
    cached(Args... _args);
    /** @brief Destructor. */
    virtual ~cached();

    /**
     * @brief Commits the internal instance as the new shared value.
     * @return Reference to this cached.
     */
    auto commit() -> cached<T>&;

    /**
     * @brief Commits a new value as the shared value.
     * @param _new_value Value to set.
     * @return Reference to this cached.
     */
    auto commit(T const& _new_value) -> cached<T>&;

    /** @brief Returns the current version number. */
    auto version() -> unsigned long long;

    /** @brief Access the thread-local copy. */
    auto operator->() -> T*;
    /** @brief Dereference to the thread-local copy. */
    auto operator*() -> T&;

  private:
    T __underlying;
    zpt::locks::spin_mutex __repository_lock{};
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
auto zpt::cached<T>::commit() -> zpt::cached<T>& {
    std::unique_lock _sentry{ this->__repository_lock };
    this->__underlying = this->instance();
    ++(*this->__cache_version);
    return (*this);
}

template<typename T>
auto zpt::cached<T>::commit(T const& _new_value) -> zpt::cached<T>& {
    std::unique_lock _sentry{ this->__repository_lock };
    this->__underlying = _new_value;
    ++(*this->__cache_version);
    return (*this);
}

template<typename T>
auto zpt::cached<T>::version() -> unsigned long long {
    return this->__cache_version->load();
}

template<typename T>
auto zpt::cached<T>::operator->() -> T* {
    return &this->instance();
}

template<typename T>
auto zpt::cached<T>::operator*() -> T& {
    return this->instance();
}

template<typename T>
auto zpt::cached<T>::instance() -> T& {
    static thread_local std::map<zpt::cached<T>*, std::tuple<T, unsigned long long>> _local;
    auto& [_local_copy, _local_version] = _local[this];
    if (this->__cache_version->load() > _local_version) {
        std::shared_lock _sentry{ this->__repository_lock };
        _local_copy = this->__underlying;
        _local_version = this->__cache_version->load();
    }
    return _local_copy;
}
