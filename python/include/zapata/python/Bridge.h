/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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
#include <Python.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	namespace python {
		
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
			virtual auto mutations(zpt::mutation::emitter _emitter) -> void;
			virtual auto mutations() -> zpt::mutation::emitter;
			virtual auto self() const -> zpt::bridge;
			virtual auto unbind() -> void;
			virtual auto eval(std::string _expr) -> zpt::python::object;
			virtual auto initialize() -> void;
			virtual auto deflbd(zpt::json _conf, std::function< zpt::python::object (int, zpt::python::object[]) > _callback, int _n_args) -> void;
			virtual auto defop(zpt::json _conf) -> void;
			virtual auto call(const char* _c_name, int _n_args, zpt::python::object _args[]) -> zpt::python::object;

			static auto instance() -> zpt::bridge;
			static auto is_booted() -> bool;
			static auto boot(zpt::json _options) -> void;
				
		private:
			zpt::bridge __self;
			zpt::ev::emitter __events;
			std::shared_ptr< std::map< std::string, std::function< zpt::python::object (int, zpt::python::object[]) > > > __lambdas;
			std::shared_ptr< std::map< std::string, PyObject* > >__modules;
		};

		class Object : public std::shared_ptr< zpt::python::Type > {
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

			inline PyObject* operator -> () {
				return this->__target;
			}

			inline PyObject* operator * () {
				return this->__target;
			}
			
			virtual auto tojson() -> zpt::json;

		private:
			PyObject* __target;
		};

		extern zpt::python::bridge* __instance;
		
		auto from_python(PyObject* _in) -> zpt::json;
		auto from_python(PyObject* _in, zpt::json& _parent) -> void;
		auto to_python(zpt::json _in, zpt::python::bridge* _bridge) -> zpt::python::object;
		auto to_python_string(zpt::json _json) -> std::string;

		namespace module {
			auto init() -> PyObject*;
			auto on(PyObject* _self, PyObject* _args) -> PyObject*;
			auto route(PyObject* _self, PyObject* _args) -> PyObject*;
			auto split(PyObject* _self, PyObject* _args) -> PyObject*;
			auto topic_var(PyObject* _self, PyObject* _args) -> PyObject*;
			auto validate_authorization(PyObject* _self, PyObject* _args) -> PyObject*;
			
			extern PyMethodDef methods[];
			extern PyModuleDef spec;
		}
	}	
}
