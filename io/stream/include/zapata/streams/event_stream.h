/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file event_stream.h
 * @brief Multiplexed eventfd stream for the polling infrastructure.
 *
 * Wraps a single Linux eventfd as a basic_stream that buffers many messages in a lock-free
 * queue, so one stream/fd can carry multiple in-flight exchanges. Each write increments the
 * eventfd semaphore, so the polling layer delivers one queued message per ready event; the
 * still-pending counter keeps epoll firing until the queue drains.
 *
 * @see zpt::basic_stream
 */

#pragma once

#include <any>
#include <sys/eventfd.h>
#include <zapata/streams/streams.h>

namespace zpt {

/**
 * @brief Multiplexed eventfd stream carrying a queue of in-flight messages.
 *
 * Wraps a single Linux eventfd as a basic_stream that buffers many logical messages in a
 * lock-free queue behind one fd. Multiple in-flight request/reply exchanges share the same
 * stream (and fd); each write bumps the eventfd semaphore, so the polling layer delivers one
 * queued message per ready event, driven by the still-pending counter. Used internally by the
 * `self` transport for in-node signaling.
 */
class event_stream : public basic_stream {
  public:
    /**
     * @brief Constructs an event_stream backed by a Linux eventfd.
     * @return void (constructors implicitly initialize the object).
     */
    event_stream(size_t _message_queue_size);
    event_stream(event_stream const& _rhs) = delete;
    event_stream(event_stream&& _rhs) = delete;
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~event_stream();

    auto operator=(event_stream const& _rhs) -> event_stream& = delete;
    auto operator=(event_stream&& _rhs) -> event_stream& = delete;

    /**
     * @brief Sets the file descriptor.
     * @param _rhs File descriptor value.
     * @return Reference to this event_stream.
     */
    auto operator=(int _rhs) -> event_stream&;
    /**
     * @brief Applies a stream manipulator (e.g., std::flush).
     * @param _in Manipulator function.
     * @return Reference to this event_stream.
     */
    auto operator<<(ostream_manipulator _in) -> event_stream&;
    /**
     * @brief Pops the next queued message without I/O.
     * @param _out Output reference for the read value.
     * @return Reference to this event_stream.
     */
    auto read_without_io(std::any& _out) -> event_stream& override;
    /**
     * @brief Pushes a message onto the internal queue without I/O.
     * @param _in Value to write.
     * @return Reference to this event_stream.
     */
    auto write_without_io(std::any const& _in) -> event_stream& override;
    /**
     * @brief Always false: delivery is driven by the eventfd semaphore counter, not by
     *        polling this stream for buffered messages.
     * @return Always false.
     */
    auto has_next() const -> bool override;
    /**
     * @brief Always true: a multiplexed stream that stays in the polling set.
     *
     * A single eventfd carries a queue of messages, so the stream is never torn down after
     * one exchange. Because `has_next()` is always `false`, `polling::unmute()` re-arms this
     * stream in epoll after every message; the still-pending eventfd counter then fires the
     * next ready event, so the backlog drains one message per cycle.
     * @return Always true.
     */
    auto persistent() const -> bool override;

  private:
    /** @brief Queue of in-flight messages: `write_without_io` pushes, `read_without_io` pops. */
    zpt::lf::queue<std::any> __content;
};
} // namespace zpt
