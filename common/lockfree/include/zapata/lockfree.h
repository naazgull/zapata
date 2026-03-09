/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file lockfree.h
 * @brief Lock-free data structures for concurrent programming.
 *
 * This is the aggregate header for the lock-free module. It provides
 * thread-safe data structures that do not use traditional locks, instead
 * relying on atomic operations for safe, contention-free access.
 *
 * Key components:
 * - `zpt::lf::queue` - Bounded lock-free FIFO queue
 *
 * @par Example
 * @code
 * #include <zapata/lockfree.h>
 *
 * // Create a lock-free queue with capacity for 1000 elements
 * zpt::lf::queue<int> queue(1000);
 *
 * // Producer thread
 * auto item = zpt::allocate_unique<int>(42);
 * queue.push(std::move(item));
 *
 * // Consumer thread
 * try {
 *     auto value = queue.pop();
 *     process(*value);
 * } catch (zpt::NoMoreElementsException&) {
 *     // Queue is empty
 * }
 * @endcode
 *
 * @see zpt::lf::queue
 */

#pragma once

#include <zapata/lockfree/queue.h>
