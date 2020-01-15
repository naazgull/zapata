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

#include <zapata/lisp/Bridge.h>

namespace zpt {

namespace lisp {
zpt::lisp::bridge* __instance = nullptr;
}
} // namespace zpt

zpt::lisp::Bridge::Bridge(zpt::json _options)
  : zpt::Bridge(_options)
  , __self(this)
  , __lambdas(
      new std::map<std::string, std::function<zpt::lisp::object(int, zpt::lisp::object[])>>())
  , __modules(new std::map<std::string, std::string>())
  , __consistency(
      new std::map<std::string, std::function<bool(const std::string, const std::string)>>()) {
    char _arg[] = { 'z', 'p', 't', '\0' };
    char* _argv[] = { _arg };

    if (!_options["lisp"]["repl"]->ok() || !bool(_options["lisp"]["repl"])) {
        ecl_set_option(ECL_OPT_TRAP_SIGSEGV, FALSE);
        ecl_set_option(ECL_OPT_TRAP_SIGFPE, FALSE);
        ecl_set_option(ECL_OPT_TRAP_SIGINT, FALSE);
        ecl_set_option(ECL_OPT_TRAP_SIGILL, FALSE);
        ecl_set_option(ECL_OPT_TRAP_INTERRUPT_SIGNAL, FALSE);
        ecl_set_option(ECL_OPT_SIGNAL_HANDLING_THREAD, FALSE);
    }

    cl_boot(1, _argv);
    atexit(cl_shutdown);
}

zpt::lisp::Bridge::~Bridge() {}

auto
zpt::lisp::Bridge::name() -> std::string {
    return std::string("lisp");
}

auto
zpt::lisp::Bridge::events(zpt::ev::emitter _emitter) -> void {
    this->__events = _emitter;
}

auto
zpt::lisp::Bridge::events() -> zpt::ev::emitter {
    return this->__events;
}

auto
zpt::lisp::Bridge::self() const -> zpt::bridge {
    return this->__self;
}

auto
zpt::lisp::Bridge::unbind() -> void {
    this->__self.reset();
}

auto
zpt::lisp::Bridge::eval(std::string _call) -> zpt::lisp::object {
    return zpt::lisp::object(cl_safe_eval(c_string_to_object(_call.c_str()), Cnil, Cnil));
}

auto
zpt::lisp::Bridge::initialize() -> void {
    /*this->eval(
        "(defmacro json (&rest args)\n"
        "(let ((temphash (gensym))\n"
        "(tempkey (gensym))\n"
        "(tempval (gensym)))\n"
        "`(let ((,temphash (make-hash-table :test #'equal :size ,(/ (length args)
       2))))\n"
        "(loop for (,tempkey ,tempval) on ',args by #'cddr do\n"
        "(setf (gethash (eval ,tempkey) ,temphash) (eval ,tempval)))\n"
        ",temphash)))"
        );*/

    this->eval("	(defmacro json (&rest args)"
               "		(let* ((temphash (gensym))"
               "			(size (/ (length args) 2))"
               "			(forms (loop for (key val) on args by #'cddr"
               "                  collect `(setf (gethash ,key ,temphash) ,val))))"
               "    `(let ((,temphash (make-hash-table :test #'equal :size ,size)))"
               "			,@forms"
               "			,temphash)))");
    ztrace(std::string("LISP bridge initialized"));
}

auto
zpt::lisp::Bridge::load_module(std::string _module) -> void {
    if (_module.find(".lisp") != std::string::npos || _module.find(".fasb") != std::string::npos) {
        zlog(std::string("loading module '") + _module + std::string("'"), zpt::notice);
        this->eval(std::string("(load \"") + _module + std::string("\")"));
    }
}

auto
zpt::lisp::Bridge::defun(zpt::json _conf, cl_objectfn_fixed _fun, int _n_args) -> void {
    cl_def_c_function(c_string_to_object(_conf["name"]->str().data()), _fun, _n_args);
    this->defop(_conf);
}

auto
zpt::lisp::Bridge::deflbd(zpt::json _conf,
                          std::function<zpt::lisp::object(int, zpt::lisp::object[])> _callback)
  -> void {
    size_t _n_args = _conf["args"]->type() != zpt::JSArray ? 0 : _conf["args"]->arr()->size();
    std::string _name(_conf["name"]->str() + std::string("/") + std::to_string(_n_args));
    auto _found = this->__lambdas->find(_name);
    if (_found == this->__lambdas->end()) {
        this->__lambdas->insert(make_pair(_name, _callback));
        std::string _args_string;
        std::string _coerced_args_string;
        if (_conf["args"]->type() == zpt::JSArray) {
            size_t _i = 0;
            for (auto _arg : _conf["args"]->arr()) {
                _args_string += (bool(_arg["optional"]) ? "&optional " : "") + std::string("arg") +
                                std::to_string(_i) + std::string(" ");
                _coerced_args_string += std::string("arg") + std::to_string(_i) + std::string(" ");
                _i++;
            }
        }
        std::string _expression =
          std::string("(defun ") + _conf["name"]->str() + std::string(" (") + _args_string +
          std::string(") (cpp-lambda-call \"") + _name + std::string("\" ") +
          std::to_string(_n_args) + std::string(" (make-array ") + std::to_string(_n_args) +
          std::string(" :initial-contents (list ") + _coerced_args_string + std::string(") ) ) )");
        this->eval(_expression);
        this->defop(_conf);
    }
}

auto
zpt::lisp::Bridge::defop(zpt::json _conf) -> void {
    expect(_conf["name"]->ok(), "the configuration attribute 'name' is required", 0, 0);
    expect(_conf["label"]->ok(), "the configuration attribute 'label' is required", 0, 0);
    expect(_conf["type"]->ok(), "the configuration attribute 'type' is required", 0, 0);
    expect(_conf["access"]->ok(), "the configuration attribute 'access' is required", 0, 0);

    std::string _expression(
      std::string("(setf (gethash '") + _conf["name"]->str() +
      std::string(" *defined-operators*) '((name . \"") + _conf["name"]->str() +
      std::string("\") (access . \"") + _conf["access"]->str() + std::string("\") ") +
      (_conf["category"]->ok()
         ? std::string("(category . \"") + ((std::string)_conf["category"]) + std::string("\") ")
         : std::string("")) +
      (_conf["triggerable"]->ok() ? std::string("(triggerable . \"") +
                                      ((std::string)_conf["triggerable"]) + std::string("\") ")
                                  : std::string("")) +
      std::string("(args . ("));
    if (_conf["args"]->ok()) {
        for (auto _a : _conf["args"]->arr()) {
            _expression += std::string("((type . \"") + _a["type"]->str() +
                           std::string("\") (label . \"") + _a["label"]->str() +
                           std::string("\")) ");
        }
    }
    _expression += std::string("))");
    if (_conf["post-state"]->ok()) {
        _expression += std::string(" (post-state . (");
        for (auto _a : _conf["post-state"]->arr()) {
            _expression += std::string("((prop . \"") + _a["prop"]->str() +
                           std::string("\") (value . \"") + _a["value"]->str() +
                           std::string("\"))");
        }
        _expression += std::string("))");
    }
    _expression += std::string(" (label . \"") + _conf["label"]->str() +
                   std::string("\") (type . \"") + _conf["type"]->str() + std::string("\")))");

    this->eval(_expression);
    this->__modules->insert(make_pair(_conf["name"]->str(), this->__current));
}

auto
zpt::lisp::Bridge::defchk(std::function<bool(const std::string, const std::string)> _callback)
  -> void {
    auto _found = this->__consistency->find(this->__current);
    if (_found == this->__consistency->end()) {
        this->__consistency->insert(make_pair(this->__current, _callback));
    }
}

auto
zpt::lisp::Bridge::defmod(std::string _module) -> void {
    this->__current.assign(_module);
}

auto
zpt::lisp::Bridge::call(const char* _c_name, int _n_args, zpt::lisp::object _args[])
  -> zpt::lisp::object {
    std::string _name(_c_name);
    auto _found = this->__lambdas->find(_name);
    if (_found != this->__lambdas->end()) {
        return (_found->second)(_n_args, _args);
    }
    return zpt::lisp::object(ecl_make_bool(false));
}

auto
zpt::lisp::Bridge::check(const std::string _op1, const std::string _op2) -> bool {
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

auto
zpt::lisp::Bridge::instance() -> zpt::bridge {
    expect(zpt::lisp::__instance != nullptr,
           "you must invoke 'zpt::bridge::boot< zpt::lisp::bridge >' before "
           "requesting the instance",
           500,
           0);
    return zpt::lisp::__instance->self();
}

auto
zpt::lisp::Bridge::is_booted() -> bool {
    return zpt::lisp::__instance != nullptr;
}

auto
zpt::lisp::Bridge::boot(zpt::json _options) -> void {
    expect(zpt::lisp::__instance == nullptr,
           "bridge instance isn't null, 'zpt::bridge::boot< zpt::lisp::bridge "
           ">' already invoked",
           500,
           0);
    zpt::lisp::bridge* _bridge = new zpt::lisp::bridge(_options);
    zpt::lisp::__instance = _bridge;
    zpt::lisp::__instance->options() << "is_lisp_booted"
                                     << "on";
    _bridge->eval("(defparameter *defined-operators* (make-hash-table))");
    _bridge->eval("(defpackage :zpt "
                  "(:use :common-lisp) "
                  "(:export "
                  ":on "
                  ":route "
                  ":split "
                  ":join "
                  ":merge "
                  ":replace "
                  ":topic-var "
                  ":authorize "
                  ":authorization-headers "
                  ":assertz-mandatory "
                  ":assertz-string "
                  ":assertz-integer "
                  ":assertz-double "
                  ":assertz-timestamp "
                  ":assertz-boolean "
                  ":assertz-complex "
                  ":assertz-object "
                  ":assertz-array "
                  ":assertz-int "
                  ":assertz-uuid "
                  ":assertz-utf8 "
                  ":assertz-ascii "
                  ":expect-hash "
                  ":assertz-token "
                  ":assertz-uri "
                  ":assertz-email "
                  ":assertz-location "
                  ":generate-key "
                  ":generate-uuid "
                  ":json-date "
                  ":credentials "
                  "))");
    ztrace(std::string("LISP bridge loading basic operators (cpp-lambda-call, "
                       "check-consistency, zlog, "
                       "get-log-level, zpt:on, zpt:route, zpt:split, "
                       "zpt:topic-var, zpt:authorize)"));
    _bridge->defun({ "name",
                     "cpp-lambda-call",
                     "type",
                     "internal",
                     "access",
                     "x",
                     "label",
                     "delegate C++ lambda call",
                     "args",
                     { zpt::array,
                       { "type", "string", "label", "function name" },
                       { "type", "integer", "label", "number of arguments" },
                       { "type", "array", "label", "arguments" } } },
                   (cl_objectfn_fixed)zpt::lisp::cpp_lambda_call,
                   3);
    zpt::lisp::builtin_operators(_bridge);
    ztrace(std::string("LISP bridge booted"));
}

zpt::lisp::Object::Object(cl_object _target)
  : std::shared_ptr<zpt::lisp::Type>(new zpt::lisp::Type(_target)) {}

zpt::lisp::Object::Object()
  : std::shared_ptr<zpt::lisp::Type>(nullptr) {}

auto
zpt::lisp::Object::bridge() -> zpt::lisp::bridge* {
    return zpt::lisp::__instance;
}

auto
zpt::lisp::Object::fromjson(zpt::json _in) -> zpt::lisp::object {
    return zpt::lisp::object(zpt::lisp::to_lisp(_in, zpt::lisp::__instance));
}

zpt::lisp::Type::Type(cl_object _target)
  : __target(_target) {}

zpt::lisp::Type::Type()
  : __target(nullptr) {}

auto
zpt::lisp::Type::tojson() -> zpt::json {
    return zpt::lisp::from_lisp(this->__target);
}

/*-------------------------------------------------------------------------\
| LISP OPERATORS                                                           |
\-------------------------------------------------------------------------*/
auto
zpt::lisp::cpp_lambda_call(cl_object _fn_name, cl_object _n_args, cl_object _args) -> cl_object {
    zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

    zpt::json _ptr = _bridge->from<zpt::lisp::object>(zpt::lisp::object(_fn_name));
    std::string _coerced_fn_name = (std::string)_ptr;
    unsigned int _coerced_n_args = ecl_to_unsigned_integer(_n_args);
    unsigned int _coerced_args_dim = ecl_array_dimension(_args, 0);
    expect(_coerced_n_args >= _coerced_args_dim,
           std::string("invalid number of arguments, ") + std::to_string(_coerced_n_args) +
             std::string(" arguments defined and ") + std::to_string(_coerced_args_dim) +
             std::string(" arguments passed"),
           0,
           0);

    zpt::lisp::object _arr[_coerced_n_args];
    for (unsigned int _i = 0; _i != _coerced_n_args; _i++) {
        _arr[_i] = zpt::lisp::object(ecl_aref1(_args, _i));
    }
    ztrace(_coerced_fn_name);
    return **((zpt::lisp::bridge*)_bridge.get())
               ->call(_coerced_fn_name.data(), _coerced_n_args, _arr)
               .get();
}

auto
zpt::lisp::builtin_operators(zpt::lisp::bridge* _bridge) -> void {
    _bridge->deflbd(
      { "name",
        "check-consistency",
        "type",
        "internal",
        "access",
        "r",
        "label",
        "Check two function consistency",
        "args",
        { zpt::array,
          { "type", "string", "label", "left hand side function name" },
          { "type", "string", "label", "right hand side function name" } } },
      [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
          zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

          std::string _op1((char*)(*_args[0])->base_string.self, (*_args[0])->base_string.fillp);
          std::string _op2((char*)(*_args[1])->base_string.self, (*_args[1])->base_string.fillp);
          return zpt::lisp::object(
            ecl_make_bool(((zpt::lisp::bridge*)_bridge.get())->check(_op1, _op2)));
      });
    _bridge->deflbd({ "name",
                      "zlog",
                      "type",
                      "internal",
                      "access",
                      "r",
                      "label",
                      "Logging function",
                      "args",
                      { zpt::array,
                        { "type", "string", "label", "text to be logged" },
                        { "type", "int", "label", "log level (syslog levels)" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _arg0 = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _op1((std::string)_arg0);
                        auto _log_level = fix((**_args[1]));
                        zlog(_op1, (zpt::LogLevel)_log_level);
                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "get-log-level",
                      "type",
                      "internal",
                      "access",
                      "r",
                      "label",
                      "Get current specified log level" },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        if (_bridge->options()["log-level"]->ok()) {
                            return zpt::lisp::object(
                              ecl_make_fixnum(_bridge->options()["log-level"]));
                        }
                        return zpt::lisp::object(ecl_make_bool(false));
                    });
    _bridge->deflbd(
      { "name",
        "zpt:on",
        "type",
        "internal",
        "access",
        "a",
        "label",
        "Registers resource handler",
        "args",
        { zpt::array,
          { "type", "string", "label", "request topic pattern" },
          { "type", "object", "label", "performative / operator hash table" },
          { "type", "object", "label", "options", "optional", true } } },
      [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
          zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

          std::string _topic = std::string(_bridge->from<zpt::lisp::object>(_args[0]));
          zpt::json _lambdas = _bridge->from<zpt::lisp::object>(_args[1]);
          zpt::json _opts = _bridge->from<zpt::lisp::object>(_args[2]);
          std::map<zpt::performative, zpt::ev::Handler> _handlers;

          for (auto _lambda : _lambdas->obj()) {
              zpt::performative _performative = zpt::ev::from_str(_lambda.first);
              std::string _name = std::string(_lambda.second);
              _handlers.insert(std::make_pair(
                _performative,
                [_name](zpt::performative _performative,
                        std::string _resource,
                        zpt::json _envelope,
                        zpt::ev::emitter _emitter) -> void {
                    zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();
                    zpt::lisp::object _ret = _bridge->eval<zpt::lisp::object>(
                      std::string("(") + _name + std::string(" \"") +
                      zpt::ev::to_str(_performative) + std::string("\" \"") + _resource +
                      std::string("\" ") + zpt::lisp::to_lisp_string(_envelope) + std::string(")"));
                    _emitter->reply(_envelope, _bridge->from<zpt::lisp::object>(_ret));
                }));
          }

          _bridge->events()->on(_topic, _handlers, _opts);
          return zpt::lisp::object(ecl_make_bool(true));
      });
    _bridge->deflbd({ "name",
                      "zpt:route",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Routes RESTful resource requests",
                      "args",
                      { zpt::array,
                        { "type", "string", "label", "request performative" },
                        { "type", "string", "label", "request topic pattern" },
                        { "type", "object", "label", "envelope to send" },
                        { "type", "object", "label", "send options", "optional", true } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::performative _performative = zpt::ev::from_str(
                          std::string(_bridge->from<zpt::lisp::object>(_args[0])));
                        std::string _topic =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        zpt::json _payload = _bridge->from<zpt::lisp::object>(_args[2]);
                        zpt::json _opts = _bridge->from<zpt::lisp::object>(_args[3]);

                        zpt::json _result; // =
                        // _bridge->events()->sync_route(_performative, _topic, _payload,
                        // _opts);
                        return _bridge->to<zpt::lisp::object>(_result);
                    });
    _bridge->deflbd({ "name",
                      "zpt:split",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Splits a string by a given separator",
                      "args",
                      { zpt::array,
                        { "type", "string", "label", "the string to split" },
                        { "type", "string", "label", "the separator to split by" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        std::string _string =
                          std::string(_bridge->from<zpt::lisp::object>(_args[0]));
                        std::string _separator =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));

                        return _bridge->to<zpt::lisp::object>(zpt::split(_string, _separator));
                    });
    _bridge->deflbd(
      { "name",
        "zpt:join",
        "type",
        "internal",
        "access",
        "a",
        "label",
        "Joins a list with a given separator",
        "args",
        { zpt::array,
          { "type", "list", "label", "the list to be joined" },
          { "type", "string", "label", "the separator to join by", "optional", true } } },
      [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
          zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

          zpt::json _list = _bridge->from<zpt::lisp::object>(_args[0]);
          zpt::json _separator = _bridge->from<zpt::lisp::object>(_args[1]);
          if (_separator->ok()) {
              return _bridge->to<zpt::lisp::object>(
                zpt::json::string(zpt::join(_list, std::string(_separator))));
          }
          zpt::lisp::object _return =
            _bridge->to<zpt::lisp::object>(zpt::json::string(zpt::path::join(_list)));
          return _return;
      });
    _bridge->deflbd({ "name",
                      "zpt:merge",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Merges two JSON objects",
                      "args",
                      { zpt::array,
                        { "type", "any", "label", "the first object to be merged" },
                        { "type", "any", "label", "the first second to be merged" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _lhs = _bridge->from<zpt::lisp::object>(_args[0]);
                        zpt::json _rhs = _bridge->from<zpt::lisp::object>(_args[1]);

                        return _bridge->to<zpt::lisp::object>(_lhs + _rhs);
                    });
    _bridge->deflbd(
      { "name",
        "zpt:replace",
        "type",
        "internal",
        "access",
        "a",
        "label",
        "Replace a string with another, within a given string",
        "args",
        { zpt::array,
          { "type", "any", "label", "the string to search within" },
          { "type", "any", "label", "the string to search for" },
          { "type", "any", "label", "the string to replace with" } } },
      [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
          zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

          zpt::json _p_target = _bridge->from<zpt::lisp::object>(_args[0]);
          zpt::json _p_search = _bridge->from<zpt::lisp::object>(_args[1]);
          zpt::json _p_replace = _bridge->from<zpt::lisp::object>(_args[2]);

          return _bridge->to<zpt::lisp::object>(zpt::json::string(zpt::r_replace(
            std::string(_p_target), std::string(_p_search), std::string(_p_replace))));
      });
    _bridge->deflbd({ "name",
                      "zpt:topic-var",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Routes RESTful resource requests",
                      "args",
                      { zpt::array,
                        { "type", "array", "label", "the splited topic" },
                        { "type", "int", "label", "index for the topic part" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _topic = _bridge->from<zpt::lisp::object>(_args[0]);
                        size_t _index = size_t(_bridge->from<zpt::lisp::object>(_args[1]));

                        return _bridge->to<zpt::lisp::object>(_topic[_index]);
                    });
    _bridge->deflbd({ "name",
                      "zpt:authorize",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Validates authorization headers",
                      "args",
                      { zpt::array,
                        { "type", "string", "label", "the topic pattern" },
                        { "type", "object", "label", "the message envelope" },
                        { "type",
                          "array",
                          "label",
                          "aditional required scope (roles list)",
                          "optional",
                          true } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        std::string _topic =
                          std::string(_bridge->from<zpt::lisp::object>(_args[0]));
                        zpt::json _envelope = _bridge->from<zpt::lisp::object>(_args[1]);
                        zpt::json _roles = _bridge->from<zpt::lisp::object>(_args[2]);
                        zpt::json _identity =
                          _bridge->events()->authorize(_topic, _envelope, _roles);

                        return _bridge->to<zpt::lisp::object>(_identity);
                    });
    _bridge->deflbd(
      { "name",
        "zpt:authorization-headers",
        "type",
        "internal",
        "access",
        "a",
        "label",
        "Given the authorization identity, returns an authorization header",
        "args",
        { zpt::array, { "type", "object", "label", "the authorized identity" } } },
      [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
          zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

          zpt::json _identity = _bridge->from<zpt::lisp::object>(_args[0]);
          zpt::json _return = { "Authorization",
                                (std::string("Bearer ") + std::string(_identity["access_token"])) };

          return _bridge->to<zpt::lisp::object>(_return);
      });
    _bridge->deflbd({ "name",
                      "zpt:assertz-mandatory",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is present in the given object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test presence for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_mandatory(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-string",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type string in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_string(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-integer",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type integer in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_integer(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-double",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type double in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_double(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-timestamp",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type timestamp in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_timestamp(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-boolean",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type boolean in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_boolean(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-complex",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type complex in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_complex(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-object",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type object in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_object(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-array",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type array in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_array(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-int",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type int in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_int(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-uuid",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type uuid in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_uuid(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-utf8",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type utf8 in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_utf8(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-ascii",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type ascii in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_ascii(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:expect-hash",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type hash in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        expect_hash(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-token",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type token in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_token(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-uri",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type uri in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_uri(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-email",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type email in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_email(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:assertz-location",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Tests whether or not the given field is of type location in the given "
                      "object",
                      "args",
                      { zpt::array,
                        { "type", "object", "label", "the parent object to use" },
                        { "type", "string", "label", "the field to test type for" },
                        { "type", "int", "label", "the error code to return" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        zpt::json _object = _bridge->from<zpt::lisp::object>(_args[0]);
                        std::string _field =
                          std::string(_bridge->from<zpt::lisp::object>(_args[1]));
                        int _code = int(_bridge->from<zpt::lisp::object>(_args[2]));

                        assertz_location(_object, _field, _code);

                        return zpt::lisp::object(ecl_make_bool(true));
                    });
    _bridge->deflbd({ "name",
                      "zpt:generate-key",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Generates an alpha-numeric key with the provide length",
                      "args",
                      { zpt::array, { "type", "int", "label", "the resulting key length" } } },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        int _length = int(_bridge->from<zpt::lisp::object>(_args[0]));

                        return _bridge->to<zpt::lisp::object>(
                          zpt::json::string(zpt::generate::r_key(_length)));
                    });
    _bridge->deflbd({ "name",
                      "zpt:generate-uuid",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Generates a UUID" },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();

                        return _bridge->to<zpt::lisp::object>(
                          zpt::json::string(zpt::generate::r_uuid()));
                    });
    _bridge->deflbd(
      { "name",
        "zpt:json-date",
        "type",
        "internal",
        "access",
        "a",
        "label",
        "Returns a string with a timestamp format",
        "args",
        { zpt::array,
          { "type",
            "timestamp",
            "label",
            "The micro-second based timestamp to be "
            "transforme into a string (optional)",
            "optional",
            true } } },
      [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
          zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();
          zpt::json _arg = _bridge->from<zpt::lisp::object>(_args[0]);
          if (_arg->type() == zpt::JSString) {
              return _bridge->to<zpt::lisp::object>(zpt::json::date(std::string(_arg)));
          }
          else if (_arg->ok()) {
              unsigned long long int _ts = (unsigned long long int)_arg;
              return _bridge->to<zpt::lisp::object>(zpt::json::date((zpt::timestamp_t)_ts));
          }
          return _bridge->to<zpt::lisp::object>(zpt::json::date());
      });
    _bridge->deflbd({ "name",
                      "zpt:credentials",
                      "type",
                      "internal",
                      "access",
                      "a",
                      "label",
                      "Returns the container credentials" },
                    [](int _n_args, zpt::lisp::object _args[]) -> zpt::lisp::object {
                        zpt::bridge _bridge = zpt::bridge::instance<zpt::lisp::bridge>();
                        return _bridge->to<zpt::lisp::object>(_bridge->events()->credentials());
                    });
}

extern "C" auto
zpt_lisp() -> int {
    return 1;
}
