#include <atomic>
#include <memory>
#include <map>
#include <thread>
#include <type_traits>

#include <zapata/base/expect.h>
#include <zapata/lockfree/spin_lock.h>

auto
zpt::lf::spin_lock::acquire_shared() -> zpt::lf::spin_lock& {
    auto _found = zpt::lf::spin_lock::__acquired_spins.find(this);
    expect(_found != zpt::lf::spin_lock::__acquired_spins.end(),
           "spin lock already acquired by thread",
           500,
           0);
    this->spin_shared_lock();
    zpt::lf::spin_lock::__acquired_spins.insert(std::make_pair(this, true));
    return (*this);
}

auto
zpt::lf::spin_lock::acquire_exclusive() -> zpt::lf::spin_lock& {
    auto _found = zpt::lf::spin_lock::__acquired_spins.find(this);
    expect(_found != zpt::lf::spin_lock::__acquired_spins.end(),
           "spin lock already acquired by thread",
           500,
           0);
    this->spin_exclusive_lock();
    zpt::lf::spin_lock::__acquired_spins.insert(std::make_pair(this, false));
    return (*this);
}

auto
zpt::lf::spin_lock::release_shared() -> zpt::lf::spin_lock& {
    zpt::lf::spin_lock::__acquired_spins.erase(zpt::lf::spin_lock::__acquired_spins.find(this));
    this->__shared_access.fetch_sub(1, std::memory_order_release);
    return (*this);
}

auto
zpt::lf::spin_lock::release_exclusive() -> zpt::lf::spin_lock& {
    zpt::lf::spin_lock::__acquired_spins.erase(zpt::lf::spin_lock::__acquired_spins.find(this));
    this->__exclusive_access.store(false, std::memory_order_release);
    return (*this);
}

auto
zpt::lf::spin_lock::exclusivity() -> zpt::lf::spin_lock& {
    while (this->__exclusive_access.exchange(true, std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    this->__shared_access.fetch_sub(1, std::memory_order_release);
    while (this->__shared_access.load(std::memory_order_seq_cst) != 0) {
        std::this_thread::yield();
    }
    return (*this);
}

auto
zpt::lf::spin_lock::shareability() -> zpt::lf::spin_lock& {
    this->__shared_access.fetch_add(1, std::memory_order_acquire);
    this->__exclusive_access.store(false, std::memory_order_release);
    return (*this);
}

auto
zpt::lf::spin_lock::spin_shared_lock() -> void {
    do {
        if (this->__exclusive_access.load(std::memory_order_seq_cst)) {
            std::this_thread::yield();
            continue;
        }

        this->__shared_access.fetch_add(1, std::memory_order_acquire);

        if (this->__exclusive_access.load(std::memory_order_seq_cst)) {
            this->__shared_access.fetch_sub(1, std::memory_order_release);
            std::this_thread::yield();
            continue;
        }

        break;
    } while (true);
}

auto
zpt::lf::spin_lock::spin_exclusive_lock() -> void {
    while (this->__exclusive_access.exchange(true, std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    while (this->__shared_access.load(std::memory_order_seq_cst) != 0) {
        std::this_thread::yield();
    }
}

zpt::lf::spin_lock::guard::guard(zpt::lf::spin_lock& _target, bool _can_be_shared)
  : __target(_target)
  , __can_be_shared(_can_be_shared) {
    try {
        if (this->__can_be_shared) {
            this->__target.acquire_shared();
        }
        else {
            this->__target.acquire_exclusive();
        }
    }
    catch (zpt::failed_expectation& _e) {
        this->__released = true;
    }
}

zpt::lf::spin_lock::guard::~guard() {
    this->release();
}

auto
zpt::lf::spin_lock::guard::release() -> zpt::lf::spin_lock::guard& {
    if (this->__released) {
        return (*this);
    }
    if (this->__can_be_shared) {
        this->__target.release_shared();
    }
    else {
        this->__target.release_shared();
    }
    this->__released = true;
    return (*this);
}

auto
zpt::lf::spin_lock::guard::exclusivity() -> zpt::lf::spin_lock::guard& {
    expect(!this->__released, "can't acquire exclusive lock on an already released lock", 500, 0);

    if (this->__can_be_shared) {
        this->__target.exclusivity();
        this->__can_be_shared = false;
    }
    return (*this);
}

auto
zpt::lf::spin_lock::guard::shareability() -> zpt::lf::spin_lock::guard& {
    expect(!this->__released, "can't acquire shared lock on an already released lock", 500, 0);
    if (!this->__can_be_shared) {
        this->__target.shareability();
        this->__can_be_shared = true;
    }
    return (*this);
}
