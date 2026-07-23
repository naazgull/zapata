#include <zapata/lua/lua.h>

namespace zpt {
namespace lua {
namespace bindings {
auto make_request(lua_State* _state) -> int;
auto send_request(lua_State* _state) -> int;
auto get_config(lua_State* _state) -> int;
auto log(lua_State* _state) -> int;
auto to_json_str(lua_State* _state) -> int;
} // namespace bindings
auto register_bindings(lua_State* _state) -> void;
} // namespace lua
} // namespace zpt
