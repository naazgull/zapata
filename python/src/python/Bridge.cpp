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

#include <zapata/python/Bridge.h>

namespace zpt {

	namespace python {
		zpt::python::bridge* __instance = nullptr;
	}
}

zpt::python::Bridge::Bridge(zpt::json _options) : zpt::Bridge(_options), __self(this), __lambdas(new std::map< std::string, std::function< zpt::python::object (int, zpt::python::object[]) > >()), __modules(new std::map< std::string, PyObject* >()) {
}

zpt::python::Bridge::~Bridge() {
	for (auto _module : (*this->__modules)) {
		Py_DECREF(_module.second);
	}
}

auto zpt::python::Bridge::name() -> std::string {
	return std::string("python");
}

auto zpt::python::Bridge::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::python::Bridge::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::python::Bridge::mutations(zpt::mutation::emitter _emitter) -> void {
}

auto zpt::python::Bridge::mutations() -> zpt::mutation::emitter {
	return this->__events->mutations();
}

auto zpt::python::Bridge::self() const -> zpt::bridge {
	return this->__self;
}

auto zpt::python::Bridge::unbind() -> void {
	this->__self.reset();
}

auto zpt::python::Bridge::eval(std::string _call) -> zpt::python::object {
	return zpt::python::object();
}

auto zpt::python::Bridge::initialize() -> void {
	if (this->options()["rest"]["modules"]->ok()) {
		for (auto _python_script : this->options()["rest"]["modules"]->arr()) {
			if (_python_script->str().find(".py") != std::string::npos) {
				zlog(std::string("PYTHON bridge loading module '") + _python_script->str() + std::string("'"), zpt::notice);
				PyObject* _name = PyUnicode_DecodeFSDefault(_python_script->str().data());				
				PyObject* _module = PyImport_Import(_name);
				Py_DECREF(_name);
				this->__modules->insert(std::make_pair(_python_script->str(), _module));
			}
		}
	}
	zlog(std::string("PYTHON bridge initialized"), zpt::trace);
}

auto zpt::python::Bridge::deflbd(zpt::json _conf, std::function< zpt::python::object (int, zpt::python::object[]) > _callback) -> void {
	size_t _n_args = _conf["args"]->type() != zpt::JSArray ? 0 : _conf["args"]->arr()->size();
	std::string _name(_conf["name"]->str() + std::string("/") + std::to_string(_n_args));
	auto _found = this->__lambdas->find(_name);
	if (_found == this->__lambdas->end()) {
		this->__lambdas->insert(std::make_pair(_name, _callback));
	}
}

auto zpt::python::Bridge::defop(zpt::json _conf) -> void {
}

auto zpt::python::Bridge::call(const char* _c_name, int _n_args, zpt::python::object _args[]) -> zpt::python::object {
	std::string _name(_c_name);
	auto _found = this->__lambdas->find(_name);
	if (_found != this->__lambdas->end()) {
		return (_found->second)(_n_args, _args);
	}
	return zpt::python::object();
}

auto zpt::python::Bridge::instance() -> zpt::bridge {
	assertz(zpt::python::__instance != nullptr, "you must invoke 'zpt::bridge::boot< zpt::python::bridge >' before requesting the instance", 500, 0);
	return zpt::python::__instance->self();
}

auto zpt::python::Bridge::is_booted() -> bool {
	return zpt::python::__instance != nullptr;
}

auto zpt::python::Bridge::boot(zpt::json _options) -> void {
	assertz(zpt::python::__instance == nullptr, "bridge instance isn't null, 'zpt::bridge::boot< zpt::python::bridge >' already invoked", 500, 0);
	zpt::python::bridge* _bridge = new zpt::python::bridge(_options);
	zpt::python::__instance = _bridge;

	zlog(std::string("PYTHON bridge loading basic module (zpt.on, zpt.route, zpt.slipt, zpt.topic_var, zpt.authorize)"), zpt::trace);
	PyImport_AppendInittab("zpt", &zpt::python::module::init);
	Py_Initialize();
	
	zlog(std::string("PYTHON bridge booted"), zpt::warning);
}

zpt::python::Object::Object(PyObject* _target) : std::shared_ptr< zpt::python::Type >(new zpt::python::Type(_target)) {
}

zpt::python::Object::Object() : std::shared_ptr< zpt::python::Type >(nullptr) {
}

auto zpt::python::Object::bridge() -> zpt::python::bridge* {
	return zpt::python::__instance;
}

auto zpt::python::Object::fromjson(zpt::json _in) -> zpt::python::object {
	return zpt::python::object(zpt::python::to_python(_in, zpt::python::__instance));
}

zpt::python::Type::Type(PyObject* _target) : __target(_target) {
}

zpt::python::Type::Type() : __target(nullptr) {
}

zpt::python::Type::~Type() {
	if (this->__target != nullptr) {
		Py_DECREF(this->__target);
	}
}

auto zpt::python::Type::tojson() -> zpt::json {
	return zpt::python::from_python(this->__target);
}

/*-------------------------------------------------------------------------\
| PYTHON MODULE                                                           |
\-------------------------------------------------------------------------*/
auto zpt::python::module::init() -> PyObject* {
	return PyModule_Create(&zpt::python::module::spec);
}

auto zpt::python::module::on(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();

	std::string _topic;
	zpt::json _lambdas;
	std::map< zpt::ev::performative, zpt::ev::Handler > _handlers;

	for (auto _lambda : _lambdas->obj()) {
		zpt::ev::performative _performative = zpt::ev::from_str(_lambda.first);
		std::string _name = std::string(_lambda.second);
		_handlers.insert(
			std::make_pair(_performative,
				[ _name ] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
					zpt::python::object _ret = _bridge->eval< zpt::python::object >(std::string("(") + _name + std::string(" \"") + zpt::ev::to_str(_performative) + std::string("\" \"") + _resource + std::string("\" `") + zpt::python::to_python_string(_envelope) + std::string(")"));
					return _bridge->from< zpt::python::object >(_ret);
				}
			)
		);
	}
	
	_bridge->events()->on(_topic, _handlers);
	Py_RETURN_TRUE;
}

auto zpt::python::module::route(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
	Py_RETURN_TRUE;
}

auto zpt::python::module::split(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
	Py_RETURN_TRUE;
}

auto zpt::python::module::topic_var(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
	Py_RETURN_TRUE;
}

auto zpt::python::module::validate_authorization(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
	Py_RETURN_TRUE;
}


namespace zpt {
	namespace python {
		namespace module {
			PyMethodDef methods[] = {
				{"on", zpt::python::module::on, METH_VARARGS, "Registers RESTful resource handler."},
				{"route", zpt::python::module::route, METH_VARARGS, "Route messages."},
				{"split", zpt::python::module::split, METH_VARARGS, "Split a string according to a separator."},
				{"topic_var", zpt::python::module::topic_var, METH_VARARGS, "Retrieves the nth part of a RESTful topic."},
				{"authorize", zpt::python::module::validate_authorization, METH_VARARGS, "Validates the received message authorization."},
				{nullptr, nullptr, 0, nullptr}
			};

			PyModuleDef spec = {
				PyModuleDef_HEAD_INIT, "zpt", nullptr, -1, zpt::python::module::methods,
				nullptr, nullptr, nullptr, nullptr
			};
		}
	}
}

