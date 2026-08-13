#include <zapata/lua/lua.h>

namespace zpt {
namespace lua {
namespace bindings {

/**
 * @brief Creates a new HTTP request from the protocol atom on the Lua stack.
 *        Pushes the resulting request object onto the stack.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 * @throws std::runtime_error if the protocol is invalid.
 */
auto make_request(lua_State* _state) -> int;
/**
 * @brief Sends an HTTP request with the given parameters on the Lua stack.
 *        Pushes the response object onto the stack.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 * @throws std::runtime_error if the request is malformed.
 */
auto send_request(lua_State* _state) -> int;
/**
 * @brief Pushes the global Zapata configuration as a JSON table onto the Lua stack.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 */
auto get_config(lua_State* _state) -> int;
/**
 * @brief Pushes the bridge's global identified by the given parameter as a JSON table onto the Lua
 * stack.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 */
auto get_global(lua_State* _state) -> int;
/**
 * @brief Takes the given global identifier and value and add it to the bridge's globals.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 */
auto set_global(lua_State* _state) -> int;
/**
 * @brief Logs the arguments from the Lua stack using the Zapata logging system.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 */
auto log(lua_State* _state) -> int;
/**
 * @brief Parses a JSON string from the Lua stack and pushes it back as a JSON value.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 * @throws std::runtime_error if the input is not valid JSON.
 */
auto to_json_str(lua_State* _state) -> int;
/**
 * @brief Sleeps for the given seconds.
 * @param _state The Lua state.
 * @return The number of results pushed onto the stack.
 */
auto sleep(lua_State* _state) -> int;
} // namespace bindings

/**
 * @brief Registers the `zpt` Lua module with bindings for HTTP requests, config, logging, and JSON
 * conversion.
 * @param _state The Lua state to register the module on.
 * @return void
 */
auto register_bindings(lua_State* _state) -> void;
} // namespace lua
} // namespace zpt
