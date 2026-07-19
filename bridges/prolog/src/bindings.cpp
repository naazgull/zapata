#include <zapata/prolog/bindings.h>
#include <zapata/rest.h>
#include <zapata/transport/engine.h>

namespace {
extern "C" {
static auto make_request(term_t _protocol_pl /*+*/, term_t _request_pl /*-*/) -> foreign_t {
    auto& _bridge = zpt::PROLOG_BRIDGE().thread_instance();
    auto _protocol = zpt::prolog::to_json(_protocol_pl);
    auto _request_js = zpt::TRANSPORT_LAYER() //
                         .get(_protocol->string())
                         ->make_request();
    auto _request = _bridge.json_to_object(
      { "protocol", _protocol, "headers", _request_js->headers(), "uri", _request_js->uri() });
    return PL_unify_term(_request_pl, PL_TERM, *_request);
}

static auto send_request(term_t _request_pl /*+*/, term_t _reply_pl /*-*/) -> foreign_t {
    auto& _bridge = zpt::PROLOG_BRIDGE().thread_instance();
    auto _request_js = zpt::prolog::to_json(_request_pl);
    auto _request = zpt::TRANSPORT_LAYER() //
                      .get(_request_js("protocol")->string())
                      ->make_request();
    _request //
      ->performative(zpt::ontology::from_str(_request_js("performative")->string()))
      .uri(_request_js("uri"))
      .headers() = _request_js("headers");

    if (_request_js("body")->ok()) { _request->body() = _request_js("body"); }

    static constexpr long long _timeout{ 20 };
    auto _start = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
    auto _context = zpt::make_call<>(zpt::REST_RESOLVER(), _request);
    while (_context->state() <= zpt::CALL_STATE_SENT) {
        auto _lap = std::chrono::duration_cast<std::chrono::seconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count();
        if (_lap - _start > _timeout) {
            auto _reply = _bridge.json_to_object({ "status", 408 });
            return PL_unify_term(_reply_pl, PL_TERM, *_reply);
        }
        std::this_thread::sleep_for(std::chrono::microseconds{ 100 });
    }

    auto _reply_js = _context->reply();
    auto _reply = _bridge.json_to_object({ "status", //
                                           _reply_js->status(),
                                           "headers",
                                           _reply_js->headers(),
                                           "body",
                                           _reply_js->body() });
    return PL_unify_term(_reply_pl, PL_TERM, *_reply);
}

static auto get_config(term_t _config_pl /*-*/) -> foreign_t {
    auto& _bridge = zpt::PROLOG_BRIDGE().thread_instance();
    auto _config = _bridge.json_to_object(zpt::GLOBAL_CONFIG()->size() != 0
                                            ? zpt::GLOBAL_CONFIG()
                                            : zpt::json{ "prolog", { "lib", "SWI Prolog" } });
    return PL_unify_term(_config_pl, PL_TERM, *_config);
}

static auto send_to_log(term_t _to_log /*+*/) -> foreign_t {
    auto& _bridge = zpt::PROLOG_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_to_log);
    if (_args->type() == zpt::JSArray) {
        std::ostringstream _oss;
        for (auto&& [_, __, _value] : _args) { _oss << static_cast<std::string>(_value); }
        _oss << std::flush;
        zlog(_oss.str(), zpt::info);
    }
    else { zlog(static_cast<std::string>(_args), zpt::info); }
    return 1;
}
}
} // namespace

extern "C" auto install_libzapata_bridge_prolog_bindings() -> install_t {
    PL_register_foreign("zpt_make_request", 2, (void*)::make_request, 0);
    PL_register_foreign("zpt_call", 2, (void*)::send_request, 0);
    PL_register_foreign("zpt_config", 1, (void*)::get_config, 0);
    PL_register_foreign("zpt_log", 1, (void*)::send_to_log, 0);
}
