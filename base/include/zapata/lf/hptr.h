#pragma once

#include <memory>
#include <iterator>
#include <atomic>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>

#include <zapata/base/assertz.h>

namespace zpt {
namespace lf {

template <typename T> class hptr_domain {
  public:
    using size_type = size_t;
    using hptr_pending_list = std::vector<T*>;

    static const int CACHE_LINE_PADDING = 128 / sizeof(std::atomic<T*>);

    hptr_domain(const hptr_domain<T>& _rhs);
    hptr_domain(hptr_domain<T>&& _rhs);
    virtual ~hptr_domain();

    auto operator=(const hptr_domain<T>& _rhs) -> hptr_domain<T>&;
    auto operator=(hptr_domain<T>&& _rhs) -> hptr_domain<T>&;

    auto add(T* _ptr) -> hptr_domain<T>&;
    auto remove(T* _ptr) -> hptr_domain<T>&;
    auto clean() -> hptr_domain<T>&;

    static auto get_instance(int _n_threads = 0, int _n_hp_per_thread = 0) -> hptr_domain<T>&;
    static auto get_this_thread_idx() -> int;
    static auto clean_this_thread() -> void;

  private:
    int __P;
    int __K;
    int __N;
    int __R;
    std::atomic<T*>* __hp;
    std::atomic<int> __next_thr_idx{0};

    hptr_domain();
    hptr_domain(int _n_threads, int _n_hp_per_thread);

    auto set_max_threads(int _n_threads) -> hptr_domain<T>&;
    auto set_max_hps_per_thread(int _n_hp_per_thread) -> hptr_domain<T>&;
    auto verify() -> hptr_domain<T>&;

    static auto get_retired() -> hptr_pending_list&;
};

template <typename T>
zpt::lf::hptr_domain<T>::hptr_domain() : __P{0}, __K{0}, __N{0}, __R{0}, __hp{nullptr} {}

template <typename T>
zpt::lf::hptr_domain<T>::hptr_domain(int _n_threads, int _n_hp_per_thread) : __P{_n_threads} {
    int _factor =
        std::ceil(static_cast<double>(_n_hp_per_thread) / static_cast<double>(CACHE_LINE_PADDING));
    this->__K = _factor * CACHE_LINE_PADDING;
    this->__N = this->__P * this->__K;
    this->__R = 2 * this->__N;
    this->__hp = new std::atomic<T*>[this->__N](nullptr);
}

template <typename T> zpt::lf::hptr_domain<T>::hptr_domain(const hptr_domain<T>& _rhs) {
    (*this) = _rhs;
}

template <typename T> zpt::lf::hptr_domain<T>::hptr_domain(hptr_domain<T>&& _rhs) {
    (*this) = _rhs;
}

template <typename T> zpt::lf::hptr_domain<T>::~hptr_domain() {
    delete[] this->__hp;
    this->__hp = nullptr;
    for (auto _hptr : zpt::lf::hptr_domain<T>::get_retired()) {
        delete _hptr;
    }
    zpt::lf::hptr_domain<T>::get_retired().clear();
}

template <typename T>
auto zpt::lf::hptr_domain<T>::operator=(const hptr_domain<T>& _rhs) -> zpt::lf::hptr_domain<T>& {
    this->__P = _rhs.__P;
    this->__K = _rhs.__K;
    this->__N = _rhs.__N;
    this->__R = _rhs.__R;
    this->__hp = _rhs.__hp;
    return (*this);
}

template <typename T>
auto zpt::lf::hptr_domain<T>::operator=(hptr_domain<T>&& _rhs) -> zpt::lf::hptr_domain<T>& {
    this->__P = _rhs.__P;
    this->__K = _rhs.__K;
    this->__N = _rhs.__N;
    this->__R = _rhs.__R;
    this->__hp = _rhs.__hp;
    _rhs.__hp = nullptr;
    return (*this);
}

template <typename T> auto zpt::lf::hptr_domain<T>::add(T* _ptr) -> zpt::lf::hptr_domain<T>& {
    size_t _idx = zpt::lf::hptr_domain<T>::get_this_thread_idx();

    for (size_t _k = _idx * this->__K; _k != (_idx + 1) * this->__K; ++_k) {
        T* _null = nullptr;
        if (this->__hp[_k].compare_exchange_strong(_null, _ptr)) {
            return (*this);
        }
    }
    assertz(false,
            "No more hazard-pointer slots available for this thread. Please, run the `clean()` "
            "before continuing",
            500,
            0);
    return (*this);
}

template <typename T> auto zpt::lf::hptr_domain<T>::remove(T* _ptr) -> zpt::lf::hptr_domain<T>& {
    int _idx = zpt::lf::hptr_domain<T>::get_this_thread_idx();
    auto _retired = zpt::lf::hptr_domain<T>::get_retired();
    _retired.push_back(_ptr);

    for (size_t _k = _idx * this->__K; _k != static_cast<size_t>((_idx + 1) * this->__K); ++_k) {
        if (this->__hp[_k].compare_exchange_strong(_ptr, nullptr)) {
            break;
        }
    }

    if (_retired.size() == static_cast<size_t>(this->__R)) {
        this->clean();
    }
    return (*this);
}

template <typename T> auto zpt::lf::hptr_domain<T>::clean() -> zpt::lf::hptr_domain<T>& {
    auto _retired = zpt::lf::hptr_domain<T>::get_retired();

    std::map<T*, size_t> _to_process;
    for (size_t idx = 0; idx != static_cast<size_t>(this->__N); ++idx) {
        T* ptr = this->__hp[idx].load();
        if (ptr != nullptr) {
            _to_process.insert(std::make_pair(ptr, idx));
        }
    }

    std::vector<size_t> _to_delete;
    for (size_t _idx = 0; _idx != _retired.size(); ++_idx) {
        if (_to_process.find(_retired[_idx]) == _to_process.end()) {
            _to_delete.push_back(_idx);
        }
    }

    for (auto _idx : _to_delete) {
        delete _retired[_idx];
        _retired.erase(_retired.begin() + _idx);
    }

    return (*this);
}

template <typename T>
auto zpt::lf::hptr_domain<T>::get_instance(int _n_threads, int _n_hp_per_thread)
    -> zpt::lf::hptr_domain<T>& {
    static zpt::lf::hptr_domain<T> _instance;
    _instance.set_max_threads(_n_threads).set_max_hps_per_thread(_n_hp_per_thread).verify();
    return _instance;
}

template <typename T>
auto zpt::lf::hptr_domain<T>::set_max_threads(int _n_threads) -> zpt::lf::hptr_domain<T>& {
    if (this->__P == 0) {
        this->__P = _n_threads;
        if (this->__K != 0) {
            this->__N = this->__P * this->__K;
            this->__R = 2 * this->__N;
        }
    }
    return (*this);
}

template <typename T>
auto zpt::lf::hptr_domain<T>::set_max_hps_per_thread(int _n_hp_per_thread)
    -> zpt::lf::hptr_domain<T>& {
    if (this->__K == 0) {
        int _factor = std::ceil(static_cast<double>(_n_hp_per_thread) /
                                static_cast<double>(CACHE_LINE_PADDING));
        this->__K = _factor * CACHE_LINE_PADDING;
        if (this->__P != 0) {
            this->__N = this->__P * this->__K;
            this->__R = 2 * this->__N;
        }
    }
    return (*this);
}

template <typename T> auto zpt::lf::hptr_domain<T>::verify() -> zpt::lf::hptr_domain<T>& {
    if (this->__hp == nullptr && this->__N != 0) {
        this->__hp = new std::atomic<T*>[this->__N]();
    }
    return (*this);
}

template <typename T> auto zpt::lf::hptr_domain<T>::get_retired() -> std::vector<T*>& {
    thread_local static hptr_pending_list _retired;
    return _retired;
}

template <typename T> auto zpt::lf::hptr_domain<T>::get_this_thread_idx() -> int {
    thread_local static int _idx =
        zpt::lf::hptr_domain<T>::get_instance().__next_thr_idx.fetch_add(1);
    return _idx;
}

template <typename T> auto zpt::lf::hptr_domain<T>::clean_this_thread() -> void {
    for (auto _hptr : zpt::lf::hptr_domain<T>::get_retired()) {
        delete _hptr;
    }
    zpt::lf::hptr_domain<T>::get_retired().clear();
}

} // namespace wf
} // namespaace zpt
