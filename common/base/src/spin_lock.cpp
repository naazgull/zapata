#include <atomic>
#include <memory>
#include <map>
#include <thread>
#include <iostream>
#include <type_traits>

#include <zapata/base/expect.h>
#include <zapata/locks/spin_lock.h>

auto
zpt::locks::spin_lock::count_shared() -> long {
    return this->__shared_access->load();
}

auto
zpt::locks::spin_lock::count_exclusive() -> long {
    return this->__exclusive_access->load();
}

auto
zpt::locks::spin_lock::count_shared_acquired_by_thread() -> long {
    auto& _count = zpt::locks::spin_lock::__acquired_spins[this];
    return _count > 0 ? _count : 0;
}

auto
zpt::locks::spin_lock::count_exclusive_acquired_by_thread() -> long {
    auto& _count = zpt::locks::spin_lock::__acquired_spins[this];
    return _count < 0 ? -_count : 0;
}

auto
zpt::locks::spin_lock::acquire_shared() -> zpt::locks::spin_lock& {
    auto& _count = zpt::locks::spin_lock::__acquired_spins[this];
    if (_count == 0) { this->spin_shared_lock(); }
    expect(_count >= 0, "can't re-acquire in shared mode while holding in exclusive mode");
    ++_count;
    return (*this);
}

auto
zpt::locks::spin_lock::acquire_exclusive() -> zpt::locks::spin_lock& {
    auto& _count = zpt::locks::spin_lock::__acquired_spins[this];
    if (_count == 0) { this->spin_exclusive_lock(); }
    expect(_count <= 0, "can't re-acquire in exclusive mode while holding in shared mode");
    --_count;
    return (*this);
}

auto
zpt::locks::spin_lock::release_shared() -> zpt::locks::spin_lock& {
    auto _found = zpt::locks::spin_lock::__acquired_spins.find(this);
    expect(_found != zpt::locks::spin_lock::__acquired_spins.end() && _found->second > 0,
           "spin lock not acquired by thread");
    --_found->second;
    if (_found->second == 0) {
        zpt::locks::spin_lock::__acquired_spins.erase(_found);
        this->__shared_access->fetch_sub(1, std::memory_order_release);
    }
    return (*this);
}

auto
zpt::locks::spin_lock::release_exclusive() -> zpt::locks::spin_lock& {
    auto _found = zpt::locks::spin_lock::__acquired_spins.find(this);
    expect(_found != zpt::locks::spin_lock::__acquired_spins.end() && _found->second < 0,
           "spin lock not acquired by thread");
    ++_found->second;
    if (_found->second == 0) {
        zpt::locks::spin_lock::__acquired_spins.erase(_found);
        this->__exclusive_access->store(false, std::memory_order_release);
    }
    return (*this);
}

auto
zpt::locks::spin_lock::spin_shared_lock() -> void {
    do {
        if (this->__exclusive_access->load(std::memory_order_seq_cst)) {
            std::this_thread::yield();
            continue;
        }

        this->__shared_access->fetch_add(1, std::memory_order_acquire);

        if (this->__exclusive_access->load(std::memory_order_seq_cst)) {
            this->__shared_access->fetch_sub(1, std::memory_order_release);
            std::this_thread::yield();
            continue;
        }

        break;
    } while (true);
}

auto
zpt::locks::spin_lock::spin_exclusive_lock() -> void {
    while (this->__exclusive_access->exchange(true, std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    while (this->__shared_access->load(std::memory_order_seq_cst) != 0) {
        std::this_thread::yield();
    }
}

zpt::locks::spin_lock::guard::guard(zpt::locks::spin_lock& _target, bool _can_be_shared)
  : __target(_target)
  , __can_be_shared(_can_be_shared) {
    try {
        if (this->__can_be_shared) { this->__target.acquire_shared(); }
        else { this->__target.acquire_exclusive(); }
    }
    catch (zpt::failed_expectation const& _e) {
        this->__released = true;
        throw;
    }
}

zpt::locks::spin_lock::guard::~guard() { this->release(); }

auto
zpt::locks::spin_lock::guard::release() -> zpt::locks::spin_lock::guard& {
    if (this->__released) { return (*this); }
    if (this->__can_be_shared) { this->__target.release_shared(); }
    else { this->__target.release_exclusive(); }
    this->__released = true;
    return (*this);
}

auto
zpt::locks::spin_lock::guard::exclusivity() -> zpt::locks::spin_lock::guard& {
    expect(!this->__released, "can't acquire exclusive lock on an already released lock");

    if (this->__can_be_shared) {
        this->__target
          .release_shared() //
          .acquire_exclusive();
        this->__can_be_shared = false;
    }
    return (*this);
}

auto
zpt::locks::spin_lock::guard::shareability() -> zpt::locks::spin_lock::guard& {
    expect(!this->__released, "can't acquire shared lock on an already released lock");
    if (!this->__can_be_shared) {
        this->__target
          .release_exclusive() //
          .acquire_shared();
        this->__can_be_shared = true;
    }
    return (*this);
}
