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
		std::shared_ptr< std::vector< std::pair< std::string, PyObject* (*)(void) > > > __modules(new std::vector< std::pair< std::string, PyObject* (*)(void) > >());
		zpt::json __sys_path = zpt::json::array();
	}
}

zpt::python::Bridge::Bridge(zpt::json _options) : zpt::Bridge(_options), __self(this), __lambdas(new std::map< std::string, std::function< zpt::python::object (int, zpt::python::object[]) > >()) {
}

zpt::python::Bridge::~Bridge() {
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
				FILE* _fp = ::fopen(_python_script->str().data(), "r");
				PyRun_SimpleFileEx(_fp, _python_script->str().data(), true);
			}
		}
	}
	zlog(std::string("PYTHON bridge initialized"), zpt::trace);
}

auto zpt::python::Bridge::deflbd(zpt::json _conf, std::function< zpt::python::object (int, zpt::python::object[]) > _callback) -> void {
	
}

auto zpt::python::Bridge::instance() -> zpt::bridge {
	assertz(zpt::python::__instance != nullptr, "you must invoke 'zpt::bridge::boot< zpt::python::bridge >' before requesting the instance", 500, 0);
	return zpt::python::__instance->self();
}

auto zpt::python::Bridge::is_booted() -> bool {
	return zpt::python::__instance != nullptr;
}

auto zpt::python::Bridge::defmdl(std::string _name, PyObject* (*_func)(void)) -> void {
	if (zpt::python::__modules->size() == 0) {
		zpt::python::__modules->push_back(std::make_pair("zpt", &zpt::python::module::init));
	}
	zpt::python::__modules->push_back(std::make_pair(_name, _func));
}

auto zpt::python::Bridge::add_syspath(std::string _name) -> void {
	zpt::python::__sys_path << _name;
}

auto zpt::python::Bridge::boot(zpt::json _options) -> void {
	assertz(zpt::python::__instance == nullptr, "bridge instance isn't null, 'zpt::bridge::boot< zpt::python::bridge >' already invoked", 500, 0);
	zpt::python::bridge* _bridge = new zpt::python::bridge(_options);
	zpt::python::__instance = _bridge;

	zlog(std::string("PYTHON bridge loading basic module (zpt.on, zpt.route, zpt.slipt, zpt.topic_var, zpt.authorize)"), zpt::trace);
	Py_SetProgramName((wchar_t*) "zpt");
	
	struct _inittab _initt[zpt::python::__modules->size() + 1];
	for (size_t _k = 0; _k != zpt::python::__modules->size(); _k++) {
		zlog(std::string("PYTHON registering module '") + zpt::python::__modules->at(_k).first + std::string("'"), zpt::notice);
		_initt[_k].name = zpt::python::__modules->at(_k).first.data();
		_initt[_k].initfunc = zpt::python::__modules->at(_k).second;
	}
	_initt[zpt::python::__modules->size()].name = nullptr;
	_initt[zpt::python::__modules->size()].initfunc = nullptr;	
	assertz(PyImport_ExtendInittab(_initt) != -1, std::string("could not import modules"), 500, 0);
	
	Py_Initialize();

	for (auto _module : (*zpt::python::__modules)) {
		PyImport_ImportModule(_module.first.data());
	}
	//PyDateTime_IMPORT;

	zpt::json _new_sys_path = zpt::python::from_python(PySys_GetObject("path")) + zpt::python::__sys_path;
	std::string _sys_path = zpt::join(_new_sys_path, ":");
	PySys_SetPath(zpt::utf8::utf8_to_wstring(_sys_path));
		
	zlog(std::string("PYTHON bridge booted"), zpt::notice);
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
	zpt::json _params = _bridge->from< zpt::python::object >(zpt::python::object(_args));
	std::map< zpt::ev::performative, zpt::ev::Handler > _handlers;
	zpt::json _lambdas = _params[1];
	std::string _topic = std::string(_params[0]);
	zpt::json _opts;
	PyObject* _context = nullptr;;
	if (_params->arr()->size() == 3) {
		_context = zpt::python::from_ref(_params[2]);;
	}
	else if (_params->arr()->size() == 4) {
		_opts = _params[2];
		_context = zpt::python::from_ref(_params[3]);;
	}
	Py_INCREF(_context);
	
	for (auto _lambda : _lambdas->obj()) {
		zpt::ev::performative _performative = zpt::ev::from_str(_lambda.first);
		PyObject* _func = **_bridge->to< zpt::python::object >(_lambda.second);
		Py_INCREF(_func);
		_handlers.insert(
			std::make_pair(_performative,
				[ _func, _context ] (zpt::ev::performative _performative, std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
					zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
					PyObject* _args = PyTuple_Pack(4, PyUnicode_DecodeFSDefault(zpt::ev::to_str(_performative).data()), PyUnicode_DecodeFSDefault(_resource.data()), zpt::python::to_python(_envelope), _context);
					PyObject* _ret = PyObject_CallObject(_func, _args);
					if (_ret == nullptr) {
						return zpt::undefined;
					}				
					return _bridge->from< zpt::python::object >(_ret);
				}
			)
		);
	}
	
	_bridge->events()->on(_topic, _handlers, _opts);
	Py_RETURN_TRUE;
}

auto zpt::python::module::route(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
	zpt::json _params = _bridge->from< zpt::python::object >(zpt::python::object(_args));
	zpt::json _performative = _params[0];
	zpt::json _topic = _params[1];
	zpt::json _envelope = _params[2];
	
	zpt::json _ret = _bridge->events()->route(zpt::ev::performative(int(_performative)), std::string(_topic), _envelope);
	return **_bridge->to< zpt::python::object >(_ret);
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

auto zpt::python::module::options(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
	return **_bridge->to< zpt::python::object >(_bridge->options());
}

auto zpt::python::module::hook(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance< zpt::python::bridge >();
	zpt::json _params = _bridge->from< zpt::python::object >(zpt::python::object(_args));
	std::map< zpt::ev::performative, zpt::ev::Handler > _handlers;
	zpt::json _lambda = _params[0];
	PyObject* _func = **_bridge->to< zpt::python::object >(_lambda);
	Py_INCREF(_func);
	PyObject* _context = zpt::python::from_ref(_params[1]);
	Py_INCREF(_context);
	_bridge->events()->hook(
		[ _func, _context ] (zpt::ev::emitter _emitter) -> void {
			PyObject* _args = PyTuple_Pack(1, _context);
			PyObject_CallObject(_func, _args);
		}
	);	
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
				{"options", zpt::python::module::options, METH_VARARGS, "Returns the present configuration."},
				{"hook", zpt::python::module::hook, METH_VARARGS, "Callbacks to be executed upon initialization."},
				{nullptr, nullptr, 0, nullptr}
			};

			PyModuleDef spec = {
				PyModuleDef_HEAD_INIT, "zpt", nullptr, -1, zpt::python::module::methods
			};
		}
	}
}

extern "C" auto zpt_python() -> int {
	return 1;
}

