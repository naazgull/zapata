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

namespace zpt {
namespace lf {

template<typename T>
class hptr_domain {
  public:
    using size_type = size_t;
    static constexpr int CACHE_LINE_PADDING{ 128 / sizeof(std::atomic<T*>) };

    hptr_domain(long _max_threads, long _ptr_per_thread);
    hptr_domain(const hptr_domain<T>& _rhs) = delete;
    hptr_domain(hptr_domain<T>&& _rhs) = delete;
    virtual ~hptr_domain();

    auto operator=(const hptr_domain<T>& _rhs) -> hptr_domain<T>& = delete;
    auto operator=(hptr_domain<T>&& _rhs) -> hptr_domain<T>& = delete;

    auto acquire(T* _ptr) -> hptr_domain<T>&;
    auto release(T* _ptr) -> hptr_domain<T>&;
    auto retire(T* _ptr) -> hptr_domain<T>&;
    auto clean() -> hptr_domain<T>&;
    auto clear() -> hptr_domain<T>&;

    auto get_thread_dangling_count() -> size_t;

    friend auto operator<<(std::ostream& _out, zpt::lf::hptr_domain<T>& _in) -> std::ostream& {
        _out << "* zpt::lf::hptr_domain(" << std::hex << &_in << "):" << std::dec << std::endl;
        _out << "  #alive    -> " << std::dec << _in.__alive.load() << std::endl
             << "  #acquired -> " << std::dec << _in.__acquired.load() << std::endl
             << "  #released -> " << _in.__released.load() << std::endl
             << "  #retired  -> " << _in.__retired.load()
             << std::endl
             // << "  #dangling ptr -> " << (_in.__allocated.load() - _in.__retired.load())
             << std::endl
             << std::flush;
        _out << "  #config -> P = " << _in.P << " | K = " << _in.K << " | N = " << _in.N
             << " | R = " << _in.R << std::dec << std::flush;
        return _out;
    }

    class hptr_pending_list {
      public:
        hptr_pending_list() = default;
        virtual ~hptr_pending_list();

        auto operator-> () -> std::map<T*, T*>*;
        auto operator*() -> std::map<T*, T*>&;

      private:
        std::map<T*, T*> __underlying;
    };

    class guard {
      public:
        friend class zpt::lf::hptr_domain<T>;

        guard(T* _target, zpt::lf::hptr_domain<T>& _parent);
        virtual ~guard();

        auto retire() -> guard&;
        auto target() const -> T&;

      private:
        T* __target{ nullptr };
        zpt::lf::hptr_domain<T>& __parent;
        bool __retire{ false };
    };

    friend class guard;

  private:
    std::atomic<T*>* __hp{ nullptr };
    std::atomic<int> __next_thr_idx{ 0 };
    std::atomic<long> __acquired{ 0 };
    std::atomic<long> __released{ 0 };
    std::atomic<long> __alive{ 0 };
    std::atomic<long> __retired{ 0 };
    long FACTOR{ 0 };
    long P{ 0 };
    long K{ 0 };
    long N{ 0 };
    long R{ 0 };

    auto get_retired() -> hptr_pending_list&;
    auto get_this_thread_idx() -> int;
};

template<typename T>
zpt::lf::hptr_domain<T>::hptr_domain(long _max_threads, long _ptr_per_thread)
  : FACTOR{ static_cast<int>(
      std::ceil(static_cast<double>(_ptr_per_thread) / static_cast<double>(CACHE_LINE_PADDING))) }
  , P{ _max_threads }
  , K{ FACTOR * CACHE_LINE_PADDING }
  , N{ P * K }
  , R{ N / 2 } {
    expect(_max_threads > 0,
           "Hazard pointer domain for the given template type has not been initialized",
           500,
           0);
    this->__hp = new std::atomic<T*>[N] { 0 };
}

template<typename T>
zpt::lf::hptr_domain<T>::~hptr_domain() {
    this->clear();
    delete[] this->__hp;
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::acquire(T* _ptr) -> zpt::lf::hptr_domain<T>& {
    size_t _idx = zpt::lf::hptr_domain<T>::get_this_thread_idx();

    for (size_t _k = _idx * K; _k != static_cast<size_t>((_idx + 1) * K); ++_k) {
        T* _null{ nullptr };
        if (this->__hp[_k].compare_exchange_strong(_null, _ptr)) {
            ++this->__acquired;
            ++this->__alive;
            return (*this);
        }
    }

    expect(false,
           "No more hazard-pointer slots available for this thread. "
           "Please, run the `clean()` "
           "before continuing",
           500,
           0);
    return (*this);
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::release(T* _ptr) -> zpt::lf::hptr_domain<T>& {
    int _idx{ zpt::lf::hptr_domain<T>::get_this_thread_idx() };

    for (size_t _k = _idx * K; _k != static_cast<size_t>((_idx + 1) * K); ++_k) {
        T* _exchange = _ptr;
        if (this->__hp[_k].compare_exchange_strong(_exchange, nullptr)) {
            ++this->__released;
            --this->__alive;
            break;
        }
    }
    return (*this);
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::retire(T* _ptr) -> zpt::lf::hptr_domain<T>& {
    auto& _retired = zpt::lf::hptr_domain<T>::get_retired();
    _retired->insert(std::make_pair(_ptr, _ptr));
    if (_retired->size() == static_cast<size_t>(R)) {
        this->clean();
    }
    return (*this);
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::clean() -> zpt::lf::hptr_domain<T>& {
    auto& _retired = zpt::lf::hptr_domain<T>::get_retired();

    std::map<T*, bool> _to_process;
    std::for_each(this->__hp, this->__hp + N, [&](const std::atomic<T*>& _item) -> void {
        T* _ptr = _item.load();
        if (_ptr != nullptr) {
            _to_process.insert(std::make_pair(_ptr, true));
        }
    });

    for (auto _it = _retired->begin(); _it != _retired->end();) {
        if (_to_process.find(_it->first) == _to_process.end()) {
            delete _it->first;
            ++this->__retired;
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
zpt::lf::hptr_domain<T>::clear() -> zpt::lf::hptr_domain<T>& {
    std::map<T*, bool> _to_process;
    std::for_each(this->__hp, this->__hp + N, [&](const std::atomic<T*>& _item) -> void {
        T* _ptr = _item.load();
        if (_ptr != nullptr)
            _to_process.insert(std::make_pair(_ptr, true));
    });
    std::for_each(
      _to_process.begin(), _to_process.end(), [&](const std::pair<T*, bool>& _item) -> void {
          ++this->__retired;
          delete _item.first;
      });
    return (*this);
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::get_thread_dangling_count() -> size_t {
    auto& _retired = zpt::lf::hptr_domain<T>::get_retired();
    return _retired->size();
}

template<typename T>
zpt::lf::hptr_domain<T>::hptr_pending_list::~hptr_pending_list() {
    for (auto _it = this->__underlying.begin(); _it != this->__underlying.end(); ++_it) {
        delete _it->first;
    }
}

template<typename T>
auto zpt::lf::hptr_domain<T>::hptr_pending_list::operator-> () -> std::map<T*, T*>* {
    return &this->__underlying;
}

template<typename T>
auto zpt::lf::hptr_domain<T>::hptr_pending_list::operator*() -> std::map<T*, T*>& {
    return this->__underlying;
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::get_retired() -> zpt::lf::hptr_domain<T>::hptr_pending_list& {
    thread_local static hptr_pending_list _retired;
    return _retired;
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::get_this_thread_idx() -> int {
    thread_local static int _idx = this->__next_thr_idx.fetch_add(1);
    return _idx;
}

template<typename T>
zpt::lf::hptr_domain<T>::guard::guard(T* _ptr, zpt::lf::hptr_domain<T>& _parent)
  : __target{ _ptr }
  , __parent{ _parent } {
    this->__parent.acquire(this->__target);
}

template<typename T>
zpt::lf::hptr_domain<T>::guard::~guard() {
    this->__parent.release(this->__target);
    if (this->__retire)
        this->__parent.retire(this->__target);
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::guard::retire() -> guard& {
    this->__retire = true;
    return (*this);
}

template<typename T>
auto
zpt::lf::hptr_domain<T>::guard::target() const -> T& {
    return *this->__target;
}

} // namespace lf
} // namespace zpt
