#include <atomic>
#include <iostream>
#include <map>
#include <memory>
#include <thread>
#include <type_traits>
#include <zapata/base/expect.h>
#include <zapata/locks/spin_mutex.h>

auto zpt::locks::spin_mutex::count_shared() -> long { return this->__shared_access->load(); }

auto zpt::locks::spin_mutex::count_exclusive() -> long { return this->__exclusive_access->load(); }

auto zpt::locks::spin_mutex::lock_shared() -> zpt::locks::spin_mutex& {
    this->spin_shared_lock();
    return (*this);
}

auto zpt::locks::spin_mutex::lock() -> zpt::locks::spin_mutex& {
    this->spin_exclusive_lock();
    return (*this);
}

auto zpt::locks::spin_mutex::unlock_shared() -> zpt::locks::spin_mutex& {
    this->__shared_access->fetch_sub(1, std::memory_order_release);
    return (*this);
}

auto zpt::locks::spin_mutex::unlock() -> zpt::locks::spin_mutex& {
    this->__exclusive_access->store(false, std::memory_order_release);
    return (*this);
}

auto zpt::locks::spin_mutex::spin_shared_lock() -> void {
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

auto zpt::locks::spin_mutex::spin_exclusive_lock() -> void {
    while (this->__exclusive_access->exchange(true, std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    while (this->__shared_access->load(std::memory_order_seq_cst) != 0) {
        std::this_thread::yield();
    }
}
