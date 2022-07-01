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

#include <zapata/lua.h>

zpt::lua::bridge _bridge;

auto
to_c(lua_State* _state) -> int {
    zlog(lua_gettop(_state), zpt::debug);
    auto& _instance = _bridge.thread_instance();
    auto _json = _instance.object_to_json(_state);
    zlog(_json, zpt::debug);
    _instance.json_to_object({ "a", _json, "b", { zpt::array, 1, 2, 3, 4, 10 } });
    return 1;
}

struct luaL_Reg _lib[] = { { "to_c", to_c } };

auto
init_x(lua_State* _state) -> void {
    zlog("Lua: init callback called", zpt::info);
    lua_newtable(_state);
    luaL_setfuncs(_state, _lib, 0);
    lua_setglobal(_state, "builtin");
}

auto
main(int argc, char* argv[]) -> int {
    _bridge                                                            //
      .add_module(init_x, { "module", "builtin" })                     //
      .add_module("/home/pf/Void/test1.lua", { "module", "builtin2" }) //
      .add_module("/home/pf/Void/test2.lua", { "module", "builtin3" });

    std::thread _thread1{ [&]() -> void {
        std::cout << "Thread1:" << std::endl << std::flush;
        zlog(_bridge.thread_instance().call(zpt::json{ "function", "to_a" },
                                            zpt::json{ zpt::array, 1, "testing", false }),
             zpt::info);
        _bridge.thread_instance().call(zpt::json{ "function", "to_b" }, zpt::undefined);
        zlog(_bridge.thread_instance().call(zpt::json{ "module", "builtin", "function", "to_c" },
                                            zpt::json{ zpt::array, 1, "testing", false }),
             zpt::info);
    } };

    std::thread _thread2{ [&]() -> void {
        std::cout << "Thread2:" << std::endl << std::flush;
        zlog(_bridge.thread_instance().call(zpt::json{ "function", "to_a" },
                                            zpt::json{ zpt::array, 1, "testing", false }),
             zpt::info);
        _bridge.thread_instance().call(zpt::json{ "function", "to_b" }, zpt::undefined);
        zlog(_bridge.thread_instance().call(zpt::json{ "module", "builtin", "function", "to_c" },
                                            zpt::json{ zpt::array, 1, "testing", false }),
             zpt::info);
    } };

    _thread1.join();
    _thread2.join();
    return 0;
}
