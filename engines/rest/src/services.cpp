#include <zapata/connector.h>
#include <zapata/rest/services.h>
#include <zapata/uri.h>

namespace {
auto add_minion(zpt::json const& _minion) -> void;
}

auto zpt::rest::minion_boot::blocked() const -> bool { return false; }

auto zpt::rest::minion_boot::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {
    auto _config = zpt::GLOBAL_CONFIG();
    auto _peer_id = this->received()->headers()("X-My-ID")->string();

    if (this->received()->performative() == zpt::Notify &&
        _peer_id != zpt::IDENTITY()("_id")->string()) {
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
              .body() = { "provider", zpt::IDENTITY(), "services", zpt::REST_RESOLVER()->list() };

            zpt::make_call<zpt::rest::services_list>(zpt::REST_RESOLVER(), _hello);
#ifndef PROPAGATE_EXCEPTION
        }
        catch (std::exception const& _e) {
            zlog(_e.what(), zpt::debug)
        }
#endif
    }

    return zpt::events::finish;
}

auto zpt::rest::minion_shutdown::blocked() const -> bool { return false; }

auto zpt::rest::minion_shutdown::operator()(zpt::events::dispatcher::ptr _dispatcher
                                            [[maybe_unused]]) -> zpt::events::state {
    auto _peer_id = this->received()->headers()("X-My-ID")->string();

    if (this->received()->performative() == zpt::Notify &&
        _peer_id != zpt::IDENTITY()("_id")->string()) {
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

auto zpt::rest::minion_hello::blocked() const -> bool { return false; }

auto zpt::rest::minion_hello::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {
    if (this->received()->performative() == zpt::Post) {
        auto _minion = this->received()->body();
        if (_minion->ok()) {
            zpt::DISPATCHER() //
              ->trigger<zpt::system_event>(zpt::system_event_type::MINION_HELLO_RECEIVED, _minion);

            if (_minion("provider")->ok()) { ::add_minion(_minion); }
            else { zlog("Malformed service list: " << _minion, zpt::error); }
        }
    }

    if (this->received()->performative() == zpt::Post ||
        this->received()->performative() == zpt::Get) {
        this //
          ->to_send()
          ->status(200)
          .body() = { "provider", zpt::IDENTITY(), "services", zpt::REST_RESOLVER()->list() };
        return zpt::events::finish;
    }

    this //
      ->to_send()
      ->status(405)
      .body() = { "message", "Only POST or GET allowed to use with `/services`" };
    return zpt::events::abort;
}

auto zpt::rest::minion_state::blocked() const -> bool { return false; }

auto zpt::rest::minion_state::operator()(zpt::events::dispatcher::ptr _dispatcher [[maybe_unused]])
  -> zpt::events::state {
    if (this->received()->performative() == zpt::Post ||
        this->received()->performative() == zpt::Get) {
        this //
          ->to_send()
          ->status(200)
          .body() = { "memory",
                      zpt::json::parse_json_str(zpt::MEM_POOL().to_string()),
                      "dispatchers",
                      { zpt::array, _dispatcher->get_state(), zpt::DISPATCHER()->get_state() } };
        return zpt::events::finish;
    }

    this //
      ->to_send()
      ->status(405)
      .body() = { "message", "Only GET allowed to use with `/services`" };
    return zpt::events::abort;
}

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
    auto _upnp_host = _config("upnp")("address")->string();
    auto _upnp_port = _config("upnp")("port")->integer();
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get("upnp");

    auto _message = _transport->make_request();
    _message //
      ->performative(zpt::Notify)
      .uri(std::format("upnp://{}:{}{}", _upnp_host, _upnp_port, _path))
      .header("X-My-Location", zpt::get_default_uri())
      .header("X-My-ID", zpt::IDENTITY()("_id"));

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
