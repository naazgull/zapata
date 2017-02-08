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

#include <zapata/lisp/Bridge.h>

namespace zpt {

	namespace lisp {
		zpt::lisp::bridge* __instance = nullptr;
	}
}

zpt::lisp::Bridge::Bridge(zpt::json _options) : zpt::Bridge(_options), __self(this), __lambdas(new std::map<std::string, std::function< zpt::lisp::object (int, zpt::lisp::object[]) > >()), __modules(new std::map<std::string, std::string>()), __consistency(new std::map<std::string, std::function< bool (const std::string, const std::string) > >())  {
	char _arg[] = { 'z', 'p', 't', '\0'};
	char* _argv[] = { _arg };
	cl_boot(1, _argv);
	atexit(cl_shutdown);
}

zpt::lisp::Bridge::~Bridge() {
}

auto zpt::lisp::Bridge::name() -> std::string {
	return std::string("lisp");
}

auto zpt::lisp::Bridge::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::lisp::Bridge::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::lisp::Bridge::mutations(zpt::mutation::emitter _emitter) -> void {
}

auto zpt::lisp::Bridge::mutations() -> zpt::mutation::emitter {
	return this->__events->mutations();
}

auto zpt::lisp::Bridge::self() const -> zpt::bridge {
	return this->__self;
}

auto zpt::lisp::Bridge::eval(std::string _call) -> zpt::lisp::object {
	return zpt::lisp::object(cl_safe_eval(c_string_to_object(_call.c_str()), Cnil, Cnil));
}

auto zpt::lisp::Bridge::initialize() -> void {
	this->eval(
		"(defmacro json (&rest args)\n"
		"(let ((temphash (gensym))\n"
		"(tempkey (gensym))\n"
		"(tempval (gensym)))\n"
		"`(let ((,temphash (make-hash-table :test #'equal :size ,(/ (length args) 2))))\n"
		"(loop for (,tempkey ,tempval) on ',args by #'cddr do\n"
		"(setf (gethash (eval ,tempkey) ,temphash) (eval ,tempval)))\n"
		",temphash)))"
	);
	
	if (this->options()["rest"]["modules"]->ok()) {
		for (auto _lisp_script : this->options()["rest"]["modules"]->arr()) {
			if (_lisp_script->str().find(".lisp") != std::string::npos || _lisp_script->str().find(".fasb") != std::string::npos) {
				zlog(std::string("LISP bridge loading module '") + _lisp_script->str() + std::string("'"), zpt::notice);
				this->eval(std::string("(load \"") + ((std::string) _lisp_script) + std::string("\")"));
			}
		}
	}
	zlog(std::string("LISP bridge initialized"), zpt::info);
}

auto zpt::lisp::Bridge::defun(zpt::json _conf, cl_objectfn_fixed _fun, int _n_args) -> void {
	cl_def_c_function(c_string_to_object(_conf["name"]->str().data()), _fun, _n_args);
	this->defop(_conf);
}

auto zpt::lisp::Bridge::deflbd(zpt::json _conf, std::function< zpt::lisp::object (int, zpt::lisp::object[]) > _callback, int _n_args) -> void {
	std::string _name(_conf["name"]->str() + std::string("/") + std::to_string(_n_args));
	auto _found = this->__lambdas->find(_name);
	if (_found == this->__lambdas->end()) {
		this->__lambdas->insert(make_pair(_name, _callback));
		std::string _args_string;
		std::string _coerced_args_string;
		for (int _i = 0; _i != _n_args; _i++) {
			_args_string += std::string("arg") + std::to_string(_i) + std::string(" ");
			_coerced_args_string += std::string("arg") + std::to_string(_i) + std::string(" ");
		}
		std::string _expression = std::string("(defun ") + _conf["name"]->str() + std::string(" (") + _args_string + std::string(") (cpp-lambda-call \"") + _name + std::string("\" ") + std::to_string(_n_args) + std::string(" (make-array ") + std::to_string(_n_args) + std::string(" :initial-contents (list ") + _coerced_args_string + std::string(") ) ) )");
		this->eval(_expression);
		this->defop(_conf);
	}
}

auto zpt::lisp::Bridge::defop(zpt::json _conf) -> void {
	assertz(_conf["name"]->ok(), "the configuration attribute 'name' is required", 0, 0);
	assertz(_conf["label"]->ok(), "the configuration attribute 'label' is required", 0, 0);
	assertz(_conf["type"]->ok(), "the configuration attribute 'type' is required", 0, 0);
	assertz(_conf["access"]->ok(), "the configuration attribute 'access' is required", 0, 0);

	std::string _expression(std::string("(setf (gethash '") + _conf["name"]->str() + std::string(" *defined-operators*) '((name . \"") + _conf["name"]->str() + std::string("\") (access . \"") + _conf["access"]->str() + std::string("\") ") + (_conf["category"]->ok() ? std::string("(category . \"") + ((std::string) _conf["category"]) + std::string("\") ") : std::string("")) + (_conf["triggerable"]->ok() ? std::string("(triggerable . \"") + ((std::string) _conf["triggerable"]) + std::string("\") ") : std::string("")) + std::string("(args . ("));
	if(_conf["args"]->ok()){
		for (auto _a : _conf["args"]->arr()) {
			_expression += std::string("((type . \"") + _a["type"]->str() + std::string("\") (label . \"") + _a["label"]->str() + std::string("\")) ");
		}
	}
	_expression += std::string("))");
	if (_conf["post-state"]->ok()) {
		_expression += std::string(" (post-state . (");
		for (auto _a : _conf["post-state"]->arr()) {
			_expression += std::string("((prop . \"") + _a["prop"]->str() + std::string("\") (value . \"") + _a["value"]->str() + std::string("\"))");
		}
		_expression += std::string("))");
	}
	_expression += std::string(" (label . \"") + _conf["label"]->str() + std::string("\") (type . \"") + _conf["type"]->str() + std::string("\")))");

	this->eval(_expression);
	this->__modules->insert(make_pair(_conf["name"]->str(), this->__current));
}

auto zpt::lisp::Bridge::defchk(std::function< bool (const std::string, const std::string) > _callback) -> void {
	auto _found = this->__consistency->find(this->__current);
	if (_found == this->__consistency->end()) {
		this->__consistency->insert(make_pair(this->__current, _callback));
	}
}

auto zpt::lisp::Bridge::defmod(std::string _module) -> void {
	this->__current.assign(_module);
}

auto zpt::lisp::Bridge::call(const char* _c_name, int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
	std::string _name(_c_name);
	auto _found = this->__lambdas->find(_name);
	if (_found != this->__lambdas->end()) {
		return (_found->second)(_n_args, _args);
	}
	return zpt::lisp::object(ecl_make_bool(false));
}

auto zpt::lisp::Bridge::check(const std::string _op1, const std::string _op2) -> bool {
	if (_op1 == _op2) {
		return true;
	}
	std::string _name(_op1);
	auto _module_found = this->__modules->find(_name);
	if (_module_found != this->__modules->end()) {
		string _module = _module_found->second;
		auto _found = this->__consistency->find(_module);
		if (_found != this->__consistency->end()) {
			return (_found->second)(_op1, _op2);
		}
	}
	return true;
}

auto zpt::lisp::Bridge::instance() -> zpt::bridge {
	assertz(zpt::lisp::__instance != nullptr, "you must invoke 'zpt::bridge::boot< zpt::lisp::bridge >' before requesting the instance", 500, 0);
	return zpt::lisp::__instance->self();
}

auto zpt::lisp::Bridge::is_booted() -> bool {
	return zpt::lisp::__instance != nullptr;
}

auto zpt::lisp::Bridge::boot(zpt::json _options) -> void {
	assertz(zpt::lisp::__instance == nullptr, "bridge instance isn't null, 'zpt::bridge::boot< zpt::lisp::bridge >' already invoked", 500, 0);
	zpt::lisp::bridge* _bridge = new zpt::lisp::bridge(_options);
	zpt::lisp::__instance = _bridge;
	_bridge->eval("(defparameter *defined-operators* (make-hash-table))");
	_bridge->eval(
		"(defpackage :zpt "
		"(:use :common-lisp) "
		"(:export "
		":on "
		":route "
		":split "
		":topic-var "
		":authorize "
		"))"
	);

	zlog(std::string("LISP bridge loading basic operators (cpp-lambda-call, check-consistency, zlog, get-log-level, zpt:on, zpt:route, zpt:split, zpt:topic-var, zpt:authorize)"), zpt::info);
	_bridge->defun(
		{
			"name", "cpp-lambda-call",
			"type", "internal",
			"access", "x",
			"label", "delegate C++ lambda call",
			"args", { zpt::array,
				{ "type", "string", "label", "function name" },
				{ "type", "integer", "label", "number of arguments" },
				{ "type", "array", "label", "arguments" }
			}
		},
		(cl_objectfn_fixed) zpt::lisp::cpp_lambda_call,
		3
	);
	_bridge->defun(
		{
			"name", "check-consistency",
			"type", "internal",
			"access", "r",
			"label", "Check two function consistency",
			"args", { zpt::array,
				{ "type", "string", "label", "left hand side function name" },
				{ "type", "string", "label", "right hand side function name" }
			}
		},
		(cl_objectfn_fixed) zpt::lisp::cpp_check_call,
		2
	);
	_bridge->defun(
		{
			"name", "zlog",
			"type", "internal",
			"access", "r",
			"label", "Logging function",
			"args", { zpt::array,
				{ "type", "string", "label", "text to be logged" },
				{ "type", "int", "label", "log level (syslog levels)" }
			}
		},
		(cl_objectfn_fixed) zpt::lisp::logger,
		2
	);
	_bridge->defun(
		{
			"name", "get-log-level",
			"type", "internal",
			"access", "r",
			"label", "Get current specified log level",
			"args", zpt::json::array()
		},
		(cl_objectfn_fixed) zpt::lisp::get_log_level,
		0
	);
	_bridge->defun(
		{
			"name", "zpt:on",
			"type", "internal",
			"access", "a",
			"label", "Registers resource handler",
			"args", { zpt::array,
				{ "type", "string", "label", "request topic pattern" },
				{ "type", "object", "label", "performative <> operator hash table" },
				{ "type", "object", "label", "options" }				
			}
		},
		(cl_objectfn_fixed) zpt::lisp::on,
		3
	);	
	_bridge->defun(
		{
			"name", "zpt:route",
			"type", "internal",
			"access", "a",
			"label", "Routes RESTful resource requests",
			"args", { zpt::array,
				{ "type", "string", "label", "request performative" },
				{ "type", "string", "label", "request topic pattern" },
				{ "type", "object", "label", "payload to send" }
			}
		},
		(cl_objectfn_fixed) zpt::lisp::route,
		3
	);
	_bridge->defun(
		{
			"name", "zpt:split",
			"type", "internal",
			"access", "a",
			"label", "Splits a string by a given separator",
			"args", { zpt::array,
				{ "type", "string", "label", "the string to split" },
				{ "type", "string", "label", "the separator to split by" }
			}
		},
		(cl_objectfn_fixed) zpt::lisp::split,
		2
	);
	_bridge->defun(
		{
			"name", "zpt:topic-var",
			"type", "internal",
			"access", "a",
			"label", "Routes RESTful resource requests",
			"args", { zpt::array,
				{ "type", "array", "label", "the splited topic" },
				{ "type", "int", "label", "index for the topic part" }
			}
		},
		(cl_objectfn_fixed) zpt::lisp::topic_var,
		2
	);
	_bridge->defun(
		{
			"name", "zpt:authorize",
			"type", "internal",
			"access", "a",
			"label", "Validates authorization headers",
			"args", { zpt::array,
				{ "type", "object", "label", "the message envelope" }
			}
		},
		(cl_objectfn_fixed) zpt::lisp::validate_authorization,
		1
	);
	zlog(std::string("LISP bridge booted"), zpt::alert);
}

zpt::lisp::Object::Object(cl_object _target) : std::shared_ptr< zpt::lisp::Type >(new zpt::lisp::Type(_target)) {
}

zpt::lisp::Object::Object() : std::shared_ptr< zpt::lisp::Type >(nullptr) {
}

auto zpt::lisp::Object::bridge() -> zpt::lisp::bridge* {
	return zpt::lisp::__instance;
}

auto zpt::lisp::Object::fromjson(zpt::json _in) -> zpt::lisp::object {
	return zpt::lisp::object(zpt::lisp::to_lisp(_in, zpt::lisp::__instance));
}

zpt::lisp::Type::Type(cl_object _target) : __target(_target) {
}

zpt::lisp::Type::Type() : __target(nullptr) {
}

auto zpt::lisp::Type::tojson() -> zpt::json {
	return zpt::lisp::from_lisp(this->__target);
}

/*-------------------------------------------------------------------------\
| LISP OPERATORS                                                           |
\-------------------------------------------------------------------------*/
auto zpt::lisp::cpp_lambda_call(cl_object _fn_name, cl_object _n_args, cl_object _args) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();
	
	zpt::json _ptr = _bridge->from<zpt::lisp::object>(zpt::lisp::object(_fn_name));	
	std::string _coerced_fn_name = (std::string) _ptr;
	unsigned int _coerced_n_args = ecl_to_unsigned_integer(_n_args);
	unsigned int _coerced_args_dim = ecl_array_dimension(_args, 0);
	assertz(_coerced_n_args == _coerced_args_dim, std::string("invalid number of arguments, ") + std::to_string(_coerced_n_args) + std::string(" arguments defined and ") + std::to_string(_coerced_args_dim) + std::string(" arguments passed") , 0, 0);

	zpt::lisp::object _arr[_coerced_n_args];
	for (unsigned int _i = 0; _i != _coerced_n_args; _i++) {
		_arr[_i] = zpt::lisp::object(ecl_aref1(_args, _i));
	}
	return **((zpt::lisp::bridge*) _bridge.get())->call(_coerced_fn_name.data(), _coerced_n_args, _arr).get();
}

auto zpt::lisp::cpp_check_call(cl_object _op1_name, cl_object _op2_name) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();

	std::string _op1((char*) _op1_name->base_string.self, _op1_name->base_string.fillp);
	std::string _op2((char*) _op2_name->base_string.self, _op2_name->base_string.fillp);
	return ecl_make_bool(((zpt::lisp::bridge*) _bridge.get())->check(_op1, _op2));
}

auto zpt::lisp::logger(cl_object _text, cl_object _level) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();

	zpt::json _arg0 = _bridge->from< zpt::lisp::object >(zpt::lisp::object(_text));
	std::string _op1((std::string) _arg0);
	auto _log_level = fix(_level);
	zlog(_op1, (zpt::LogLevel) _log_level);
	return ecl_make_bool(true);
}

auto zpt::lisp::get_log_level() -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();

	if(_bridge->options()["log-level"]->ok()){
		return ecl_make_fixnum(_bridge->options()["log-level"]);
	}
	return ecl_make_bool(false);
}

auto zpt::lisp::on(cl_object _cl_topic, cl_object _cl_lambda, cl_object _cl_opts) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();

	std::string _topic = std::string(_bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_topic)));
	zpt::json _lambdas = _bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_lambda));
	zpt::json _opts = _bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_opts));
	std::map< zpt::ev::performative, zpt::ev::Handler > _handlers;

	for (auto _lambda : _lambdas->obj()) {
		zpt::ev::performative _performative = zpt::ev::from_str(_lambda.first);
		std::string _name = std::string(_lambda.second);
		_handlers.insert(
			std::make_pair(_performative,
				[ _name ] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();
					zpt::lisp::object _ret = _bridge->eval< zpt::lisp::object >(std::string("(") + _name + std::string(" \"") + zpt::ev::to_str(_performative) + std::string("\" \"") + _resource + std::string("\" `") + zpt::lisp::to_lisp_string(_envelope) + std::string(")"));
					return _bridge->from< zpt::lisp::object >(_ret);
				}
			)
		);
	}
	
	_bridge->events()->on(_topic, _handlers, _opts);
	return ecl_make_bool(true);
}

auto zpt::lisp::route(cl_object _cl_performative, cl_object _cl_topic, cl_object _cl_payload) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();

	zpt::ev::performative _performative = zpt::ev::from_str(std::string(_bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_performative))));
	std::string _topic = std::string(_bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_topic)));
	zpt::json _payload = _bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_payload));
	
	zpt::json _result = _bridge->events()->route(_performative, _topic, _payload);
	return **_bridge->to< zpt::lisp::object >(_result);
}

auto zpt::lisp::split(cl_object _cl_string, cl_object _cl_separator) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();

	std::string _string = std::string(_bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_string)));
	std::string _separator = std::string(_bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_separator)));
	
	return **_bridge->to< zpt::lisp::object >(zpt::split(_string, _separator));
}

auto zpt::lisp::topic_var(cl_object _cl_topic, cl_object _cl_index) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();

	zpt::json _topic = _bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_topic));
	size_t _index = size_t(_bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_index)));
	
	return **_bridge->to< zpt::lisp::object >(_topic[_index]);
}

auto zpt::lisp::validate_authorization(cl_object _cl_envelope) -> cl_object {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::lisp::bridge >();
	zpt::json _envelope = _bridge->from< zpt::lisp::object >(zpt::lisp::object(_cl_envelope));

	_bridge->events()->authorize(_envelope);
	
	return ecl_make_bool(true);
}
