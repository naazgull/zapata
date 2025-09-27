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

#pragma once

#include <zapata/catalog.h>
#include <zapata/events.h>
#include <zapata/ontology.h>
#include <zapata/rest/pending_messages.h>
#include <zapata/startup.h>
#include <zapata/transport.h>
#include <zapata/transport/engine.h>

namespace zpt {
namespace rest {
class resolver_t : public zpt::events::resolver_t {
  public:
    resolver_t(zpt::json _rest_config);
    resolver_t(resolver_t const&) = delete;
    resolver_t(resolver_t&&) = delete;
    virtual ~resolver_t() = default;

    auto operator=(resolver_t const&) -> resolver_t& = delete;
    auto operator=(resolver_t&&) -> resolver_t& = delete;

    using zpt::events::resolver_t::add;
    using zpt::events::resolver_t::remove;
    auto add(zpt::json const& _service_description) -> resolver_t& override;
    auto add(zpt::message _sent, zpt::events::resolver_callback callback) -> resolver_t& override;
    auto add(zpt::performative _performative,
             zpt::json const& _id,
             zpt::json const& _metadata,
             zpt::events::resolver_callback _callback) -> resolver_t& override;
    auto remove(zpt::message _sent) -> resolver_t& override;
    auto remove(zpt::performative _performative, zpt::json const& _id) -> resolver_t& override;
    auto resolve(zpt::message _received, zpt::events::initializer_t _initializer) const
      -> std::list<zpt::event> override;
    auto search(zpt::json const& _id, std::string const& _provider_id = "") const
      -> zpt::json override;
    auto list(std::string const& _provider_id = "") const -> zpt::json override;
    auto register_provider(zpt::json const& _provider) -> zpt::rest::resolver_t& override;
    auto unregister_provider(std::string const& _id) -> zpt::rest::resolver_t& override;
    auto get_provider(std::string const& _id) const -> zpt::json override;
    auto clear() -> resolver_t& override;

  private:
    zpt::catalog<std::string, zpt::json> __catalog;
    std::vector<zpt::events::resolver_callback> __callbacks;
    mutable zpt::rest::pending_messages __pending_requests;
    zpt::json __configuration;
};
} // namespace rest
auto REST_RESOLVER(zpt::json _config = nullptr) -> zpt::events::resolver;
} // namespace zpt
