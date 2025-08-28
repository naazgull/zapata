#include <zapata/rest/services.h>
#include <zapata/connector.h>
#include <zapata/uri.h>

namespace {
auto add_minion(zpt::json const& _minion) -> void;
}

zpt::rest::minion_boot::minion_boot(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::minion_boot::blocked() const -> bool { return false; }

auto zpt::rest::minion_boot::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {
    auto _config = zpt::GLOBAL_CONFIG();
    auto _peer_id = this->received()->headers()("X-My-ID")->string();

    if (this->received()->performative() == zpt::Notify &&
        _peer_id != zpt::SELF()("_id")->string()) {
        zpt::DISPATCHER() //
          ->trigger<zpt::system_event>(zpt::system_event_type::MINION_BOOT_RECEIVED,
                                       this->received()->headers());

#ifndef PROPAGATE_EXCEPTION
        try {
#endif
            auto _peer = zpt::uri::parse(this->received()->headers()("X-My-Location")->string());
            auto _peer_scheme = _peer("scheme")->string();
            auto _transport = zpt::TRANSPORT_LAYER() //
                                .get(_peer_scheme);

            auto _hello = _transport->make_request();
            _hello //
              ->performative(zpt::Post)
              .uri(std::format("{}://{}:{}/minions/hello",
                               _peer_scheme,
                               _peer("domain")->string(),
                               _peer("port")->integer()))
              .body() = { "provider", zpt::SELF(), "services", zpt::REST_RESOLVER()->list() };

            _dispatcher->trigger<zpt::events::call<zpt::rest::services_list>>(zpt::REST_RESOLVER(),
                                                                              _hello);
#ifndef PROPAGATE_EXCEPTION
        }
        catch (std::exception const& _e) {
            zlog(_e.what(), zpt::debug)
        }
#endif
    }

    return zpt::events::finish;
}

zpt::rest::minion_shutdown::minion_shutdown(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::minion_shutdown::blocked() const -> bool { return false; }

auto zpt::rest::minion_shutdown::operator()(zpt::events::dispatcher::ptr _dispatcher
                                            [[maybe_unused]]) -> zpt::events::state {
    auto _peer_id = this->received()->headers()("X-My-ID")->string();

    if (this->received()->performative() == zpt::Notify &&
        _peer_id != zpt::SELF()("_id")->string()) {
        zpt::DISPATCHER() //
          ->trigger<zpt::system_event>(zpt::system_event_type::MINION_SHUTDOWN_RECEIVED,
                                       this->received()->headers());

        try {
            zpt::REST_RESOLVER()->unregister_provider(_peer_id);
        }
        catch (std::exception const& _e) {
            zlog(_e.what(), zpt::debug)
        }
    }

    return zpt::events::finish;
}

zpt::rest::minion_hello::minion_hello(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::minion_hello::blocked() const -> bool { return false; }

auto zpt::rest::minion_hello::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {
    if (this->received()->performative() == zpt::Post) {
        auto _minion = this->received()->body();

        zpt::DISPATCHER() //
          ->trigger<zpt::system_event>(zpt::system_event_type::MINION_HELLO_RECEIVED, _minion);

        if (_minion("provider")->ok()) { ::add_minion(_minion); }
        else { zlog("Malformed service list: " << _minion, zpt::error); }

        this //
          ->to_send()
          ->status(200)
          .body() = { "provider", zpt::SELF(), "services", zpt::REST_RESOLVER()->list() };
        return zpt::events::finish;
    }

    this //
      ->to_send()
      ->status(405)
      .body() = { "message", "Only GET allowed to use with `/services`" };
    return zpt::events::abort;
}

zpt::rest::services_list::services_list(zpt::message _received)
  : zpt::events::process{ _received } {}

auto zpt::rest::services_list::blocked() const -> bool { return false; }

auto zpt::rest::services_list::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {
    auto _minion = this->received()->body();
    if (_minion("provider")->ok()) {
        ::add_minion(_minion);
        return zpt::events::finish;
    }
    zlog("Malformed service list: " << _minion, zpt::error);
    return zpt::events::abort;
}

auto zpt::rest::services::broadcast(std::string const& _path, zpt::json const& _config) -> void {
    auto _upnp_host = _config("upnp")("bind")->string();
    auto _upnp_port = _config("upnp")("port")->integer();
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get("upnp");

    auto _message = _transport->make_request();
    _message //
      ->performative(zpt::Notify)
      .uri(std::format("upnp://{}:{}{}", _upnp_host, _upnp_port, _path));

    _message->headers()["X-My-Location"] = zpt::get_default_uri();
    _message->headers()["X-My-ID"] = zpt::SELF()("_id");

    auto _stream = zpt::make_stream<zpt::socketstream>(zpt::NO_SSL, IPPROTO_UDP);
    _stream //
      ->transport("upnp")
      .set_peer<zpt::socketstream>(_upnp_host, _upnp_port);

    _transport->send(_stream, _message);
}

namespace {
auto add_minion(zpt::json const& _minion) -> void {
    try {
        auto _resolver = zpt::REST_RESOLVER();
        _resolver->register_provider(_minion("provider"));

        for (auto const& [_, __, _service] : _minion("services")) {
            if (_service("_id")->string().find("/minions") == std::string::npos) {
                _resolver->add(_service);

                zpt::DISPATCHER() //
                  ->trigger<zpt::system_event>(zpt::system_event_type::REGISTERED_REMOTE_SERVICE,
                                               _service);
            }
        }

        return;
    }
    catch (std::exception const& _e) {
        zlog("Error caught while processing service list: " << _e.what(), zpt::error)
    }
}
} // namespace
