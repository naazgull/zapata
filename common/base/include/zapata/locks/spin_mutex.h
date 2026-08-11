/**
 * @file spin_mutex.h
 * @brief Spin-lock based reader-writer mutex.
 *
 * Provides a spin-lock mutex that supports both shared (reader) and exclusive
 * (writer) access patterns. Uses busy-waiting, making it suitable for
 * short critical sections where blocking would be more expensive.
 */

#pragma once

#include <map>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <zapata/atomics/padded_atomic.h>

namespace zpt {
namespace locks {

/**
 * @brief Spin-lock based reader-writer mutex.
 *
 * A mutex implementation using atomic spin-waiting rather than OS-level
 * blocking. Supports multiple concurrent readers or a single exclusive writer.
 *
 * This is suitable for:
 * - Short critical sections where context switch overhead exceeds wait time
 * - Real-time systems where blocking is undesirable
 * - Lock-free data structure implementations
 *
 * @par Example Usage
 * @code
 * zpt::locks::spin_mutex mutex;
 *
 * // Exclusive access (writer)
 * mutex.lock();
 * // ... modify data ...
 * mutex.unlock();
 *
 * // Shared access (reader)
 * mutex.lock_shared();
 * // ... read data ...
 * mutex.unlock_shared();
 * @endcode
 *
 * @warning Spin-locks consume CPU while waiting. Use std::shared_mutex for
 *          longer critical sections.
 *
 * @note This class is non-copyable.
 */
class spin_mutex {
  public:
    friend class T;

    static constexpr bool shared{ true };     ///< Constant for shared lock mode.
    static constexpr bool exclusive{ false }; ///< Constant for exclusive lock mode.

    /**
     * @brief Default constructor.
     * @return none
     */
    spin_mutex() = default;
    spin_mutex(zpt::locks::spin_mutex const&) = delete;
    /** @brief Destructor. */
    virtual ~spin_mutex() = default;

    auto operator=(zpt::locks::spin_mutex const&) -> zpt::locks::spin_mutex& = delete;

    /**
     * @brief Returns the number of active shared locks.
     * @return Count of threads holding shared access.
     */
    auto count_shared() -> long;

    /**
     * @brief Returns whether exclusive lock is held.
     * @return 1 if exclusive lock is held, 0 otherwise.
     */
    auto count_exclusive() -> long;

    /**
     * @brief Acquires exclusive (writer) lock.
     * @return Reference to this mutex.
     *
     * Blocks (spins) until exclusive access can be acquired.
     * No other threads may hold shared or exclusive locks.
     */
    auto lock() -> zpt::locks::spin_mutex&;

    /**
     * @brief Acquires shared (reader) lock.
     * @return Reference to this mutex.
     *
     * Blocks (spins) until shared access can be acquired.
     * Multiple threads may hold shared locks simultaneously.
     */
    auto lock_shared() -> zpt::locks::spin_mutex&;

    /**
     * @brief Releases exclusive (writer) lock.
     * @return Reference to this mutex.
     */
    auto unlock() -> zpt::locks::spin_mutex&;

    /**
     * @brief Releases shared (reader) lock.
     * @return Reference to this mutex.
     */
    auto unlock_shared() -> zpt::locks::spin_mutex&;

  private:
    zpt::padded_atomic<long> __shared_access{ 0 };        ///< Count of shared lock holders.
    zpt::padded_atomic<bool> __exclusive_access{ false }; ///< Exclusive lock flag.
    std::thread::id __exclusive_owner;                    ///< Thread ID of exclusive lock holder.

    /**
     * @brief Spins until shared lock can be acquired.
     * @return void (blocks until shared lock is held).
     */
    auto spin_shared_lock() -> void;

    /**
     * @brief Spins until exclusive lock can be acquired.
     * @return void (blocks until exclusive lock is held).
     */
    auto spin_exclusive_lock() -> void;
};

} // namespace locks
} // namespace zpt
