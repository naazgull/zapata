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
 * Provides bidirectional integration between C++ and Lua.
 * Supports loading Lua modules, calling Lua functions from C++,
 * and registering C++ callbacks for Lua to call.
 *
 * @see zpt::lua::bridge
 * @see zpt::LUA_BRIDGE
 */

#pragma once

#include <lua.hpp>
#include <zapata/base/safe_access.h>
#include <zapata/bridge.h>

namespace zpt {
namespace lua {

/**
 * @brief Lua scripting language bridge.
 *
 * Integrates Lua with Zapata, providing:
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
class bridge : public zpt::programming::bridge<zpt::lua::bridge, lua_State*> {
  public:
    using underlying_type = lua_State*;                         ///< Raw Lua state pointer
    using callback_type = std::function<void(underlying_type)>; ///< C++ callback for Lua
    using lambda_type = std::function<int(underlying_type)>;    ///< Lambda as Lua C function

    /** @brief Creates a new Lua state with standard libraries open.
     * @return void (constructors implicitly initialize the object). */
    bridge();
    bridge(bridge&& _rhs) = delete;
    /** @brief Closes the Lua state if it is not null.
     * @return void (destructors implicitly clean up the object). */
    ~bridge();

    auto operator=(bridge const& _rhs) -> zpt::lua::bridge& = delete;
    auto operator=(bridge&& _rhs) -> zpt::lua::bridge& = delete;

    /** @brief Returns "lua".
     * @return The name of this bridge. */
    auto name() const -> std::string;
    /** @brief Returns the raw lua_State pointer.
     * @return Raw Lua state pointer. */
    auto state() -> lua_State*;
    /** @brief Returns thread-local bridge instance.
     * @return Reference to thread-local bridge instance. */
    auto thread_instance() -> bridge&;

    /** @brief Loads a Lua module from file.
     * @param _conf Configuration JSON.
     * @param _external_path External file path to load.
     * @param _persist Whether to persist the module.
     * @return Reference to this bridge. */
    auto setup_module(zpt::json _conf, std::string _external_path, bool _persist = true)
      -> zpt::lua::bridge&;
    /** @brief Registers a C++ callback as a Lua module.
     * @param _conf Configuration JSON.
     * @param _callback C++ callback to register.
     * @param _persist Whether to persist the module.
     * @return Reference to this bridge. */
    auto setup_module(zpt::json _conf, callback_type _callback, bool _persist = true)
      -> zpt::lua::bridge&;
    /** @brief Locates a Lua value by path.
     * @param _to_locate JSON value with path.
     * @return Lua value at the given path. */
    auto find(zpt::json _to_locate) -> object_type;

    /** @brief Clears the Lua stack.
     * @return Reference to this bridge. */
    auto clear_stack() -> zpt::lua::bridge&;

    /** @brief Converts Lua stack to JSON.
     * @param _to_convert Object to convert.
     * @return JSON representation of the stack. */
    auto to_json(object_type _to_convert) -> zpt::json;
    /** @brief Converts Lua value at index to JSON.
     * @param _to_convert Object to convert.
     * @param _index Stack index to convert.
     * @return JSON representation of the value. */
    auto to_json(object_type _to_convert, int _index) -> zpt::json;
    /** @brief Creates a JSON reference to a Lua value.
     * @param _to_convert Object to reference.
     * @param _index Stack index of the value.
     * @return JSON reference to the Lua value. */
    auto to_ref(object_type _to_convert, int _index = 1) -> zpt::json;
    /** @brief Pushes JSON value onto Lua stack.
     * @param _to_convert JSON value to push.
     * @return Lua value pushed. */
    auto to_object(zpt::json _to_convert) -> object_type;
    /** @brief Pushes JSON value using existing state.
     * @param _to_convert JSON value to push.
     * @param _return Existing state to push into.
     * @return Lua value pushed. */
    auto to_object(zpt::json _to_convert, object_type _return) -> object_type;
    /** @brief Dereferences a JSON Lua reference.
     * @param _to_convert JSON reference to dereference.
     * @param _return Target state to push into.
     * @return Lua value from the reference. */
    auto from_ref(zpt::json _to_convert, object_type _return) -> object_type;

    /** @brief Executes a Lua function with arguments.
     * @param _func Function to execute.
     * @param _args Function arguments.
     * @return Result of the function execution. */
    auto execute(zpt::json _func, zpt::json _args) -> zpt::lua::bridge::object_type;

    /** @brief Initializes the bridge (loads all modules).
     * @return Reference to this bridge. */
    auto initialize() -> zpt::lua::bridge&;
    /** @brief Clears all loaded modules and resets the bridge state.
     * @return Reference to this bridge. */
    auto cleanup() -> zpt::lua::bridge&;

  private:
    lua_State* __underlying{ nullptr }; ///< Raw Lua state
    std::map<std::string, std::tuple<callback_type, zpt::json>>
      __builtin_to_load;                                 ///< Built-in modules to register
    std::map<std::string, zpt::json> __external_to_load; ///< External file modules to load

    /** @brief Copy constructor for creating a new thread-local bridge instance.
     * @param _rhs Bridge instance to copy from. */
    bridge(bridge const& _rhs);
    /** @brief Executes the Lua function currently on top of the stack.
     * @return Result of the function execution. */
    auto execute() -> zpt::lua::bridge::object_type;
    /** @brief Pushes all JSON array elements onto the Lua stack as function arguments.
     * @param _to_convert JSON array to push elements from.
     * @return Reference to this bridge. */
    auto to_args(zpt::json _to_convert) -> zpt::lua::bridge&;
};
} // namespace lua

/**
 * @brief Retrieves the global Lua bridge instance.
 * @return Reference to the thread-local Lua bridge.
 */
auto LUA_BRIDGE() -> zpt::lua::bridge&;
/**
 * @brief Retrieves the Lua bridge global variables.
 * @return Reference to the thread-local Lua bridge.
 */
auto LUA_GLOBALS() -> zpt::safe_access<zpt::json, zpt::locks::spin_mutex>&;
} // namespace zpt
