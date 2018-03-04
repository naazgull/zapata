/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <zapata/events.h>
#include <ossp/uuid++.hh>
#include <mutex>
#include <Lua.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

namespace lua {
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
	virtual auto eval(std::string _expr) -> zpt::lua::object;
	virtual auto initialize() -> void;
	virtual auto load_module(std::string _module) -> void;
	virtual auto deflbd(zpt::json _conf, std::function<zpt::lua::object(int, zpt::lua::object[])> _callback)
	    -> void;

	static auto instance() -> zpt::bridge;
	static auto is_booted() -> bool;
	static auto boot(zpt::json _options) -> void;
	static auto defmdl(std::string _name, PyObject* (*initfunc)(void)) -> void;
	static auto add_syspath(std::string _name) -> void;

      private:
	zpt::bridge __self;
	zpt::ev::emitter __events;
	std::shared_ptr<std::map<std::string, std::function<zpt::lua::object(int, zpt::lua::object[])>>>
	    __lambdas;
};

class Object : public std::shared_ptr<zpt::lua::Type> {
      public:
	Object(PyObject* _target);
	Object();

	static auto bridge() -> zpt::lua::bridge*;
	static auto fromjson(zpt::json _in) -> zpt::lua::object;
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

extern zpt::lua::bridge* __instance;

auto from_lua(PyObject* _in) -> zpt::json;
auto from_lua(PyObject* _in, zpt::json& _parent) -> void;
auto from_ref(zpt::json _in) -> PyObject*;
auto to_ref(PyObject* _in) -> zpt::json;
auto to_lua(zpt::json _in, zpt::lua::bridge* _bridge) -> zpt::lua::object;
auto to_lua(zpt::json _in) -> PyObject*;

namespace module {
auto init() -> PyObject*;
auto on(PyObject* _self, PyObject* _args) -> PyObject*;
auto route(PyObject* _self, PyObject* _args) -> PyObject*;
auto route(PyObject* _self, zpt::json _params) -> PyObject*;
auto reply(PyObject* _self, PyObject* _args) -> PyObject*;
auto validate_authorization(PyObject* _self, PyObject* _args) -> PyObject*;
auto options(PyObject* _self, PyObject* _args) -> PyObject*;
auto hook(PyObject* _self, PyObject* _args) -> PyObject*;
auto log(PyObject* _self, PyObject* _args) -> PyObject*;
auto assertion(PyObject* _self, PyObject* _args) -> PyObject*;
auto path_join(PyObject* _self, PyObject* _args) -> PyObject*;
auto authorization_headers(PyObject* _self, PyObject* _args) -> PyObject*;
auto merge(PyObject* _self, PyObject* _args) -> PyObject*;

extern PyMethodDef methods[];
extern PyModuleDef spec;
}
}
}
