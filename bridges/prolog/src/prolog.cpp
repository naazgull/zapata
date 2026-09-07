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

#include <zapata/base/sentry.h>
#include <zapata/prolog/prolog.h>

zpt::prolog::bridge::bridge(std::string const& _cmd)
  : __engine_args{ _cmd }
  , __main_engine{ true } {
    this->initialize();
}

zpt::prolog::bridge::~bridge() throw() {
    if (!this->__main_engine) {
        zlog("Detaching Prolog engine from " << zpt::this_thread::name(), zpt::debug);
        PL_thread_destroy_engine();
    }
}

auto zpt::prolog::bridge::name() const -> std::string { return "prolog"; }

auto zpt::prolog::bridge::thread_instance() -> bridge& {
    static thread_local zpt::prolog::bridge _return{ *this };
    return _return;
}

auto zpt::prolog::bridge::setup_module(zpt::json _conf, std::string _external_path, bool _persist)
  -> zpt::prolog::bridge& {
    static atom_t _erase = PL_new_atom("erase");

    zpt::prolog::term _setup{ R"(
        asserta(
            (user:thread_message_hook(Term, error, _Lines) :-
                    zpt_consult_log(Term)
            ),
            Ref
        )
    )" };
    zpt::prolog::term _ref;
    PL_get_arg(2, _setup, *_ref);

    zpt::prolog::term _call{ std::format("consult('{}')", _external_path) };

    zpt::prolog::term _cleanup;
    expect(PL_put_functor(*_cleanup, PL_new_functor(_erase, 1)),
           "couldn't add functor to Prolog term");
    expect(PL_unify_arg(1, *_cleanup, *_ref), "couldn't unify `Ref` in erase");

    term_t _args = PL_new_term_refs(3);
    expect(PL_put_term(_args + 0, *_setup), "unable to set argument 1 in `setup_call_cleanup`");
    expect(PL_put_term(_args + 1, *_call), "unable to set argument 2 in `setup_call_cleanup`");
    expect(PL_put_term(_args + 2, *_cleanup), "unable to set argument 3 in `setup_call_cleanup`");

    predicate_t _setup_call_cleanup = PL_predicate("setup_call_cleanup", 3, nullptr);
    qid_t _qid = PL_open_query(nullptr, PL_Q_PASS_EXCEPTION, _setup_call_cleanup, _args);

    auto _result = PL_next_solution(_qid);
    PL_close_query(_qid);
    PL_free_term_ref(_args + 2);
    PL_free_term_ref(_args + 1);
    PL_free_term_ref(_args + 0);

    if (_result == PL_S_FALSE) {
        auto _term = PL_exception(0);
        if (_term != 0) {
            auto _message = zpt::prolog::term_to_string(_term);
            PL_clear_exception();
            throw zpt::exception{ _message };
        }
        expect(_result != PL_S_FALSE, "unable to properly load `" << _external_path << "`");
    }

    auto& _global = zpt::PROLOG_GLOBALS();
    std::unique_lock _guard{ _global.mutex() };
    if ((*_global)("consult_log")(zpt::this_thread::name())->is_array()) {
        std::ostringstream _oss;
        _oss << "errors found while loading `" << _external_path << "`:";
        for (auto&& [_idx, __, _message] : (*_global)("consult_log")(zpt::this_thread::name())) {
            _oss << "\n    " << (_idx + 1) << ". " << _message;
        }
        _oss << std::flush;
        (*_global)["consult_log"]->object()->pop(zpt::this_thread::name());
        throw zpt::exception{ _oss.str() };
    }

    if (_persist) {
        zlog("Prolog: loading module " << _conf("module") << " from " << _external_path, zpt::info);
        this->__external_to_load.insert(std::make_pair(_external_path, _conf));
    }

    return (*this);
}

auto zpt::prolog::bridge::setup_module(zpt::json _conf, callback_type _callback, bool _persist)
  -> zpt::prolog::bridge& {
    _callback();
    if (_persist) {
        zlog("Prolog: loading builtin module " << _conf("module"), zpt::info);
        this->__builtin_to_load.insert(
          std::make_pair(_conf("module")->string(), std::make_tuple(_callback, _conf)));
    }
    return (*this);
}

auto zpt::prolog::bridge::unload_module(std::string _external_path) -> zpt::prolog::bridge& {
    zpt::prolog::term _file;
    PL_put_atom_chars(*_file, _external_path.data());

    predicate_t _unload_file = PL_predicate("unload_file", 1, nullptr);
    qid_t _qid = PL_open_query(nullptr, PL_Q_NORMAL, _unload_file, _file);
    zpt::sentry _cleanup{ [_qid]() { PL_close_query(_qid); } };

    expect(PL_next_solution(_qid) != PL_S_FALSE,
           "unable to properly unload `" << _external_path << "`");

    this->__external_to_load.erase(_external_path);

    return (*this);
}

auto zpt::prolog::bridge::setup_lambda(zpt::json, lambda_type) -> zpt::prolog::bridge& {
    bool _supported{ false };
    expect(_supported, "lambda register not yet supported in Prolog bridge");
    return (*this);
}

auto zpt::prolog::bridge::find(zpt::json) -> object_type {
    bool _applicable{ false };
    expect(_applicable, "object search doesn't apply to Prolog bridge");
    return zpt::prolog::term::null();
}

auto zpt::prolog::bridge::to_json(object_type _to_convert) -> zpt::json {
    if (_to_convert == zpt::prolog::term::null()) { return zpt::undefined; }
    return zpt::prolog::to_json(*_to_convert);
}

auto zpt::prolog::bridge::to_object(zpt::json _to_convert) -> object_type {
    return zpt::prolog::to_object(_to_convert);
}

auto zpt::prolog::bridge::execute(zpt::prolog::term _to_call) -> zpt::prolog::bridge::object_type {
    expect(PL_term_type(_to_call) == PL_TERM && PL_is_compound(_to_call),
           "term parameter must be a compound");
    auto&& [_functor, _arity] = zpt::prolog::get_name_arity(_to_call);
    if (_functor == "," && _arity == 2) {
        zpt::prolog::term _goal;
        PL_get_arg(1, _to_call, *_goal);
        zpt::prolog::term _template;
        PL_get_arg(2, _to_call, *_template);

        term_t _args = PL_new_term_refs(3);
        expect(PL_put_term(_args + 0, *_template), "unable to set argument 1 in `findall`");
        expect(PL_put_term(_args + 1, *_goal), "unable to set argument 2 in `findall`");

        predicate_t _findall = PL_predicate("findall", 3, nullptr);
        qid_t _qid = PL_open_query(nullptr, PL_Q_PASS_EXCEPTION, _findall, _args);
        auto _cleanup = [_args, _qid]() {
            PL_close_query(_qid);
            PL_free_term_ref(_args + 2);
            PL_free_term_ref(_args + 1);
            PL_free_term_ref(_args + 0);
        };

        if (PL_next_solution(_qid) != PL_S_FALSE) {
            auto _record = PL_record(_args + 2);
            _cleanup();
            zpt::prolog::term _return;
            PL_recorded(_record, *_return);
            PL_erase(_record);
            return _return;
        }
        else {
            _cleanup();
            auto _term = PL_exception(0);
            if (_term != 0) {
                auto _message = zpt::prolog::term_to_string(_term);
                PL_clear_exception();
                throw zpt::exception{ _message };
            }
        }
    }
    else {
        term_t _args = PL_new_term_refs(1);
        expect(PL_put_term(_args + 0, *_to_call), "unable to set argument 1 in `call`");

        predicate_t _call = PL_predicate("call", 1, nullptr);
        qid_t _qid = PL_open_query(nullptr, PL_Q_PASS_EXCEPTION, _call, _args);
        auto _result = PL_next_solution(_qid);
        PL_close_query(_qid);
        PL_free_term_ref(_args + 0);

        if (_result == PL_S_FALSE) {
            auto _term = PL_exception(0);
            if (_term != 0) {
                auto _message = zpt::prolog::term_to_string(_term);
                PL_clear_exception();
                throw zpt::exception{ _message };
            }
        }

        zpt::prolog::term _return;
        expect(PL_put_integer(*_return, _result), "couldn't add integer to Prolog term");
        return _return;
    }

    return zpt::prolog::term::null();
}

auto zpt::prolog::bridge::cleanup() -> zpt::prolog::bridge& {
    this->__builtin_to_load.clear();
    this->__external_to_load.clear();
    return (*this);
}

zpt::prolog::bridge::bridge(bridge const& _rhs)
  : __engine_args{ _rhs.__engine_args }
  , __builtin_to_load{ _rhs.__builtin_to_load }
  , __external_to_load{ _rhs.__external_to_load } {
    this->set_options(_rhs.options());
    this->initialize_thread();
    this->cleanup();
}

auto zpt::prolog::bridge::initialize() -> zpt::prolog::bridge& {
    char* _arg = const_cast<char*>(this->__engine_args.data());
    expect(PL_initialise(1, &_arg), "couldn't initialise Prolog engine");
    auto& _global = zpt::PROLOG_GLOBALS();
    std::unique_lock _guard{ _global.mutex() };
    (*_global)["consult_log"] = zpt::json::object();
    return (*this);
}

auto zpt::prolog::bridge::initialize_thread() -> zpt::prolog::bridge& {
    PL_thread_attach_engine(nullptr);

    for (auto&& [_, _pair] : this->__builtin_to_load) {
        auto [_callback, _conf] = _pair;
        this->setup_module(_conf, _callback, false);
    }
    for (auto&& [_file, _conf] : this->__external_to_load) {
        this->setup_module(_conf, _file, false);
    }

    return (*this);
}

auto zpt::prolog::to_json(term_t _to_convert) -> zpt::json {
    bool _supported_type{ false };

    switch (PL_term_type(_to_convert)) {
        case PL_VARIABLE: {
            expect(_supported_type, "unsupported type PL_VARIABLE");
            break;
        }
        case PL_NIL: {
            return zpt::undefined;
        }
        case PL_BLOB: {
            expect(_supported_type, "unsupported type PL_VARIABLE");
            break;
        }
        case PL_ATOM:
        case PL_STRING: {
            char* _buffer{ nullptr };
            expect(PL_get_chars(_to_convert, &_buffer, CVT_ATOM | CVT_STRING | BUF_MALLOC),
                   "counldn't extract the string from the term");
            auto _str = zpt::json::string(std::string{ const_cast<char const*>(_buffer) });
            PL_free(_buffer);
            return _str;
        }
        case PL_INTEGER: {
            std::int64_t _int{ 0 };
            expect(PL_get_int64(_to_convert, &_int), "couldn't extract the integer from the term");
            return zpt::json::integer(_int);
        }
        case PL_RATIONAL:
        case PL_FLOAT: {
            double _floating{ 0 };
            expect(PL_get_float(_to_convert, &_floating),
                   "couldn't extract the float from the term");
            return zpt::json::floating(_floating);
        }
        case PL_TERM: {
            if (PL_is_compound(_to_convert)) {
                auto&& [_functor, _arity] = zpt::prolog::get_name_arity(_to_convert);
                if (_functor != "," && _functor != ":") {
                    return zpt::json::string(zpt::prolog::term_to_string(_to_convert));
                }

                auto _return = zpt::json::object();

                if (_functor == ":") {
                    zpt::prolog::term _term1;
                    PL_get_arg(1, _to_convert, *_term1);
                    auto _name = zpt::prolog::to_json(*_term1);
                    zpt::prolog::term _term2;
                    PL_get_arg(2, _to_convert, *_term2);
                    auto _value = zpt::prolog::to_json(*_term2);
                    _return << _name->string() << _value;
                }
                if (_functor == ",") {
                    for (size_t _idx = 1; _idx != _arity + 1; ++_idx) {
                        zpt::prolog::term _term;
                        PL_get_arg(_idx, _to_convert, *_term);
                        auto _element = zpt::prolog::to_json(*_term);
                        expect(_element->type() == zpt::JSObject,
                               "invalid Prolog, JSON object type expected");
                        _return += _element;
                    }
                }

                return _return;
            }
            break;
        }
        case PL_LIST_PAIR: {
            zpt::prolog::term _head;
            term_t _tail = PL_copy_term_ref(_to_convert);
            auto _return = zpt::json::array();
            while (PL_get_list_ex(_tail, _head, _tail)) { _return << zpt::prolog::to_json(_head); }
            return _return;
        }
        case PL_DICT: {
            break;
        }
    }

    return zpt::undefined;
}

auto zpt::prolog::to_object(zpt::json _to_convert) -> zpt::prolog::term {
    static atom_t _colon = PL_new_atom(":");
    static atom_t _comma = PL_new_atom(",");

    switch (_to_convert->type()) {
        case zpt::JSObject: {
            zpt::prolog::term _term;
            size_t _idx{ 0 };
            for (auto&& [_, _key, _value] : _to_convert) {
                zpt::prolog::term _pair;
                expect(PL_put_functor(*_pair, PL_new_functor(_colon, 2)),
                       "couldn't add functor to Prolog term");
                auto _p_key = zpt::prolog::to_object(zpt::json::string(_key));
                auto _p_value = zpt::prolog::to_object(_value);
                _pair //
                  .add(_p_key)
                  .add(_p_value);
                expect(PL_unify_arg(1, *_pair, *_p_key), "couldn't add `:` key to Prolog term");
                expect(PL_unify_arg(2, *_pair, *_p_value), "couldn't add `:` value to Prolog term");

                if (_idx == 0) { _term = _pair; }
                else {
                    zpt::prolog::term _chain;
                    expect(PL_put_functor(*_chain, PL_new_functor(_comma, 2)),
                           "couldn't add functor to Prolog term");
                    _chain //
                      .add(_pair)
                      .add(_term);
                    expect(PL_unify_arg(1, *_chain, *_pair),
                           "couldn't add `,` first argument to Prolog term");
                    expect(PL_unify_arg(2, *_chain, *_term),
                           "couldn't add `,` second argument to Prolog term");
                    _term = _chain;
                }
                ++_idx;
            }
            return _term;
        }
        case zpt::JSArray: {
            if (_to_convert->size() == 0) {
                zpt::prolog::term _array;
                PL_put_nil(*_array);
                return _array;
            }

            zpt::prolog::term _tail;
            PL_put_nil(*_tail);
            for (size_t _idx = _to_convert->size(); _idx != 0; --_idx) {
                auto _head = zpt::prolog::to_object(_to_convert(_idx - 1));
                _tail //
                  .add(_head);
                zpt::prolog::term _new_tail;
                expect(PL_cons_list(*_new_tail, *_head, *_tail), "couldn't construct list");
                expect(PL_put_term(*_tail, *_new_tail), "couldn't copy the tail content");
            }
            return _tail;
        }
        case zpt::JSString: {
            zpt::prolog::term _string;
            expect(PL_put_string_chars(*_string, _to_convert->string().data()),
                   "couldn't add string to Prolog term");
            return _string;
        }
        case zpt::JSInteger: {
            zpt::prolog::term _integer;
            expect(PL_put_integer(*_integer, _to_convert->integer()),
                   "couldn't add integer to Prolog term");
            return _integer;
        }
        case zpt::JSDouble: {
            zpt::prolog::term _float;
            expect(PL_put_float(*_float, _to_convert->floating()),
                   "couldn't add float to Prolog term");
            return _float;
        }
        case zpt::JSBoolean: {
            zpt::prolog::term _boolean;
            expect(PL_put_atom_chars(*_boolean, _to_convert->boolean() ? "true" : "false"),
                   "couldn't add boolean atom to Prolog term");
            return _boolean;
        }
        case zpt::JSUndefined:
        case zpt::JSNil: {
            zpt::prolog::term _nil;
            PL_put_nil(*_nil);
            return _nil;
        }
        case zpt::JSDate: {
            zpt::prolog::term _date;
            expect(PL_put_string_chars(*_date, static_cast<std::string>(_to_convert).data()),
                   "couldn't add date string to Prolog term");
            return _date;
        }
        case zpt::JSLambda:
        case zpt::JSRegex: {
            expect(_to_convert->type() != zpt::JSLambda, "can't convert type JSON lambda");
            expect(_to_convert->type() != zpt::JSRegex, "can't convert type JSON regexp");
        }
    }

    return zpt::prolog::term::null();
}

auto zpt::prolog::get_name_arity(term_t _term) -> std::tuple<std::string, size_t> {
    atom_t _name{ 0 };
    size_t _arity{ 0 };
    expect(PL_get_name_arity(_term, &_name, &_arity),
           "couldn't get name and arity from the compound term");
    size_t _{ 0 };
    return { PL_atom_nchars(_name, &_), _arity };
}

auto zpt::PROLOG_BRIDGE(std::string const& _cmd) -> zpt::prolog::bridge& {
    static zpt::prolog::bridge _bridge{ _cmd };
    return _bridge;
}

auto zpt::PROLOG_GLOBALS() -> zpt::safe_access<zpt::json, zpt::locks::spin_mutex>& {
    static zpt::safe_access<zpt::json, zpt::locks::spin_mutex> _global{ zpt::json::object() };
    return _global;
}
