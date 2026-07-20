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

std::string _script = R"(
    local builtin2 = {}

    function builtin2.fact (n)
      local r = builtin.to_c(n)
      print("LUA: "..r.a.." "..r.b[1])

      if n == 0 then
        return 1
      else
        return n * builtin2.fact(n-1)
      end
    end

    return builtin2
)";

auto to_c(lua_State* _state) -> int {
    auto& _instance = _bridge.thread_instance();
    zpt::json _json = _instance.object_to_json(_state);
    zlog(_json, zpt::debug);
    _instance.to_object({ "a", _json, "b", { zpt::array, 1, 2, 3, 4, 10 } }, _state);
    return 1;
}

struct luaL_Reg _lib[] = { { "to_c", to_c }, { nullptr, nullptr } };

auto init_x(lua_State* _state) -> void {
    zlog("Lua: init callback called", zpt::info);
    lua_newtable(_state);
    luaL_setfuncs(_state, _lib, 0);
    lua_setglobal(_state, "builtin");
}

auto main(int, char**) -> int {
    std::filesystem::path _module =
      std::filesystem::temp_directory_path() / "zapata_test_lua_module1.lua";
    std::ofstream _ofs;
    _ofs.open(_module);
    _ofs << _script << std::flush;
    _ofs.close();

    _bridge                                        //
      .add_module(init_x, { "module", "builtin" }) //
      .add_module(_module.string(), { "module", "builtin2" });

    std::thread _thread1{ [&]() -> void {
        std::cout << "Thread1:" << std::endl << std::flush;
        zlog(_bridge.thread_instance().call(zpt::json{ "module", "builtin2", "function", "fact" },
                                            zpt::json{ zpt::array, 10 }),
             zpt::info);
        zlog(_bridge.thread_instance().call(zpt::json{ "module", "builtin", "function", "to_c" },
                                            zpt::undefined),
             zpt::info);
        zlog(
          _bridge.thread_instance().call(
            zpt::json{ "module", "builtin", "function", "to_c" },
            zpt::json{ zpt::array,
                       zpt::json{ "c", 1, "d", zpt::json{ "e", zpt::json{ zpt::array, 1, 2, 3 } } },
                       1,
                       "testing",
                       false }),
          zpt::info);
    } };

    std::thread _thread2{ [&]() -> void {
        std::cout << "Thread2:" << std::endl << std::flush;
        zlog(_bridge.thread_instance().call(zpt::json{ "module", "builtin2", "function", "fact" },
                                            zpt::json{ zpt::array, 20 }),
             zpt::info);
        zlog(_bridge.thread_instance().call(zpt::json{ "module", "builtin", "function", "to_c" },
                                            zpt::undefined),
             zpt::info);
        zlog(_bridge.thread_instance().call(zpt::json{ "module", "builtin", "function", "to_c" },
                                            zpt::json{ zpt::array, "something" }),
             zpt::info);
    } };

    _thread1.join();
    _thread2.join();
    return 0;
}
