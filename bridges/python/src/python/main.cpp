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

#include <zapata/python.h>

zpt::python::bridge _bridge;

auto
to_a(PyObject* _self, PyObject* _args) -> PyObject* {
    auto _json = _bridge.object_to_json(_args);
    auto _ret = _bridge.json_to_object({ "a", _json[0] });
    return _ret;
}

PyMethodDef methods[] = { { "to_a", to_a, METH_VARARGS, "To a." },
                          { nullptr, nullptr, 0, nullptr } };
PyModuleDef spec = { PyModuleDef_HEAD_INIT, "builtin", nullptr, -1, methods };

auto
init_module_x() -> zpt::python::bridge::underlying_type {
    return PyModule_Create(&spec);
}

auto
main(int argc, char* argv[]) -> int {
    _bridge                                                          //
      .set_options({ "sys_path", { zpt::array, "/home/pf/Void/" } }) //
      .add_module(init_module_x, { "name", "builtin" })              //
      .add_module("py_external", { "name", "external" })             //
      .init();

    std::thread _thread1{ [&]() -> void {
        auto _result = _bridge.call(zpt::json{ "module", "builtin", "function", "to_a" },
                                    zpt::json{ zpt::array, { "field", "xpto" } });
        zlog(_result, zpt::info);

        auto _instance =
          _bridge.call(zpt::json{ "module", "external", "function", "to_b" }, zpt::undefined);
        _bridge.call(_instance, "set", zpt::json{ "field", "xpto" });
        _result = _bridge.call(_instance, "get", nullptr);
        zlog(_result, zpt::info);
    } };

    std::thread _thread2{ [&]() -> void {
        auto _result = _bridge.call(zpt::json{ "module", "builtin", "function", "to_a" },
                                    zpt::json{ zpt::array, { "field", "xpto" } });
        zlog(_result, zpt::info);

        auto _instance =
          _bridge.call(zpt::json{ "module", "external", "function", "to_b" }, zpt::undefined);
        _bridge.call(_instance, "set", zpt::json{ "field", "xpto" });
        _result = _bridge.call(_instance, "get", nullptr);
        zlog(_result, zpt::info);
    } };

    _thread1.join();
    _thread2.join();
    return 0;
}
