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

#include <Python.h>
#include <mutex>
#include <ossp/uuid++.hh>
#include <zapata/events.h>

namespace zpt {

namespace python {
extern std::shared_ptr<std::vector<std::pair<std::string, PyObject* (*)(void)>>> __modules;
extern zpt::json __sys_path;

class Bridge;
class Type;
class Object;

typedef Object object;
typedef Bridge bridge;

class Bridge : public zpt::Bridge {
  public:
    Bridge(zpt::json _options);
    virtual ~Bridge();

    virtual auto name() -> std::string;
    virtual auto events(zpt::ev::emitter _emitter) -> void;
    virtual auto events() -> zpt::ev::emitter;
    virtual auto self() const -> zpt::bridge;
    virtual auto unbind() -> void;
    virtual auto eval(std::string const& _expr) -> zpt::python::object;
    virtual auto initialize() -> void;
    virtual auto load_module(std::string const& _module) -> void;
    virtual auto deflbd(zpt::json _conf,
                        std::function<zpt::python::object(int, zpt::python::object[])> _callback)
      -> void;

    static auto instance() -> zpt::bridge;
    static auto is_booted() -> bool;
    static auto boot(zpt::json _options) -> void;
    static auto defmdl(std::string const& _name, PyObject* (*initfunc)(void)) -> void;
    static auto add_syspath(std::string const& _name) -> void;

  private:
    zpt::bridge __self;
    zpt::ev::emitter __events;
    std::shared_ptr<
      std::map<std::string, std::function<zpt::python::object(int, zpt::python::object[])>>>
      __lambdas;
};

class Object : public std::shared_ptr<zpt::python::Type> {
  public:
    Object(PyObject* _target);
    Object();

    static auto bridge() -> zpt::python::bridge*;
    static auto fromjson(zpt::json _in) -> zpt::python::object;
};

class Type {
  public:
    Type(PyObject* _target);
    Type();
    virtual ~Type();

    inline PyObject* operator->() { return this->__target; }

    inline PyObject* operator*() { return this->__target; }

    virtual auto tojson() -> zpt::json;

  private:
    PyObject* __target;
};

extern zpt::python::bridge* __instance;

auto
from_python(PyObject* _in) -> zpt::json;
auto
from_python(PyObject* _in, zpt::json& _parent) -> void;
auto
from_ref(zpt::json _in) -> PyObject*;
auto
to_ref(PyObject* _in) -> zpt::json;
auto
to_python(zpt::json _in, zpt::python::bridge* _bridge) -> zpt::python::object;
auto
to_python(zpt::json _in) -> PyObject*;

namespace module {
auto
init() -> PyObject*;
auto
on(PyObject* _self, PyObject* _args) -> PyObject*;
auto
route(PyObject* _self, PyObject* _args) -> PyObject*;
auto
route(PyObject* _self, zpt::json _params) -> PyObject*;
auto
reply(PyObject* _self, PyObject* _args) -> PyObject*;
auto
validate_authorization(PyObject* _self, PyObject* _args) -> PyObject*;
auto
options(PyObject* _self, PyObject* _args) -> PyObject*;
auto
hook(PyObject* _self, PyObject* _args) -> PyObject*;
auto
log(PyObject* _self, PyObject* _args) -> PyObject*;
auto
assertion(PyObject* _self, PyObject* _args) -> PyObject*;
auto
path_join(PyObject* _self, PyObject* _args) -> PyObject*;
auto
authorization_headers(PyObject* _self, PyObject* _args) -> PyObject*;
auto
merge(PyObject* _self, PyObject* _args) -> PyObject*;

extern PyMethodDef methods[];
extern PyModuleDef spec;
} // namespace module
} // namespace python
} // namespace zpt
