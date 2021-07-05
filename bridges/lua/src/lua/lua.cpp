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

#include <zapata/lua/lua.h>

auto
zpt::LUA_BRIDGE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::lua_object::lua_object()
  : __underlying{ luaL_newstate() }
  , __initialized_internally{ true } {
    expect(this->__underlying != nullptr, "Lua: couldn't allocate new `lua_State`", 500, 0);
}

zpt::lua_object::lua_object(lua_State* _rhs)
  : __underlying{ _rhs } {}

zpt::lua_object::lua_object(lua_object const& _rhs)
  : __underlying{ _rhs.__underlying } {}

zpt::lua_object::lua_object(lua_object&& _rhs)
  : __underlying{ _rhs.__underlying } {
    _rhs.__underlying = nullptr;
}

zpt::lua_object::~lua_object() {
    if (this->__initialized_internally) { lua_close(this->__underlying); }
    this->__underlying = nullptr;
}

auto
zpt::lua_object::operator=(lua_object const& _rhs) -> lua_object& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::lua_object::operator=(lua_object&& _rhs) -> lua_object& {
    this->__underlying = _rhs.__underlying;
    _rhs.__underlying = nullptr;
    return (*this);
}

auto
zpt::lua_object::operator=(lua_State* _rhs) -> lua_object& {
    this->__underlying = _rhs;
    return (*this);
}

auto
zpt::lua_object::operator->() -> lua_State* {
    return this->__underlying;
}

auto
zpt::lua_object::operator*() -> lua_State& {
    return *this->__underlying;
}

zpt::lua_object::operator lua_State*() { return this->__underlying; }

auto
zpt::lua_object::get() -> lua_State* {
    return this->__underlying;
}

zpt::lua::bridge::bridge()
  : __underlying{ luaL_newstate() } {
    luaL_openlibs(this->__underlying);
}

zpt::lua::bridge::~bridge() { lua_close(this->__underlying); }

auto
zpt::lua::bridge::name() const -> std::string {
    return "lua";
}

auto
zpt::lua::bridge::state() -> lua_State* {
    return this->__underlying;
}

auto
zpt::lua::bridge::setup_module(zpt::json _conf, std::string _external_path) -> zpt::lua::bridge& {
    expect(!luaL_loadfile(this->__underlying, _external_path.data()),
           "Lua: error loading module '" << _external_path
                                         << "': " << lua_tostring(this->__underlying, -1),
           500,
           0);
    expect(!lua_pcall(this->__underlying, 0, LUA_MULTRET, 0),
           "Lua: error invoking function: " << lua_tostring(this->__underlying, -1),
           500,
           0);
    zlog("Lua: loading " << _external_path, zpt::info);
    return (*this);
}

auto
zpt::lua::bridge::setup_module(zpt::json _conf, callback_type _callback) -> zpt::lua::bridge& {
    _callback(this->__underlying);
    return (*this);
}

auto
zpt::lua::bridge::to_json(zpt::lua::bridge::object_type _to_convert) -> zpt::json {
    if (_to_convert.get() == nullptr) { return zpt::undefined; }

    int _size = lua_gettop(_to_convert);
    if (_size == 0) { return zpt::undefined; }
    if (_size == 1) { return this->to_json(_to_convert, 1); }

    zpt::json _return = zpt::json::array();
    for (int _idx = 0; _idx != _size; ++_idx) { _return << this->to_json(_to_convert, _idx + 1); }

    if (_to_convert.get() == this->__underlying) { lua_pop(_to_convert, _size); }
    return _return;
}

auto
zpt::lua::bridge::to_json(zpt::lua::bridge::object_type _to_convert, int _index) -> zpt::json {
    switch (lua_type(_to_convert, _index)) {
        case LUA_TNIL: {
            return zpt::undefined;
        }
        case LUA_TNUMBER: {
            return lua_tonumber(_to_convert, _index);
        }
        case LUA_TBOOLEAN: {
            return static_cast<bool>(lua_toboolean(_to_convert, _index));
        }
        case LUA_TSTRING: {
            return lua_tostring(_to_convert, _index);
        }
        case LUA_TTABLE: {
            zpt::json _return;
            lua_pushnil(_to_convert);
            while (lua_next(_to_convert, _index) != 0) {
                if (lua_type(_to_convert, -2) == LUA_TSTRING) { // The table key type is string
                    if (!_return->ok()) { _return = zpt::json::object(); }
                    std::string _key{ lua_tostring(_to_convert, -2) };
                    _return << _key << this->to_json(_to_convert, lua_gettop(_to_convert));
                }
                else { // Otherwise, assume array
                    if (!_return->ok()) { _return = zpt::json::array(); }
                    _return << this->to_json(_to_convert, lua_gettop(_to_convert));
                }
                lua_pop(_to_convert, 1);
            }
            return _return;
        }
        case LUA_TFUNCTION: {
            std::ostringstream _oss;
            _oss << std::hex << lua_tocfunction(_to_convert, _index) << std::flush;
            return zpt::json::lambda(_oss.str(), 0);
        }
        case LUA_TLIGHTUSERDATA:
        case LUA_TUSERDATA: {
            return this->to_ref(_to_convert, _index);
        }
        case LUA_TTHREAD: {
            expect(lua_type(_to_convert, _index) != LUA_TTHREAD,
                   "Lua: unmanaged lua type LUA_TTHREAD",
                   500,
                   0);
        }
    }
    return zpt::undefined;
}

auto
zpt::lua::bridge::to_ref(zpt::lua::bridge::object_type _to_convert, int _index) -> zpt::json {
    if (_to_convert == nullptr) { return zpt::undefined; }
    std::ostringstream _oss;
    _oss << std::hex << lua_touserdata(_to_convert, _index) << std::flush;
    return zpt::json::string(std::string("ref(") + _oss.str() + std::string(")"));
}

auto
zpt::lua::bridge::to_object(zpt::json _to_convert, object_type _return)
  -> zpt::lua::bridge::object_type {
    switch (_to_convert->type()) {
        case zpt::JSObject: {
            lua_newtable(_return);
            int _index = lua_gettop(_return);
            for (auto [_, _key, _value] : _to_convert) {
                lua_pushstring(_return, _key.data());
                this->to_object(_value, _return);
                lua_settable(_return, _index);
            }
            break;
        }
        case zpt::JSArray: {
            lua_newtable(_return);
            int _index = lua_gettop(_return);
            for (auto [_idx, _, _value] : _to_convert) {
                lua_pushinteger(_return, _idx);
                this->to_object(_value, _return);
                lua_settable(_return, _index);
            }
            break;
        }
        case zpt::JSString: {
            if (_to_convert->string().find("ref(") == 0) { this->from_ref(_to_convert, _return); }
            else {
                lua_pushstring(_return, _to_convert->string().data());
            }
            break;
        }
        case zpt::JSDate: {
            lua_pushinteger(_return, (zpt::timestamp_t)_to_convert);
            break;
        }
        case zpt::JSBoolean: {
            lua_pushboolean(_return, _to_convert->boolean());
            break;
        }
        case zpt::JSInteger: {
            lua_pushinteger(_return, _to_convert->integer());
            break;
        }
        case zpt::JSDouble: {
            lua_pushnumber(_return, _to_convert->floating());
            break;
        }
        case zpt::JSLambda: {
            break;
        }
        case zpt::JSNil: {
            lua_pushnil(_return);
            break;
        }
        default: {
            break;
        }
    }
    return _return;
}

auto
zpt::lua::bridge::from_ref(zpt::json _to_convert, object_type _return)
  -> zpt::lua::bridge::object_type {
    unsigned long _ref{ 0 };
    if (_to_convert->is_lambda()) {
        std::istringstream _iss;
        _iss.str(_to_convert->lambda()->name());
        _iss >> std::hex >> _ref;
    }
    else {
        std::string _s_ref = std::string(_to_convert);
        zpt::replace(_s_ref, "ref(", "");
        zpt::replace(_s_ref, ")", "");
        std::istringstream _iss;
        _iss.str(_s_ref);
        _iss >> std::hex >> _ref;
    }
    lua_pushlightuserdata(_return, reinterpret_cast<void*>(_ref));
    return _return;
}

auto
zpt::lua::bridge::execute(zpt::json _func, zpt::json _args) -> zpt::lua::bridge::object_type {
    lua_pop(this->__underlying, lua_gettop(this->__underlying));
    lua_getglobal(this->__underlying, _func->string().data());
    if (_args->is_array()) { this->to_args(_args); }
    return this->execute();
}

auto
zpt::lua::bridge::execute() -> zpt::lua::bridge::object_type {
    expect(lua_isfunction(this->__underlying, -lua_gettop(this->__underlying)),
           "Lua: there is no callable item in the stack",
           500,
           0);
    expect(!lua_pcall(this->__underlying, lua_gettop(this->__underlying) - 1, LUA_MULTRET, 0),
           "Lua: error invoking function: " << lua_tostring(this->__underlying, -1),
           500,
           0);
    return this->__underlying;
}

auto
zpt::lua::bridge::to_args(zpt::json _args) -> zpt::lua::bridge& {
    expect(_args->is_array(), "Lua: `to_args` parameter `_args` must be an array", 500, 0);
    for (auto [_, __, _arg] : _args) { this->to_object(_arg, this->__underlying); }
    return (*this);
}
