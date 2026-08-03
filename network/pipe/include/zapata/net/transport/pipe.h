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
 * @file pipe.h
 * @brief Named pipe transport implementation.
 *
 * Provides inter-process communication via named pipes.
 * Used for internal service communication within the same host.
 *
 * Capabilities: SYNCHRONOUS
 *
 * @see zpt::net::transport::pipe_stream
 */

#pragma once
#include <string>
#include <utility>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
/** @brief Returns reference to the internal server stream counter. */
auto INTERNAL_SERVER_STREAM() -> ssize_t&;
namespace net {
namespace transport {

/**
 * @brief Named pipe transport implementation.
 *
 * Supports inter-process communication within the same host.
 * Registered for "pipe" URI scheme.
 */
class pipe_stream : public zpt::basic_transport {
  public:
    pipe_stream() = default;
    virtual ~pipe_stream() = default;

    /** @brief Returns true for SYNCHRONOUS capability. */
    auto has_capability(std::uint64_t _capability) const -> bool override;
    /** @brief Creates a new request message with JSON payload. */
    auto make_request() const -> zpt::message override;
    /** @brief Creates a new reply message, optionally using the allocator. */
    auto make_reply(bool _with_allocator = true) const -> zpt::message override;
    /** @brief Creates a reply message derived from the given request. */
    auto make_reply(zpt::message _request) const -> zpt::message override;
    /** @brief Parses an incoming request message from the pipe stream. */
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
    /** @brief Parses an incoming reply message from the pipe stream. */
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override;
};
} // namespace transport
} // namespace net
} // namespace zpt
