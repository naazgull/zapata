#include <zapata/lua/bindings.h>
#include <zapata/rest.h>
#include <zapata/transport/engine.h>

namespace {
struct luaL_Reg _lib[] = { { "make_request", zpt::lua::bindings::make_request },
                           { "call", zpt::lua::bindings::send_request },
                           { "config", zpt::lua::bindings::get_config },
                           { "get_global", zpt::lua::bindings::get_global },
                           { "set_global", zpt::lua::bindings::set_global },
                           { "log", zpt::lua::bindings::log },
                           { "to_json", zpt::lua::bindings::to_json_str },
                           { "sleep", zpt::lua::bindings::sleep },
                           { nullptr, nullptr } };
}

auto zpt::lua::bindings::make_request(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    expect(_args->is_string(), "1st parameter of `zpt.make_request` isn't a string");
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
    expect(_args->is_object(), "1st parameter of `zpt.call` isn't an object");
    auto _transport = zpt::TRANSPORT_LAYER() //
                        .get(_args("protocol")->string());

    auto _request = _transport->make_request();
    _request //
      ->performative(zpt::ontology::from_str(_args("performative")->string()))
      .uri(_args("uri"))
      .headers() = _args("headers");

    if (_args("body")->ok()) { _request->body() = _args("body"); }

    if (_transport->has_capability(zpt::transport_capability::SYNCHRONOUS)) {
        auto _context = zpt::make_call(zpt::REST_RESOLVER(), _request);
        static constexpr long long _timeout{ 20 };
        auto _start = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count();
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
    else {
        zpt::make_call(zpt::REST_RESOLVER(), _request);
        return 0;
    }
}

auto zpt::lua::bindings::get_config(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    _bridge.to_object(zpt::GLOBAL_CONFIG(), _state);
    return 1;
}

auto zpt::lua::bindings::get_global(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    expect(_args->is_string(), "1st parameter of `zpt.get_global` isn't a string");
    auto& _global = zpt::LUA_GLOBALS();
    std::shared_lock _guard{ _global.mutex() };
    _bridge.to_object((*_global)->get_path(_args->string()), _state);
    return 1;
}

auto zpt::lua::bindings::set_global(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    expect(_args->is_array() && _args->size() == 2, "`zpt.set_global` expects 2 parameters");
    expect(_args(0)->is_string(), "1st parameter of `zpt.set_global` isn't a string");
    auto& _global = zpt::LUA_GLOBALS();
    std::unique_lock _guard{ _global.mutex() };
    (*_global)->set_path(_args(0)->string(), _args(1));
    return 0;
}

auto zpt::lua::bindings::log(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);

    if (_args->type() == zpt::JSArray) {
        std::ostringstream _oss;
        for (auto&& [_, __, _value] : _args) { _oss << static_cast<std::string>(_value); }
        _oss << std::flush;
        zlog(_oss.str(), static_cast<zpt::LogLevel>(zpt::log_lvl));
    }
    else { zlog(static_cast<std::string>(_args), static_cast<zpt::LogLevel>(zpt::log_lvl)); }

    return 0;
}

auto zpt::lua::bindings::to_json_str(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    _bridge.json_to_object(static_cast<std::string>(_args));
    return 1;
}

auto zpt::lua::bindings::sleep(lua_State* _state) -> int {
    auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
    auto _args = _bridge.object_to_json(_state);
    expect(_args->is_number(), "1st parameter of `zpt.sleep` isn't a number");
    std::this_thread::sleep_for(
      std::chrono::duration<double, std::milli>{ static_cast<double>(_args) * 1000 });
    return 0;
}

auto zpt::lua::register_bindings(lua_State* _state) -> void {
    lua_newtable(_state);
    luaL_setfuncs(_state, _lib, 0);
    lua_setglobal(_state, "zpt");
}
