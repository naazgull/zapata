#include <zapata/rest/services.h>
#include <zapata/connector.h>

zpt::rest::services::services(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::services::blocked() const -> bool { return false; }

auto zpt::rest::services::operator()(zpt::events::dispatcher::ptr _dispatcher
                                     [[maybe_unused]]) -> zpt::events::state {

    if (this->received()->performative() == zpt::Msearch) {
        this->to_send()->status(200);
        this->to_send()->body() = { "items", zpt::json::array(), "size", 0 };
        zlog(this->to_send(), zpt::debug);
        return zpt::events::finish;
    }

    this->to_send()->status(405);
    this->to_send()->body() = { "message", "Only MSEARCH allowed to use with *" };
    return zpt::events::abort;
}
