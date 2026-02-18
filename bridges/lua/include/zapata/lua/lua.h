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

/**
 * @file lua.h
 * @brief Lua bridge implementation.
 *
 * Provides bidirectional integration between C++ and Lua 5.4.
 * Supports loading Lua modules, calling Lua functions from C++,
 * and registering C++ callbacks for Lua to call.
 *
 * @see zpt::lua::bridge
 * @see zpt::LUA_BRIDGE
 */

#pragma once

#include <lua.hpp>
#include <zapata/bridge.h>

namespace zpt {

/**
 * @brief RAII wrapper for lua_State.
 *
 * Manages Lua state lifetime and provides convenient access.
 */
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

/**
 * @brief Lua scripting language bridge.
 *
 * Integrates Lua 5.4 with Zapata, providing:
 * - Loading Lua scripts from files
 * - Calling Lua functions from C++
 * - Registering C++ callbacks callable from Lua
 * - Automatic JSON/Lua value conversion
 *
 * @par Example
 * @code
 * auto& lua = zpt::LUA_BRIDGE();
 * lua.add_module("/path/to/script.lua")
 *    .init();
 *
 * auto result = lua.call({ "function", "my_lua_func" },
 *                        { "arg", "hello" });
 * @endcode
 */
class bridge : public zpt::programming::bridge<zpt::lua::bridge, zpt::lua_object> {
  public:
    using underlying_type = lua_State*;  ///< Raw Lua state pointer
    using callback_type = std::function<void(underlying_type)>;  ///< C++ callback for Lua
    using lambda_type = std::function<int(underlying_type)>;     ///< Lambda as Lua C function

    bridge();
    bridge(bridge&& _rhs) = delete;
    virtual ~bridge();

    auto operator=(bridge const& _rhs) -> zpt::lua::bridge& = delete;
    auto operator=(bridge&& _rhs) -> zpt::lua::bridge& = delete;

    /** @brief Returns "lua". */
    auto name() const -> std::string;
    /** @brief Returns the raw lua_State pointer. */
    auto state() -> lua_State*;
    /** @brief Returns thread-local bridge instance. */
    auto thread_instance() -> bridge&;

    /** @brief Loads a Lua module from file. */
    auto setup_module(zpt::json _conf, std::string _external_path, bool _persist = true)
      -> zpt::lua::bridge&;
    /** @brief Registers a C++ callback as a Lua module. */
    auto setup_module(zpt::json _conf, callback_type _callback, bool _persist = true)
      -> zpt::lua::bridge&;
    /** @brief Locates a Lua value by path. */
    auto find(zpt::json _to_locate) -> object_type;

    /** @brief Clears the Lua stack. */
    auto clear_stack() -> zpt::lua::bridge&;

    /** @brief Converts Lua stack to JSON. */
    auto to_json(object_type _to_convert) -> zpt::json;
    /** @brief Converts Lua value at index to JSON. */
    auto to_json(object_type _to_convert, int _index) -> zpt::json;
    /** @brief Creates a JSON reference to a Lua value. */
    auto to_ref(object_type _to_convert, int _index = 1) -> zpt::json;
    /** @brief Pushes JSON value onto Lua stack. */
    auto to_object(zpt::json _to_convert) -> object_type;
    /** @brief Pushes JSON value using existing state. */
    auto to_object(zpt::json _to_convert, object_type _return) -> object_type;
    /** @brief Dereferences a JSON Lua reference. */
    auto from_ref(zpt::json _to_convert, object_type _return) -> object_type;

    /** @brief Executes a Lua function with arguments. */
    auto execute(zpt::json _func, zpt::json _args) -> zpt::lua::bridge::object_type;

    /** @brief Initializes the bridge (loads all modules). */
    auto initialize() -> zpt::lua::bridge&;

  private:
    lua_State* __underlying{ nullptr };
    std::map<std::string, std::tuple<callback_type, zpt::json>> __builtin_to_load;
    std::map<std::string, zpt::json> __external_to_load;

    bridge(bridge const& _rhs);
    auto execute() -> zpt::lua::bridge::object_type;
    auto to_args(zpt::json _to_convert) -> zpt::lua::bridge&;
};
} // namespace lua

/**
 * @brief Returns the global Lua bridge instance.
 * @return Reference to the thread-local Lua bridge.
 */
auto LUA_BRIDGE() -> zpt::lua::bridge&;
} // namespace zpt
