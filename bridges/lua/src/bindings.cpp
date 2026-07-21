#include <zapata/lua/bindings.h>
#include <zapata/rest.h>
#include <zapata/transport/engine.h>

namespace {
struct luaL_Reg _lib[] = { { "make_request", zpt::lua::bindings::make_request },
                           { "call", zpt::lua::bindings::send_request },
                           { "config", zpt::lua::bindings::get_config },
                           { "log", zpt::lua::bindings::log },
                           { "to_json", zpt::lua::bindings::to_json_str },
                           { nullptr, nullptr } };
}

auto zpt::lua::bindings::make_request(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    auto _request = zpt::TRANSPORT_LAYER() //
                      .get(_args->string())
                      ->make_request();
    _bridge.to_object({ "protocol", _args, "headers", _request->headers(), "uri", _request->uri() },
                      _state);
    return 1;
}

auto zpt::lua::bindings::send_request(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    auto _request = zpt::TRANSPORT_LAYER() //
                      .get(_args("protocol")->string())
                      ->make_request();
    _request //
      ->performative(zpt::ontology::from_str(_args("performative")->string()))
      .uri(_args("uri"))
      .headers() = _args("headers");

    if (_args("body")->ok()) { _request->body() = _args("body"); }

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
            _bridge.to_object({ "status", 408 }, _state);
            return 0;
        }
        std::this_thread::sleep_for(std::chrono::microseconds{ 100 });
    }

    auto _reply = _context->reply();
    _bridge.to_object({ "status", //
                        _reply->status(),
                        "headers",
                        _reply->headers(),
                        "body",
                        _reply->body() },
                      _state);
    return 1;
}

auto zpt::lua::bindings::get_config(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    _bridge.to_object(zpt::GLOBAL_CONFIG(), _state);
    return 1;
}

auto zpt::lua::bindings::log(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);

    if (_args->type() == zpt::JSArray) {
        std::ostringstream _oss;
        for (auto&& [_, __, _value] : _args) { _oss << static_cast<std::string>(_value); }
        _oss << std::flush;
        zlog(_oss.str(), zpt::info);
    }
    else { zlog(static_cast<std::string>(_args), zpt::info); }

    return 1;
}

auto zpt::lua::bindings::to_json_str(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    _bridge.json_to_object(static_cast<std::string>(_args));
    return 1;
}

auto zpt::lua::register_bindings(lua_State* _state) -> void {
    lua_newtable(_state);
    luaL_setfuncs(_state, _lib, 0);
    lua_setglobal(_state, "zpt");
}
