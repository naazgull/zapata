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
 * @brief Event file descriptor-based stream for epoll integration.
 *
 * Wraps a Linux eventfd as a basic_stream, enabling event notification
 * through the standard polling infrastructure.
 *
 * @see zpt::basic_stream
 */

#pragma once

#include <any>
#include <sys/eventfd.h>
#include <zapata/streams/streams.h>

namespace zpt {

/**
 * @brief Stream backed by a Linux eventfd for event signaling.
 *
 * Provides a basic_stream interface over a Linux eventfd, allowing
 * event-driven wakeup of polling threads. Used internally by the
 * transport layer for signaling between threads.
 */
class event_stream : public basic_stream {
  public:
    event_stream();
    event_stream(event_stream const& _rhs) = delete;
    event_stream(event_stream&& _rhs) = delete;
    virtual ~event_stream();

    auto operator=(event_stream const& _rhs) -> event_stream& = delete;
    auto operator=(event_stream&& _rhs) -> event_stream& = delete;

    /** @brief Sets the file descriptor. */
    auto operator=(int _rhs) -> event_stream&;
    /** @brief Applies a stream manipulator (e.g., std::flush). */
    auto operator<<(ostream_manipulator _in) -> event_stream&;
    /** @brief Reads content from the internal buffer without I/O. */
    auto read_without_io(std::any& _out) -> event_stream& override;
    /** @brief Writes content to the internal buffer without I/O. */
    auto write_without_io(std::any const& _in) -> event_stream& override;
    /**
     * @brief Returns false once both the request write and the reply read have completed.
     *
     * The stream counts `read_without_io()` calls via `__reads`. When `__reads` reaches
     * `FINAL_REPLY_RECEIVED` (2), `persistent()` returns false so `unmute()` erases the
     * stream from the polling set.
     */
    auto persistent() -> bool override;

  private:
    std::any __content;
    /** @brief Number of times `read_without_io()` has been called; stream is disposed at 2. */
    unsigned short __reads{ 0 };
};
} // namespace zpt
