#pragma once

#include <map>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <zapata/atomics/padded_atomic.h>

namespace zpt {
namespace locks {

class spin_mutex {
  public:
    friend class T;
    static constexpr bool shared{ true };
    static constexpr bool exclusive{ false };

    spin_mutex() = default;
    spin_mutex(zpt::locks::spin_mutex const&) = delete;
    virtual ~spin_mutex() = default;

    auto operator=(zpt::locks::spin_mutex const&) -> zpt::locks::spin_mutex& = delete;

    auto count_shared() -> long;
    auto count_exclusive() -> long;
    auto lock() -> zpt::locks::spin_mutex&;
    auto lock_shared() -> zpt::locks::spin_mutex&;
    auto unlock() -> zpt::locks::spin_mutex&;
    auto unlock_shared() -> zpt::locks::spin_mutex&;

  private:
    zpt::padded_atomic<long> __shared_access{ 0 };
    zpt::padded_atomic<bool> __exclusive_access{ false };

    auto spin_shared_lock() -> void;
    auto spin_exclusive_lock() -> void;
};

} // namespace locks
} // namespace zpt
