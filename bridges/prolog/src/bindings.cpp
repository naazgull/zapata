/**
 * @file bindings.cpp
 * @brief SWI-Prolog foreign interface for Zapata.
 *
 * Implements five Prolog predicates: zpt_make_request/2, zpt_call/2,
 * zpt_config/1, zpt_log/1, and zpt_value_for/3. Each converts between
 * Prolog terms and JSON for seamless integration with the Zapata framework.
 *
 * @see install_libzapata_bridge_prolog_bindings
 */

#include <zapata/prolog/bindings.h>
#include <zapata/rest.h>
#include <zapata/runtime.h>
#include <zapata/transport/engine.h>

namespace {
extern "C" {
/**
 * @brief Creates a new HTTP request from a Prolog term.
 *
 * Converts a Prolog term containing the protocol name into a full HTTP request object,
 * then pushes the request onto the Prolog stack for further manipulation.
 *
 * @param _protocol_pl The Prolog term containing the request protocol (e.g., "http", "https")
 * @param _request_pl The Prolog term to store the created request
 * @return foreign_t 0 on success, throws exception on error
 *
 * @throws expect Failed if _request_pl is not a variable or _protocol_pl is not a string
 */
static auto make_request(term_t _protocol_pl /*+*/, term_t _request_pl /*-*/) -> foreign_t {
    expect(PL_term_type(_request_pl) == PL_VARIABLE,
           "`zpt_make_request`'s second parameter must be a variable");
    auto _protocol = zpt::prolog::to_json(_protocol_pl);
    expect(_protocol->is_string(), "`zpt_make_request`'s first parameter must be a string/atom");

    auto _request_js = zpt::TRANSPORT_LAYER() //
                         .get(_protocol->string())
                         ->make_request();
    auto _request = zpt::prolog::to_object(
      { "protocol", _protocol, "headers", _request_js->headers(), "uri", _request_js->uri() });

    return PL_unify_term(_request_pl, PL_TERM, *_request);
}

/**
 * @brief Sends an HTTP request and receives a response.
 *
 * Constructs an HTTP request from the provided Prolog term, sends it to the
 * configured transport layer, and waits for the response (with 20-second timeout).
 * The response is then pushed onto the Prolog stack.
 *
 * @param _request_pl The Prolog term containing the HTTP request parameters:
 *                     - protocol (string)
 *                     - performative (string, e.g., "GET", "POST")
 *                     - uri (JSON object)
 *                     - headers (JSON object)
 *                     - body (optional JSON object)
 * @param _reply_pl The Prolog term to store the HTTP response:
 *                  - status (integer)
 *                  - headers (JSON object)
 *                  - body (optional JSON object)
 * @return foreign_t 0 on success, throws exception on error
 *
 * @throws expect Failed if _reply_pl is not a variable or request is malformed
 */
static auto send_request(term_t _request_pl /*+*/, term_t _reply_pl /*-*/) -> foreign_t {
    expect(PL_term_type(_reply_pl) == PL_VARIABLE,
           "`zpt_call`'s second parameter must be a variable");
    auto _request_js = zpt::prolog::to_json(_request_pl);
    expect(_request_js->is_object(),
           "`zpt_make_request`'s first parameter must be a term convertible to JSON");

    auto _transport = zpt::TRANSPORT_LAYER().get(_request_js("protocol")->string());
    auto _request = _transport->make_request();
    _request //
      ->performative(zpt::ontology::from_str(_request_js("performative")->string()))
      .uri(_request_js("uri"))
      .headers() = _request_js("headers");

    if (_request_js("body")->ok()) { _request->body() = _request_js("body"); }

    if (_transport->has_capability(zpt::transport_capability::SYNCHRONOUS)) {
        static constexpr long long _timeout{ 20 };
        auto _start = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count();
        auto _context = zpt::make_call<>(zpt::REST_RESOLVER(), _request);
        while (_context->state() <= zpt::CALL_STATE_SENT) {
            auto _lap = std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count();
            if (_lap - _start > _timeout) {
                auto _reply = zpt::prolog::to_object({ "status", 408 });
                return PL_unify_term(_reply_pl, PL_TERM, *_reply);
            }
            std::this_thread::sleep_for(std::chrono::microseconds{ 100 });

            if (zpt::runtime::is_in_shutdown()) { return false; }
        }

        auto _reply_js = _context->reply();
        auto _reply = zpt::prolog::to_object({ "status", //
                                               _reply_js->status(),
                                               "headers",
                                               _reply_js->headers(),
                                               "body",
                                               _reply_js->body() });

        return PL_unify_term(_reply_pl, PL_TERM, *_reply);
    }
    else {
        zpt::make_call(zpt::REST_RESOLVER(), _request);
        return true;
    }
}

/**
 * @brief Pushes the global Zapata configuration onto the Prolog stack.
 *
 * Retrieves the global configuration from Zapata and converts it to a Prolog term,
 * then pushes it onto the Prolog stack. If no global configuration is available,
 * returns a default configuration with Prolog library information.
 *
 * @param _config_pl The Prolog term to store the configuration
 * @return foreign_t 0 on success, throws exception on error
 *
 * @throws expect Failed if _config_pl is not a variable
 */
static auto get_config(term_t _config_pl /*-*/) -> foreign_t {
    expect(PL_term_type(_config_pl) == PL_VARIABLE, "`zpt_config`'s parameter must be a variable");

    auto _config = zpt::prolog::to_object(zpt::GLOBAL_CONFIG()->size() != 0
                                            ? zpt::GLOBAL_CONFIG()
                                            : zpt::json{ "prolog", { "lib", "SWI Prolog" } });

    return PL_unify_term(_config_pl, PL_TERM, *_config);
}

/**
 * @brief Sets or unifies the global Prolog variable identified by the first parameter with the
 * second parameter.
 *
 * @param _global_key_pl The identifier of the global
 * @param _global_value_pl The Prolog term to unify with the value of the given global
 * @return foreign_t 1 on success, throws exception on error
 *
 * @throws expect Failed if _global_key_pl is not a atom or string
 */
static auto global(term_t _global_key_pl /*+*/, term_t _global_value_pl /*?*/) -> foreign_t {
    expect(PL_term_type(_global_key_pl) == PL_ATOM || PL_term_type(_global_key_pl) == PL_STRING,
           "`zpt_global`'s first parameter must be a string or atom");

    auto& _global = zpt::PROLOG_GLOBALS();
    auto _global_key = zpt::prolog::to_json(_global_key_pl);
    if (PL_term_type(_global_value_pl) == PL_VARIABLE) {
        std::shared_lock _guard{ _global.mutex() };
        auto _global_value = zpt::prolog::to_object((*_global)->get_path(_global_key->string()));
        return PL_unify_term(_global_value_pl, PL_TERM, *_global_value);
    }
    else {
        std::unique_lock _guard{ _global.mutex() };
        auto _global_value = zpt::prolog::to_json(_global_value_pl);
        (*_global)->set_path(_global_key->string(), _global_value);
    }
    return 1;
}

/**
 * @brief Logs a message to the Zapata logging system.
 *
 * Converts the Prolog term to a JSON representation and logs it using
 * Zapata's logging system. Accepts either a single string/atom or a list
 * of terms, converting them to a single log message.
 *
 * @param _to_log The Prolog term to log - either a string/atom or a list of terms
 * @return foreign_t 1 on success
 *
 * @throws expect Failed if _to_log is a variable
 *
 * Example Prolog usage:
 * @code
 * ?- zpt_log("Application started").
 * ?- zpt_log([Debug, "Connection established", Info, "Processing request"]).
 * @endcode
 */
static auto send_to_log(term_t _to_log /*+*/) -> foreign_t {
    expect(PL_term_type(_to_log) != PL_VARIABLE,
           "`zpt_config`'s second parameter must NOT be a variable");

    auto _args = zpt::prolog::to_json(_to_log);
    if (_args->type() == zpt::JSArray) {
        std::ostringstream _oss;
        for (auto&& [_, __, _value] : _args) { _oss << static_cast<std::string>(_value); }
        _oss << std::flush;
        zlog(_oss.str(), static_cast<zpt::LogLevel>(zpt::log_lvl));
    }
    else { zlog(static_cast<std::string>(_args), static_cast<zpt::LogLevel>(zpt::log_lvl)); }

    return 1;
}

/**
 * @brief Retrieves a value from a Prolog term by key.
 *
 * Searches through a Prolog term (list or compound) and extracts the value
 * associated with a given key. Supports searching in both lists (by integer index)
 * and compound terms (by string key).
 *
 * @param _to_search_pl The Prolog term to search in - a list or compound term
 * @param _key_pl The key to search for - integer index for lists, string key for compounds
 * @param _result_pl The Prolog term to store the result (may be unbound)
 * @return foreign_t 0 if key not found, 1 on success with result
 *
 * @throws expect Failed if key or search term are variables
 */
static auto get_value_for_key(term_t _to_search_pl /*+*/,
                              term_t _key_pl /*+*/,
                              term_t _result_pl /*?*/) -> foreign_t {
    expect(PL_term_type(_key_pl) != PL_VARIABLE,
           "`zpt_key`'s first parameter must NOT be a variable");
    expect(PL_term_type(_to_search_pl) != PL_VARIABLE,
           "`zpt_key`'s second parameter must NOT be a variable");

    auto _to_search = zpt::prolog::to_json(_to_search_pl);
    if (_to_search->type() == zpt::JSArray) {
        auto _key = zpt::prolog::to_json(_key_pl);
        expect(_key->is_integer(), "key must be an integer in order to search in a list");
        auto _value = _to_search(static_cast<size_t>(_key));
        if (_value->ok()) {
            auto _result = zpt::prolog::to_object(_value);
            return PL_unify_term(_result_pl, PL_TERM, *_result);
        }
    }
    else if (_to_search->type() == zpt::JSObject) {
        auto _key = zpt::prolog::to_json(_key_pl);
        expect(_key->is_string(), "key must be a string in order to search in a compound");
        auto _value = _to_search->get_path(_key->string());
        if (_value->ok()) {
            auto _result = zpt::prolog::to_object(_value);
            return PL_unify_term(_result_pl, PL_TERM, *_result);
        }
    }

    return 0;
}

/**
 * @brief Retrieves whether or not the system is in shutdown.
 *
 * @return bool True if the system is in shutdown.
 */
static auto is_in_shutdown(term_t _result_pl /*?*/) -> foreign_t {
    auto _result = zpt::prolog::to_object(zpt::runtime::is_in_shutdown());
    return PL_unify_term(_result_pl, PL_TERM, *_result);
}
}
} // namespace

/**
 * @brief SWI-Prolog library initializer that registers all `zpt_*` foreign predicates.
 *
 * Registers all Zapata Prolog bridge foreign functions with the SWI-Prolog engine.
 * This function should be called by the Prolog library initialization code to make
 * the `zpt_*` predicates available to Prolog programs.
 *
 * @return install_t SWI-Prolog install_t handle
 *
 * Registered predicates:
 * - zpt_make_request/2 - Create HTTP request from protocol
 * - zpt_call/2 - Send HTTP request and receive response
 * - zpt_config/1 - Get global Zapata configuration
 * - zpt_log/1 - Log message via Zapata logging system
 * - zpt_value_for/3 - Get value by key from list or compound term
 */
extern "C" auto install_libzapata_bridge_prolog_bindings() -> install_t {
    PL_register_foreign("zpt_make_request", 2, (void*)::make_request, 0);
    PL_register_foreign("zpt_call", 2, (void*)::send_request, 0);
    PL_register_foreign("zpt_config", 1, (void*)::get_config, 0);
    PL_register_foreign("zpt_global", 2, (void*)::global, 0);
    PL_register_foreign("zpt_log", 1, (void*)::send_to_log, 0);
    PL_register_foreign("zpt_value_for", 3, (void*)::get_value_for_key, 0);
    PL_register_foreign("zpt_is_in_shutdown", 1, (void*)::is_in_shutdown, 0);
}
