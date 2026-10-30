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
 * queue, so one stream/fd can carry multiple in-flight exchanges. Event notification goes
 * through the standard polling layer, which drains the queue via has_next().
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
 * stream (and fd); the polling layer drains the queue via `has_next()` without re-arming
 * epoll for each message. Used internally by the `self` transport for in-node signaling.
 */
class event_stream : public basic_stream {
  public:
    /**
     * @brief Constructs an event_stream backed by a Linux eventfd.
     * @return void (constructors implicitly initialize the object).
     */
    event_stream();
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
     * @brief Returns true if there are buffered messages waiting to be read.
     * @return True if there are buffered messages available.
     */
    auto has_next() const -> bool override;
    /**
     * @brief Always true: a multiplexed stream that stays in the polling set.
     *
     * A single eventfd carries a queue of messages, so the stream is never torn down after
     * one exchange. `polling::unmute()` keeps a persistent stream with buffered work
     * (`has_next()`) in the processing loop by re-delegating it instead of re-arming
     * epoll, and re-arms it only once the queue has drained.
     * @return Always true.
     */
    auto persistent() -> bool override;

  private:
    /** @brief Queue of in-flight messages: `write_without_io` pushes, `read_without_io` pops. */
    zpt::lf::queue<std::any> __content;
};
} // namespace zpt
