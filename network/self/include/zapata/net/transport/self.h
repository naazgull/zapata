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
 * @file self.h
 * @brief In-process transport for local service calls.
 *
 * Provides zero-copy message passing within the same process.
 * Used when a service calls another service in the same application.
 *
 * Capabilities: SYNCHRONOUS
 *
 * @see zpt::net::transport::self
 */

#pragma once
#include <string>
#include <utility>
#include <zapata/catalog.h>
#include <zapata/streams.h>
#include <zapata/transport.h>

namespace zpt {
namespace net {
namespace transport {

/**
 * @brief In-process transport for local calls.
 *
 * Provides efficient in-memory message passing without serialization.
 * Registered for "self" URI scheme.
 */
class self : public zpt::basic_transport {
  public:
    self() = default;
    virtual ~self() = default;

    /** @brief Returns true for SYNCHRONOUS capability.
     * @param _capability Capability flag to check.
     * @return True if checking for SYNCHRONOUS. */
    auto has_capability(std::uint64_t _capability) const -> bool override;
    /** @brief Creates a new request message with JSON payload.
     * @return Shared pointer to the new request message. */
    auto make_request() const -> zpt::message override;
    /** @brief Creates a new reply message, optionally using the allocator.
     * @param _with_allocator If true, uses the memory pool allocator.
     * @return Shared pointer to the new reply message. */
    auto make_reply(bool _with_allocator = true) const -> zpt::message override;
    /** @brief Creates a reply message derived from the given request.
     * @param _request The request message to reply to.
     * @return Shared pointer to the new reply message. */
    auto make_reply(zpt::message _request) const -> zpt::message override;
    /** @brief Processes an incoming request message in the same process.
     * @param _stream Input stream containing the serialized request.
     * @return Parsed request message. */
    auto process_incoming_request(zpt::stream _stream) const -> zpt::message override;
    /** @brief Processes an incoming reply message in the same process.
     * @param _stream Input stream containing the serialized reply.
     * @return Parsed reply message. */
    auto process_incoming_reply(zpt::stream _stream) const -> zpt::message override;
    /** @brief Creates a copy of the given message with the transport's protocol and format.
     * @param _to_copy The message to copy.
     * @return The copied message. */
    auto copy(zpt::message const& _to_copy) const -> zpt::message override;
};
} // namespace transport
} // namespace net
/** @brief Returns the global catalog identifier.
 * @param _name Catalog name.
 * @param _self_id Self identifier.
 * @return Shared pointer to the global catalog. */
auto CATALOG(std::string const& _name = "", std::string const& _self_id = "")
  -> zpt::catalog<std::string, zpt::json>::ptr;
} // namespace zpt
