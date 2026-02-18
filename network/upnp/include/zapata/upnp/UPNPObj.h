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
 * @file UPNPObj.h
 * @brief UPnP/SSDP message types (request and reply).
 *
 * Extends the HTTP message types with SSDP-specific serialization
 * for M-SEARCH and NOTIFY messages.
 */

#pragma once

#include <zapata/http/HTTPObj.h>

namespace zpt {
namespace upnp {

/** @brief SSDP request message (M-SEARCH, NOTIFY). */
class basic_request : public zpt::http::basic_request {
  public:
    basic_request();
    basic_request(zpt::basic_message const& _request, bool);
    virtual ~basic_request() = default;

    auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    auto from_stream(std::istream& _in) -> zpt::basic_message& override;
};
using request = std::shared_ptr<zpt::upnp::basic_request>;

/** @brief SSDP reply message. */
class basic_reply : public zpt::http::basic_reply {
  public:
    basic_reply();
    basic_reply(zpt::basic_message const& _request, bool);
    virtual ~basic_reply() = default;

    auto to_stream(std::ostream& _out) const -> zpt::basic_message const& override;
    auto from_stream(std::istream& _in) -> zpt::basic_message& override;
};
using reply = std::shared_ptr<zpt::upnp::basic_reply>;
} // namespace upnp

void init(zpt::upnp::basic_request& _out);
void init(zpt::upnp::basic_reply& _out);
} // namespace zpt
