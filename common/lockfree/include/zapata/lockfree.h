/*
  This is free and unencumbered software released into the public domain.
*/

/**
 * @file lockfree.h
 * @brief Lock-free data structures for concurrent programming.
 *
 * This is the aggregate header for the lock-free module. It provides
 * thread-safe data structures that do not use traditional locks, instead
 * relying on atomic operations and hazard pointers for safe memory reclamation.
 *
 * Key components:
 * - `zpt::lf::hazard_ptr` - Hazard pointer implementation for safe memory reclamation
 * - `zpt::lf::queue` - Lock-free FIFO queue
 *
 * @par Memory Safety
 * Lock-free data structures require careful memory management. This module
 * uses hazard pointers to safely reclaim memory without risking use-after-free
 * errors that can occur with simple reference counting in lock-free contexts.
 *
 * @par Example
 * @code
 * #include <zapata/lockfree.h>
 *
 * // Create a lock-free queue for up to 8 threads
 * zpt::lf::queue<int> queue(8);
 *
 * // Producer thread
 * queue.push(42);
 * queue.push(100);
 *
 * // Consumer thread
 * try {
 *     int value = queue.pop();
 * } catch (zpt::NoMoreElementsException&) {
 *     // Queue is empty
 * }
 *
 * // Clean up thread-local hazard pointer state before thread exit
 * queue.clear_thread_context();
 * @endcode
 *
 * @see zpt::lf::hazard_ptr
 * @see zpt::lf::queue
 */

#pragma once

#include <zapata/lockfree/queue.h>
