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
    zpt::json _return;
    int n = lua_gettop(_to_convert); /* number of arguments */
    lua_Number sum = 0;
    int i;
    for (i = 1; i <= n; i++) {
        if (!lua_isnumber(_to_convert, i)) {
            lua_pushstring(_to_convert, "incorrect argument");
            lua_error(_to_convert);
        }
        sum += lua_tonumber(_to_convert, i);
    }
    return zpt::undefined;
}

auto
zpt::lua::bridge::to_ref(zpt::lua::bridge::object_type _to_convert) -> zpt::json {
    return zpt::undefined;
}

auto
zpt::lua::bridge::to_object(zpt::json _to_convert) -> zpt::lua::bridge::object_type {
    return nullptr;
}

auto
zpt::lua::bridge::from_ref(zpt::json _to_convert) -> zpt::lua::bridge::object_type {
    return nullptr;
}

auto
zpt::lua::bridge::execute(zpt::json _func, zpt::json _args) -> zpt::lua::bridge::object_type {
    lua_getglobal(this->__underlying, _func->string().data());
    this->to_args(_args);
    return this->execute();
}

auto
zpt::lua::bridge::execute() -> zpt::lua::bridge::object_type {
    expect(lua_isfunction(this->__underlying, -1),
           "Lua: there is no callable item in the state",
           500,
           0);
    expect(!lua_pcall(this->__underlying, 0, LUA_MULTRET, 0),
           "Lua: error invoking function: " << lua_tostring(this->__underlying, -1),
           500,
           0);
    return this->__underlying;
}

auto
zpt::lua::bridge::to_args(zpt::json _args) -> zpt::lua::bridge& {
    expect(_args->is_array(), "Lua: `to_args` parameter `_args` must be an array", 500, 0);
    for (auto [_, __, _arg] : _args) {
        auto _lua_arg = this->to_object(_arg);
        
    }
    return (*this);
}
