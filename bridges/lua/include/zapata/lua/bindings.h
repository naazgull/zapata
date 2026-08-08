#include <zapata/lua/lua.h>

namespace zpt {
namespace lua {
namespace bindings {

/** @brief Creates a new HTTP request from the protocol atom on the Lua stack. Pushes the resulting
 * request object onto the stack. */
auto make_request(lua_State* _state) -> int;
/** @brief Sends an HTTP request with the given parameters on the Lua stack. Pushes the response
 * object onto the stack. */
auto send_request(lua_State* _state) -> int;
/** @brief Pushes the global Zapata configuration as a JSON table onto the Lua stack. */
auto get_config(lua_State* _state) -> int;
/** @brief Logs the arguments from the Lua stack using the Zapata logging system. */
auto log(lua_State* _state) -> int;
/** @brief Parses a JSON string from the Lua stack and pushes it back as a JSON value. */
auto to_json_str(lua_State* _state) -> int;
/** @brief Sleeps for the given seconds. */
auto sleep(lua_State* _state) -> int;
} // namespace bindings

/** @brief Registers the `zpt` Lua module with bindings for HTTP requests, config, logging, and JSON
 * conversion. */
auto register_bindings(lua_State* _state) -> void;
} // namespace lua
} // namespace zpt
