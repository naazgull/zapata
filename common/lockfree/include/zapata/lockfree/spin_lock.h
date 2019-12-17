#pragma once

#include <atomic>
#include <memory>
#include <map>
#include <type_traits>

namespace zpt {
namespace lf {

class spin_lock {
  public:
    friend class T;

    class guard {
      public:
        friend class zpt::lf::spin_lock;

        guard(zpt::lf::spin_lock& _target, bool _can_be_shared = false);
        guard(zpt::lf::spin_lock::guard const&) = delete;
        guard(zpt::lf::spin_lock::guard&&) = delete;
        virtual ~guard();

        auto operator=(zpt::lf::spin_lock::guard const&) -> zpt::lf::spin_lock::guard& = delete;
        auto operator=(zpt::lf::spin_lock::guard &&) -> zpt::lf::spin_lock::guard& = delete;

        auto release() -> zpt::lf::spin_lock::guard&;
        auto exclusivity() -> zpt::lf::spin_lock::guard&;
        auto shareability() -> zpt::lf::spin_lock::guard&;

      private:
        zpt::lf::spin_lock& __target;
        bool __can_be_shared{ false };
        bool __released{ false };
    };
    friend class zpt::lf::spin_lock::guard;

    spin_lock() = default;
    virtual ~spin_lock() = default;

    auto acquire_shared() -> zpt::lf::spin_lock&;
    auto acquire_exclusive() -> zpt::lf::spin_lock&;
    auto release_shared() -> zpt::lf::spin_lock&;
    auto release_exclusive() -> zpt::lf::spin_lock&;

    auto exclusivity() -> zpt::lf::spin_lock&;
    auto shareability() -> zpt::lf::spin_lock&;

  private:
    std::atomic<long> __shared_access{ 0 };
    std::atomic<bool> __exclusive_access{ false };

    static inline thread_local std::map<zpt::lf::spin_lock*, bool> __acquired_spins;

    auto spin_shared_lock() -> void;
    auto spin_exclusive_lock() -> void;
};

} // namespace lf
} // namespace zpt
