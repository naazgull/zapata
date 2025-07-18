#include <zapata/rest/services.h>
#include <zapata/connector.h>

zpt::rest::minion_boot::minion_boot(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::minion_boot::blocked() const -> bool { return false; }

auto zpt::rest::minion_boot::operator()(zpt::events::dispatcher::ptr _dispatcher
                                        [[maybe_unused]]) -> zpt::events::state {

    zlog(this->received(), zpt::debug);
    if (this->received()->performative() == zpt::Notify) {}

    return zpt::events::finish;
}

zpt::rest::services_collection::services_collection(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::services_collection::blocked() const -> bool { return false; }

auto zpt::rest::services_collection::operator()(zpt::events::dispatcher::ptr _dispatcher
                                                [[maybe_unused]]) -> zpt::events::state {

    if (this->received()->performative() == zpt::Get) { return zpt::events::finish; }

    this->to_send()->status(405);
    this->to_send()->body() = { "message", "Only GET allowed to use with `/services`" };
    return zpt::events::abort;
}
