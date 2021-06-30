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

#pragma once

#include <zapata/bridge.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

namespace zpt {
auto
LUA_BRIDGE() -> ssize_t&;

class lua_object {
  public:
    lua_object();
    lua_object(lua_State* _rhs);
    lua_object(lua_object const& _rhs);
    lua_object(lua_object&& _rhs);
    virtual ~lua_object();

    auto operator=(lua_object const& _rhs) -> lua_object&;
    auto operator=(lua_object&& _rhs) -> lua_object&;
    auto operator=(lua_State* _rhs) -> lua_object&;
    auto operator->() -> lua_State*;
    auto operator*() -> lua_State&;
    operator lua_State*();

    auto get() -> lua_State*;

  private:
    lua_State* __underlying{ nullptr };
    bool __initialized_internally{ false };
};

namespace lua {
class bridge : public zpt::programming::bridge<zpt::lua::bridge, zpt::lua_object> {
  public:
    using underlying_type = lua_State*;
    using callback_type = std::function<void(underlying_type)>;
    using lambda_type = std::function<int(underlying_type)>;

    bridge();
    virtual ~bridge();

    auto name() const -> std::string;
    auto state() -> lua_State*;

    auto setup_module(zpt::json _conf, std::string _external_path) -> zpt::lua::bridge&;
    auto setup_module(zpt::json _conf, callback_type _callback) -> zpt::lua::bridge&;

    auto to_json(object_type _to_convert) -> zpt::json;
    auto to_ref(object_type _to_convert) -> zpt::json;
    auto to_object(zpt::json _to_convert) -> object_type;
    auto from_ref(zpt::json _to_convert) -> object_type;

    auto execute(zpt::json _func, zpt::json _args) -> zpt::lua::bridge::object_type;
    auto execute() -> zpt::lua::bridge::object_type;

  private:
    lua_State* __underlying{ nullptr };

    auto to_args(zpt::json _to_convert) -> zpt::lua::bridge&;
};
} // namespace lua
} // namespace zpt
