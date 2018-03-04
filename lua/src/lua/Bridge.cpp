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

#include <zapata/lua/Bridge.h>

namespace zpt {

namespace lua {
zpt::lua::bridge* __instance = nullptr;
std::shared_ptr<std::vector<std::pair<std::string, PyObject* (*)(void)>>>
__modules(new std::vector<std::pair<std::string, PyObject* (*)(void)>>());
zpt::json __sys_path = zpt::json::array();
}
}

zpt::lua::Bridge::Bridge(zpt::json _options)
    : zpt::Bridge(_options), __self(this),
      __lambdas(new std::map<std::string, std::function<zpt::lua::object(int, zpt::lua::object[])>>()) {
	if (zpt::lua::__modules->size() == 0) {
		zpt::lua::__modules->push_back(std::make_pair("zpt", &zpt::lua::module::init));
	}
}

zpt::lua::Bridge::~Bridge() {}

auto zpt::lua::Bridge::name() -> std::string { return std::string("lua"); }

auto zpt::lua::Bridge::events(zpt::ev::emitter _emitter) -> void { this->__events = _emitter; }

auto zpt::lua::Bridge::events() -> zpt::ev::emitter { return this->__events; }

auto zpt::lua::Bridge::self() const -> zpt::bridge { return this->__self; }

auto zpt::lua::Bridge::unbind() -> void { this->__self.reset(); }

auto zpt::lua::Bridge::eval(std::string _call) -> zpt::lua::object { return zpt::lua::object(); }

auto zpt::lua::Bridge::initialize() -> void { ztrace(std::string("PYTHON bridge initialized")); }

auto zpt::lua::Bridge::load_module(std::string _module) -> void {
	if (_module.find(".py") != std::string::npos) {
		zlog(std::string("loading module '") + _module + std::string("'"), zpt::notice);
		FILE* _fp = ::fopen(_module.data(), "r");
		PyRun_SimpleFileEx(_fp, _module.data(), true);
	}
}

auto zpt::lua::Bridge::deflbd(zpt::json _conf,
				 std::function<zpt::lua::object(int, zpt::lua::object[])> _callback) -> void {}

auto zpt::lua::Bridge::instance() -> zpt::bridge {
	assertz(zpt::lua::__instance != nullptr,
		"you must invoke 'zpt::bridge::boot< zpt::lua::bridge >' before requesting the instance",
		500,
		0);
	return zpt::lua::__instance->self();
}

auto zpt::lua::Bridge::is_booted() -> bool { return zpt::lua::__instance != nullptr; }

auto zpt::lua::Bridge::defmdl(std::string _name, PyObject* (*_func)(void)) -> void {
	if (zpt::lua::__modules->size() == 0) {
		zpt::lua::__modules->push_back(std::make_pair("zpt", &zpt::lua::module::init));
	}
	zpt::lua::__modules->push_back(std::make_pair(_name, _func));
}

auto zpt::lua::Bridge::add_syspath(std::string _name) -> void { zpt::lua::__sys_path << _name; }

auto zpt::lua::Bridge::boot(zpt::json _options) -> void {
	assertz(zpt::lua::__instance == nullptr,
		"bridge instance isn't null, 'zpt::bridge::boot< zpt::lua::bridge >' already invoked",
		500,
		0);
	zpt::lua::bridge* _bridge = new zpt::lua::bridge(_options);
	zpt::lua::__instance = _bridge;

	// zlog(std::string("PYTHON bridge loading basic module (zpt.on, zpt.route, zpt.authorize, zpt.path_join,
	// zpt.merge, zpt.auth_header)"), zpt::notice);
	Py_SetProgramName((wchar_t*)"zpt");

	struct _inittab _initt[zpt::lua::__modules->size() + 1];
	for (size_t _k = 0; _k != zpt::lua::__modules->size(); _k++) {
		zlog(std::string("loading module 'lua-") + zpt::lua::__modules->at(_k).first + std::string("'"),
		     zpt::notice);
		_initt[_k].name = zpt::lua::__modules->at(_k).first.data();
		_initt[_k].initfunc = zpt::lua::__modules->at(_k).second;
	}
	_initt[zpt::lua::__modules->size()].name = nullptr;
	_initt[zpt::lua::__modules->size()].initfunc = nullptr;
	assertz(PyImport_ExtendInittab(_initt) != -1, std::string("could not import modules"), 500, 0);

	Py_Initialize();
	if (!PyEval_ThreadsInitialized()) {
		PyEval_InitThreads();
	}

	for (auto _module : (*zpt::lua::__modules)) {
		PyImport_ImportModule(_module.first.data());
	}
	// PyDateTime_IMPORT;

	zpt::json _new_sys_path = zpt::lua::from_lua(PySys_GetObject("path")) + zpt::lua::__sys_path;
	std::string _sys_path = zpt::join(_new_sys_path, ":");
	PySys_SetPath(zpt::utf8::utf8_to_wstring(_sys_path));

	ztrace(std::string("PYTHON bridge booted"));
}

zpt::lua::Object::Object(PyObject* _target) : std::shared_ptr<zpt::lua::Type>(new zpt::lua::Type(_target)) {}

zpt::lua::Object::Object() : std::shared_ptr<zpt::lua::Type>(nullptr) {}

auto zpt::lua::Object::bridge() -> zpt::lua::bridge* { return zpt::lua::__instance; }

auto zpt::lua::Object::fromjson(zpt::json _in) -> zpt::lua::object {
	return zpt::lua::object(zpt::lua::to_lua(_in, zpt::lua::__instance));
}

zpt::lua::Type::Type(PyObject* _target) : __target(_target) {}

zpt::lua::Type::Type() : __target(nullptr) {}

zpt::lua::Type::~Type() {
	if (this->__target != nullptr) {
		Py_DECREF(this->__target);
	}
}

auto zpt::lua::Type::tojson() -> zpt::json { return zpt::lua::from_lua(this->__target); }

/*-------------------------------------------------------------------------\
| PYTHON MODULE                                                           |
\-------------------------------------------------------------------------*/
auto zpt::lua::module::init() -> PyObject* { return PyModule_Create(&zpt::lua::module::spec); }

auto zpt::lua::module::on(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	std::map<zpt::ev::performative, zpt::ev::Handler> _handlers;
	assertz_mandatory(_params[0], "", 412);
	assertz_mandatory(_params[1], "", 412);

	if (_params[1]->is_string() && std::string(_params[1]).find("ref(") == 0) {
		std::string _topic = std::string(_params[0]);
		std::string _ref = _params[1];
		zpt::json _opts = _params[2];
		std::vector<std::string> _performatives = {"get", "post", "put", "patch", "delete", "head", "reply"};

		for (auto _p : _performatives) {
			zpt::ev::performative _performative = zpt::ev::from_str(_p);
			_handlers.insert(std::make_pair(
			    _performative,
			    [_ref](zpt::ev::performative _performative,
				   std::string _resource,
				   zpt::json _envelope,
				   zpt::ev::emitter _emitter) mutable -> void {
				    zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
				    std::string _s_performative = zpt::ev::to_str(_performative);
				    std::transform(_s_performative.begin(),
						   _s_performative.end(),
						   _s_performative.begin(),
						   ::tolower);
				    PyObject* _instance = zpt::lua::from_ref(zpt::json::string(_ref));

				    try {
					    PyErr_Clear();
					    PyObject_CallMethodObjArgs(
						_instance,
						(_performative == zpt::ev::Delete
						     ? PyUnicode_DecodeFSDefault("remove")
						     : PyUnicode_DecodeFSDefault(_s_performative.data())),
						PyUnicode_DecodeFSDefault(_s_performative.data()),
						PyUnicode_DecodeFSDefault(_resource.data()),
						zpt::lua::to_lua(_envelope),
						nullptr);
				    } catch (zpt::assertion& _e) {
					    throw;
				    } catch (...) {
				    }

				    PyObject *_py_error_type = nullptr, *_py_error = nullptr, *_traceback = nullptr;
				    PyErr_Fetch(&_py_error_type, &_py_error, &_traceback);
				    if (_py_error_type == nullptr) {
					    return;
				    }

				    zpt::json _e_type =
					_bridge->from<zpt::lua::object>(zpt::lua::object(_py_error_type));
				    zpt::json _e_message =
					_bridge->from<zpt::lua::object>(zpt::lua::object(_py_error));
				    std::string _text = std::string("lua error: ") + std::string(_e_type) +
							std::string(": ") + std::string(_e_message);
				    zlog(std::string("error processing '") + _resource + std::string("': ") + _text,
					 zpt::error);
				    _bridge->events()->reply(_envelope,
							     {"status", 500, "payload", {"text", _text, "code", 3000}});
			    }));
		}

		_bridge->events()->on(_topic, _handlers, _opts);
	} else {
		zpt::json _callbacks = _params[1];
		std::string _topic = std::string(_params[0]);
		zpt::json _opts = _params[2];
		zpt::json _instance;
		PyObject* _context = nullptr;
		if (_params->arr()->size() == 4) {
			_context = zpt::lua::from_ref(_params[3]);
			_instance = _params[3];
			Py_INCREF(_context);
		}

		_opts >> "bubble-error";

		for (auto _handler : _callbacks->obj()) {
			zpt::ev::performative _performative = zpt::ev::from_str(_handler.first);
			PyObject* _func = **_bridge->to<zpt::lua::object>(_handler.second);
			zpt::json _lambda = zpt::lua::to_ref(_func);
			Py_INCREF(_func);
			_handlers.insert(std::make_pair(
			    _performative,
			    [_lambda, _instance](zpt::ev::performative _performative,
						 std::string _resource,
						 zpt::json _envelope,
						 zpt::ev::emitter _emitter) mutable -> void {
				    zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
				    std::string _s_performative = zpt::ev::to_str(_performative);
				    std::transform(_s_performative.begin(),
						   _s_performative.end(),
						   _s_performative.begin(),
						   ::tolower);
				    PyObject* _func = zpt::lua::from_ref(_lambda);
				    PyObject* _args = nullptr;
				    PyObject* _context = Py_None;
				    if (_instance->ok()) {
					    _context = zpt::lua::from_ref(_instance);
				    }
				    _args = PyTuple_Pack(4,
							 PyUnicode_DecodeFSDefault(_s_performative.data()),
							 PyUnicode_DecodeFSDefault(_resource.data()),
							 zpt::lua::to_lua(_envelope),
							 _context);

				    try {
					    PyErr_Clear();
					    PyObject_CallObject(_func, _args);
				    } catch (zpt::assertion& _e) {
					    throw;
				    } catch (...) {
				    }

				    PyObject *_py_error_type = nullptr, *_py_error = nullptr, *_traceback = nullptr;
				    PyErr_Fetch(&_py_error_type, &_py_error, &_traceback);
				    if (_py_error_type == nullptr) {
					    return;
				    }

				    zpt::json _e_type =
					_bridge->from<zpt::lua::object>(zpt::lua::object(_py_error_type));
				    zpt::json _e_message =
					_bridge->from<zpt::lua::object>(zpt::lua::object(_py_error));
				    std::string _text = std::string("lua error: ") + std::string(_e_type) +
							std::string(": ") + std::string(_e_message);
				    zlog(std::string("error processing '") + _resource + std::string("': ") + _text,
					 zpt::error);
				    _bridge->events()->reply(_envelope,
							     {"status", 500, "payload", {"text", _text, "code", 3000}});
			    }));
		}

		_bridge->events()->on(_topic, _handlers, _opts);
	}
	Py_RETURN_TRUE;
}

auto zpt::lua::module::route(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	assertz_mandatory(_params[0], "", 412);
	assertz_mandatory(_params[1], "", 412);

	return zpt::lua::module::route(_self, _params);
}

auto zpt::lua::module::route(PyObject* _self, zpt::json _params) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _performative = _params[0];
	zpt::json _topic = _params[1];
	zpt::json _envelope = _params[2];
	zpt::json _opts = _params[3];
	zpt::json _callback = _params[4];

	_opts >> "bubble-error";

	if (_callback->is_lambda()) {
		PyObject* _func = **_bridge->to<zpt::lua::object>(_callback);
		zpt::json _lambda = zpt::lua::to_ref(_func);
		Py_INCREF(_func);
		zpt::json _context = _opts["context"];
		_bridge->events()->route(
		    zpt::ev::performative(int(zpt::ev::from_str(std::string(_performative)))),
		    std::string(_topic),
		    _envelope,
		    _opts,
		    [_lambda, _context](zpt::ev::performative _p_performative,
					std::string _p_topic,
					zpt::json _p_result,
					zpt::ev::emitter _emitter) mutable -> void {
			    zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
			    PyObject* _func = zpt::lua::from_ref(_lambda);
			    PyObject* _ctx = Py_None;
			    if (_context->ok()) {
				    if (_context->is_object()) {
					    _ctx = zpt::lua::to_lua(_context);
				    } else {
					    _ctx = zpt::lua::from_ref(_context);
				    }
			    }
			    PyObject* _args =
				PyTuple_Pack(4,
					     PyUnicode_DecodeFSDefault(zpt::ev::to_str(_p_performative).data()),
					     PyUnicode_DecodeFSDefault(_p_topic.data()),
					     zpt::lua::to_lua(_p_result),
					     _ctx);
			    try {
				    PyErr_Clear();
				    PyObject_CallObject(_func, _args);
			    } catch (...) {
			    }

			    PyObject *_py_error_type = nullptr, *_py_error = nullptr, *_traceback = nullptr;
			    PyErr_Fetch(&_py_error_type, &_py_error, &_traceback);
			    if (_py_error_type == nullptr) {
				    return;
			    }

			    zpt::json _e_type = _bridge->from<zpt::lua::object>(zpt::lua::object(_py_error_type));
			    zpt::json _e_message = _bridge->from<zpt::lua::object>(zpt::lua::object(_py_error));
			    std::string _text = std::string("lua error: ") + std::string(_e_type) +
						std::string(": ") + std::string(_e_message);
			    zlog(std::string("error processing '") + _p_topic + std::string("': ") + _text, zpt::error);
			    if (_context->is_object() && _context["channel"]->is_string()) {
				    _bridge->events()->reply(_context,
							     {"status", 500, "payload", {"text", _text, "code", 3000}});
			    }
		    });
	} else {
		_bridge->events()->route(zpt::ev::performative(int(zpt::ev::from_str(std::string(_performative)))),
					 std::string(_topic),
					 _envelope,
					 _opts);
	}
	Py_RETURN_TRUE;
}

auto zpt::lua::module::reply(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	assertz_mandatory(_params[0], "", 412);
	assertz_mandatory(_params[1], "", 412);
	zpt::json _to_reply_to = _params[0];
	zpt::json _reply = _params[1];
	_bridge->events()->reply(_to_reply_to, _reply);
	Py_RETURN_TRUE;
}

auto zpt::lua::module::validate_authorization(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	assertz_mandatory(_params[0], "", 412);
	assertz_mandatory(_params[1], "", 412);
	zpt::json _topic = _params[0];
	zpt::json _envelope = _params[1];
	zpt::json _roles = _params[2];
	try {
		zpt::json _identity = _bridge->events()->authorize(std::string(_topic), _envelope, _roles);
		return **_bridge->to<zpt::lua::object>(_identity);
	} catch (zpt::assertion& _e) {
		Py_RETURN_NONE;
	}
}

auto zpt::lua::module::options(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	return **_bridge->to<zpt::lua::object>(_bridge->options());
}

auto zpt::lua::module::hook(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	std::map<zpt::ev::performative, zpt::ev::Handler> _handlers;
	assertz_mandatory(_params[0], "", 412);
	zpt::json _lambda = _params[0];
	zpt::json _ref = _params[1];
	PyObject* _func = **_bridge->to<zpt::lua::object>(_lambda);
	Py_INCREF(_func);
	PyObject* _context = zpt::lua::from_ref(_ref);
	Py_INCREF(_context);
	_bridge->events()->hook([_lambda, _ref](zpt::ev::emitter _emitter) -> void {
		zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
		PyObject* _func = zpt::lua::from_ref(_lambda);
		PyObject* _context = zpt::lua::from_ref(_ref);
		PyObject* _args = PyTuple_Pack(1, _context);

		try {
			PyErr_Clear();
			PyObject_CallObject(_func, _args);
		} catch (...) {
		}

		PyObject *_py_error_type = nullptr, *_py_error = nullptr, *_traceback = nullptr;
		PyErr_Fetch(&_py_error_type, &_py_error, &_traceback);
		if (_py_error_type == nullptr) {
			return;
		}

		zpt::json _e_type = _bridge->from<zpt::lua::object>(zpt::lua::object(_py_error_type));
		zpt::json _e_message = _bridge->from<zpt::lua::object>(zpt::lua::object(_py_error));
		std::string _text =
		    std::string("lua error: ") + std::string(_e_type) + std::string(": ") + std::string(_e_message);
		zlog(std::string("error processing hook: ") + _text, zpt::error);

	});
	Py_RETURN_TRUE;
}

auto zpt::lua::module::log(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	assertz_mandatory(_params[1], "", 412);
	std::string _text = std::string(_params[0]);
	int _level = int(_params[1]);
	zlog(_text, (zpt::LogLevel)_level);
	Py_RETURN_TRUE;
}

auto zpt::lua::module::assertion(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	assertz_mandatory(_params[0], "", 412);
	assertz_mandatory(_params[1], "", 412);
	assertz_mandatory(_params[2], "", 412);
	bool _guard = bool(_params[0]);
	std::string _message = std::string(_params[1]);
	int _status = int(_params[2]);
	assertz(_guard, _message, _status, 0);
	Py_RETURN_TRUE;
}

auto zpt::lua::module::path_join(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	assertz_mandatory(_params[0], "", 412);
	zpt::json _array = _params[0];
	if (_array->is_array()) {
		std::string _path = zpt::path::join(_params[0]);
		return **_bridge->to<zpt::lua::object>(zpt::json::string(_path));
	}
	Py_RETURN_NONE;
}

auto zpt::lua::module::authorization_headers(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	assertz_mandatory(_params[0], "", 412);
	zpt::json _identity = _params[0];
	if (_identity->is_object()) {
		zpt::json _return = {"Authorization",
				     (std::string("Bearer ") + std::string(_identity["access_token"]))};
		return **_bridge->to<zpt::lua::object>(_return);
	}
	Py_RETURN_NONE;
}

auto zpt::lua::module::merge(PyObject* _self, PyObject* _args) -> PyObject* {
	zpt::bridge _bridge = zpt::bridge::instance<zpt::lua::bridge>();
	zpt::json _params = _bridge->from<zpt::lua::object>(zpt::lua::object(_args));
	zpt::json _lhs = _params[0];
	zpt::json _rhs = _params[1];
	return **_bridge->to<zpt::lua::object>(_lhs + _rhs);
}

namespace zpt {
namespace lua {
namespace module {
PyMethodDef methods[] = {
    {"on", zpt::lua::module::on, METH_VARARGS, "Registers RESTful resource handler."},
    {"route", zpt::lua::module::route, METH_VARARGS, "Route messages."},
    {"reply", zpt::lua::module::reply, METH_VARARGS, "Reply to message."},
    {"authorize",
     zpt::lua::module::validate_authorization,
     METH_VARARGS,
     "Validates the received message authorization."},
    {"path_join", zpt::lua::module::path_join, METH_VARARGS, "Converts a list of path parts into a path string."},
    {"auth_headers",
     zpt::lua::module::authorization_headers,
     METH_VARARGS,
     "Extracts the access token from object and return string with Authorization header."},
    {"merge", zpt::lua::module::merge, METH_VARARGS, "Merges two JSON objects."},
    {"options", zpt::lua::module::options, METH_VARARGS, "Returns the present configuration."},
    {"hook", zpt::lua::module::hook, METH_VARARGS, "Callbacks to be executed upon initialization."},
    {"log", zpt::lua::module::log, METH_VARARGS, "Logging function."},
    {"assertz", zpt::lua::module::assertion, METH_VARARGS, "Assertion function."},
    {nullptr, nullptr, 0, nullptr}};

PyModuleDef spec = {PyModuleDef_HEAD_INIT, "zpt", nullptr, -1, zpt::lua::module::methods};
}
}
}

extern "C" auto zpt_lua() -> int { return 1; }
