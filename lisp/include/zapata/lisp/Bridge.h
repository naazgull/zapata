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

#define GC_THREADS
#define _REENTRANT

#include <zapata/events.h>
#include <ossp/uuid++.hh>
#include <mutex>
#include <ecl/ecl.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	namespace lisp {
		
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
			virtual auto eval(std::string _expr) -> zpt::lisp::object;
			virtual auto initialize() -> void;
			virtual auto deflbd(zpt::json _conf, std::function< zpt::lisp::object (int, zpt::lisp::object[]) > _callback) -> void;
			virtual auto defop(zpt::json _conf) -> void;
			virtual auto defchk(std::function< bool (const std::string, const std::string) > _callback) -> void;
			virtual auto defmod(std::string _module) -> void;
			virtual auto call(const char* _c_name, int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object;
			virtual auto check(const std::string _op1, const std::string _op2) -> bool;

			static auto instance() -> zpt::bridge;
			static auto is_booted() -> bool;
			static auto boot(zpt::json _options) -> void;
				
		private:
			zpt::bridge __self;
			zpt::ev::emitter __events;
			std::shared_ptr< std::map< std::string, std::function< zpt::lisp::object (int, zpt::lisp::object[]) > > > __lambdas;
			std::shared_ptr< std::map< std::string, std::string > >__modules;
			std::shared_ptr< std::map< std::string, std::function< bool (const std::string, const std::string) > > > __consistency;
			std::string __current;

			virtual auto defun(zpt::json _conf, cl_objectfn_fixed _fun, int _n_args) -> void;
			
		};

		class Object : public std::shared_ptr< zpt::lisp::Type > {
		public:
			Object(cl_object _target);
			Object();

			static auto bridge() -> zpt::lisp::bridge*;
			static auto fromjson(zpt::json _in) -> zpt::lisp::object;
		};
		
		class Type {
		public:
			Type(cl_object _target);
			Type();

			inline cl_object operator -> () {
				return this->__target;
			}

			inline cl_object operator * () {
				return this->__target;
			}
			
			virtual auto tojson() -> zpt::json;

		private:
			cl_object __target;
		};

		extern zpt::lisp::bridge* __instance;
		
		auto cpp_lambda_call(cl_object _fn_name, cl_object _n_args, cl_object _args) -> cl_object;
		auto builtin_operators(zpt::lisp::bridge* _bridge) -> void;
		
		auto from_lisp(cl_object _in) -> zpt::json;
		auto from_lisp(cl_object _in, zpt::json& _parent) -> void;
		auto to_lisp(zpt::json _in, zpt::lisp::bridge* _bridge) -> zpt::lisp::object;
		auto to_lisp_string(zpt::json _json) -> std::string;
	}	
}
