#include <zapata/rest/services.h>
#include <zapata/connector.h>
#include <zapata/uri.h>

zpt::rest::minion_boot::minion_boot(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::minion_boot::blocked() const -> bool { return false; }

auto zpt::rest::minion_boot::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {
    auto _config = zpt::GLOBAL_CONFIG();
    auto _peer = zpt::uri::parse(this->received()->headers()("X-My-Location")->string());
    auto _scheme = _peer("scheme")->string();
    auto _peer_address = std::format("{}:{}", _peer("domain")->string(), _peer("port")->integer());
    auto _self_address =
      std::format("{}:{}", _config(_scheme)("bind")->string(), _config(_scheme)("port")->integer());

    if (this->received()->performative() == zpt::Notify && _peer_address != _self_address) {
        auto _get_services = zpt::make_message<zpt::json_message>();
        _get_services //
          ->performative(zpt::Get)
          .uri(std::format("{}://{}:{}/services",
                           _peer("scheme")->string(),
                           _peer("domain")->string(),
                           _peer("port")->integer()));
        _dispatcher->trigger<zpt::events::call<zpt::rest::services_list>>(zpt::REST_RESOLVER(),
                                                                          _get_services);
        zlog(_get_services, zpt::debug);
    }

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

zpt::rest::services_list::services_list(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::services_list::blocked() const -> bool { return false; }

auto zpt::rest::services_list::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {

    zlog(this->received(), zpt::debug);

    return zpt::events::finish;
}

auto zpt::rest::services::broadcast(zpt::json _config) -> void {
    auto _upnp_host = _config("upnp")("bind")->string();
    auto _upnp_port = _config("upnp")("port")->integer();
    auto _tcp_host = _config("tcp")("bind")->string();
    auto _tcp_port = _config("tcp")("port")->integer();
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get("upnp");
    auto _stream =
      zpt::make_stream<zpt::socketstream>(zpt::UPNP_BROADCAST, _upnp_port * 10, false, IPPROTO_UDP);
    _stream->transport("upnp");
    zpt::stream_cast<zpt::socketstream>(_stream) //
      .set_peer(_upnp_port);

    auto _message = _transport->make_request();
    _message //
      ->performative(zpt::Notify)
      .uri("/minions/boot")
      .headers()["X-My-Location"] = std::format("tcp://{}:{}", _tcp_host, _tcp_port);
    (*_stream) << _message << std::flush;
}
