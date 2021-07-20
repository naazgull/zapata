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
#include <Python.h>

namespace zpt {
auto
PYTHON_BRIDGE() -> ssize_t&;

class py_object {
  public:
    py_object() = default;
    py_object(PyObject* _rhs);
    py_object(py_object const& _rhs);
    py_object(py_object&& _rhs);
    virtual ~py_object();

    auto operator=(py_object const& _rhs) -> py_object&;
    auto operator=(py_object&& _rhs) -> py_object&;
    auto operator=(PyObject* _rhs) -> py_object&;
    auto operator->() -> PyObject*;
    auto operator*() -> PyObject&;
    operator PyObject*();

    auto get() -> PyObject*;

  private:
    PyObject* __underlying{ nullptr };
};

namespace python {
class bridge : public zpt::programming::bridge<zpt::python::bridge, zpt::py_object> {
  public:
    using underlying_type = PyObject*;
    using callback_type = underlying_type (*)();
    using lambda_type = std::function<underlying_type(underlying_type, underlying_type)>;

    bridge() = default;
    virtual ~bridge();

    auto name() const -> std::string;

    auto setup_module(zpt::json _conf, std::string _external_path) -> zpt::python::bridge&;
    auto setup_module(zpt::json _conf, callback_type _callback) -> zpt::python::bridge&;

    auto find(zpt::json _to_locate) -> object_type;

    auto to_json(object_type _to_convert) -> zpt::json;
    auto to_ref(object_type _to_convert) -> zpt::json;
    auto to_object(zpt::json _to_convert) -> object_type;
    auto from_ref(zpt::json _to_convert) -> object_type;

    auto execute(zpt::json _func, zpt::json _args) -> zpt::python::bridge::object_type;
    auto execute(object_type _func, object_type _args) -> zpt::python::bridge::object_type;
    auto execute(zpt::json _self, std::string _func, std::nullptr_t _args)
      -> zpt::python::bridge::object_type;
    auto execute(object_type _self, std::string _func, std::nullptr_t _args)
      -> zpt::python::bridge::object_type;
    template<typename... Args>
    auto execute(zpt::json _self, std::string _func, Args... _arg)
      -> zpt::python::bridge::object_type;
    template<typename... Args>
    auto execute(object_type _self, std::string _func, Args... _arg)
      -> zpt::python::bridge::object_type;

    auto initialize() -> zpt::python::bridge&;
    auto is_initialized() const -> bool;

  private:
    std::atomic<bool> __initialized{ false };
    std::map<std::string, object_type> __modules;
    std::map<std::string, std::tuple<callback_type, zpt::json>> __builtin_to_load;
    std::map<std::string, zpt::json> __external_to_load;
};
} // namespace python
} // namespace zpt

template<typename... Args>
auto
zpt::python::bridge::execute(zpt::json _self, std::string _func_name, Args... _args)
  -> zpt::python::bridge::object_type {
    this->initialize();
    expect(_self->ok(), "Python: cannot call a function over a null instance", 500, 0);
    return this->execute(this->to_object(_self), _func_name, this->to_object(_args).get()...);
}

template<typename... Args>
auto
zpt::python::bridge::execute(object_type _self, std::string _func_name, Args... _args)
  -> zpt::python::bridge::object_type {
    this->initialize();
    expect(_self != nullptr, "Python: cannot call a function over a null instance", 500, 0);
    auto _func = this->to_object(_func_name);

    PyErr_Clear();
    object_type _ret = PyObject_CallMethodObjArgs(_self, _func, _args..., nullptr);

    PyObject *_py_error_type = nullptr, *_py_error = nullptr, *_traceback = nullptr;
    PyErr_Fetch(&_py_error_type, &_py_error, &_traceback);

    expect(_py_error_type == nullptr,
           "Python: error invoking callable object: " << this->to_json(_py_error),
           500,
           0);
    return _ret;
}
