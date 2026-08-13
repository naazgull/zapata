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

/**
 * @file prolog.h
 * @brief Prolog bridge implementation (stub).
 *
 * @see zpt::programming::bridge
 */

#pragma once

#include <SWI-cpp2.h>
#include <zapata/base/safe_access.h>
#include <zapata/bridge.h>
#include <zapata/prolog/helpers.h>

namespace zpt {

using prolog_object = zpt::prolog::term;

namespace prolog {
/**
 * @brief Prolog scripting language bridge.
 *
 * Integrates Prolog with Zapata, providing:
 * - Loading Prolog scripts from files
 * - Calling Prolog functions from C++
 * - Registering C++ callbacks callable from Prolog
 * - Automatic JSON/Prolog value conversion
 *
 */
class bridge : public zpt::programming::bridge<zpt::prolog::bridge, zpt::prolog_object> {
  public:
    using callback_type = std::function<void()>; ///< C++ callback for Prolog
    using lambda_type = std::function<int()>;    ///< Lambda as Prolog C function
    using mutex_type = zpt::locks::spin_mutex;   ///< Mutex type for thread synchronization

    /** @brief Constructs the bridge and initializes the Prolog engine.
     * @param _cmd Command-line arguments for the Prolog engine.
     * @return void (constructors implicitly initialize the object). */
    bridge(std::string const& _cmd);
    bridge(bridge&& _rhs) = delete;
    /** @brief Destroys the bridge, detaching from the Prolog engine if not the main instance.
     * @return void (destructors implicitly clean up the object). */
    virtual ~bridge() throw();

    auto operator=(bridge const& _rhs) -> zpt::prolog::bridge& = delete;
    auto operator=(bridge&& _rhs) -> zpt::prolog::bridge& = delete;

    /** @brief Returns "prolog".
     * @return The name of this bridge. */
    auto name() const -> std::string;
    /** @brief Returns a thread-local copy of the bridge instance.
     * @return Reference to thread-local bridge instance. */
    auto thread_instance() -> bridge&;
    /** @brief Loads a Prolog module from file.
     * @param _conf Configuration JSON.
     * @param _external_path External file path to load.
     * @param _persist Whether to persist the module.
     * @return Reference to this bridge. */
    auto setup_module(zpt::json _conf, std::string _external_path, bool _persist = true)
      -> zpt::prolog::bridge&;
    /** @brief Registers a C++ callback as a Prolog module.
     * @param _conf Configuration JSON.
     * @param _callback C++ callback to register.
     * @param _persist Whether to persist the module.
     * @return Reference to this bridge. */
    auto setup_module(zpt::json _conf, callback_type _callback, bool _persist = true)
      -> zpt::prolog::bridge&;
    /** @brief Registers a C++ callback as a Prolog function.
     * @param _conf Configuration JSON.
     * @param _callback Lambda callback to register.
     * @return Reference to this bridge. */
    auto setup_lambda(zpt::json _conf, lambda_type _callback) -> zpt::prolog::bridge&;
    /** @brief Locates a Prolog value by path.
     * @param _to_locate JSON value with path.
     * @return Prolog object at the given path. */
    auto find(zpt::json _to_locate) -> object_type;
    /** @brief Converts Prolog term to JSON.
     * @param _to_convert Prolog term to convert.
     * @return JSON representation of the term. */
    auto to_json(object_type _to_convert) -> zpt::json;
    /** @brief Pushes JSON value onto Prolog term.
     * @param _to_convert JSON value to push.
     * @return Prolog term containing the JSON value. */
    auto to_object(zpt::json _to_convert) -> object_type;
    /** @brief Executes a Prolog function with arguments.
     * @param _to_call Prolog term to execute.
     * @return Result of the execution. */
    auto execute(zpt::prolog::term _to_call) -> zpt::prolog::bridge::object_type;
    /** @brief Clears all loaded modules and resets the bridge state.
     * @return Reference to this bridge. */
    auto cleanup() -> zpt::prolog::bridge&;

  private:
    std::string __engine_args;   ///< Command-line arguments for the Prolog engine
    bool __main_engine{ false }; ///< Whether this instance owns the main Prolog engine
    std::map<std::string, std::tuple<callback_type, zpt::json>>
      __builtin_to_load;                                 ///< Built-in modules to register
    std::map<std::string, zpt::json> __external_to_load; ///< External file modules to load

    /** @brief Copy constructor for creating a new thread-local bridge instance.
     * @param _rhs Bridge instance to copy from. */
    bridge(bridge const& _rhs);
    /** @brief Initializes the bridge (loads all modules).
     * @return Reference to this bridge. */
    auto initialize() -> zpt::prolog::bridge&;
    /** @brief Attaches the current thread to the Prolog engine and loads all modules.
     * @return Reference to this bridge. */
    auto initialize_thread() -> zpt::prolog::bridge&;
};

/** @brief Converts Prolog term to JSON.
 * @param _to_convert Prolog term to convert.
 * @return JSON representation of the term. */
auto to_json(term_t _to_convert) -> zpt::json;
/** @brief Converts JSON to Prolog term.
 * @param _to_convert JSON value to convert.
 * @return Prolog term containing the JSON value. */
auto to_object(zpt::json _to_convert) -> zpt::prolog::term;
/** @brief Extracts the functor name and arity from a Prolog compound term.
 * @param _term Prolog compound term.
 * @return Tuple of (functor name, arity). */
auto get_name_arity(term_t _term) -> std::tuple<std::string, size_t>;
} // namespace prolog

/**
 * @brief Retrieves the global Prolog bridge instance.
 * @return Reference to the thread-local Prolog bridge.
 */
auto PROLOG_BRIDGE(std::string const& _cmd = "") -> zpt::prolog::bridge&;
/**
 * @brief Retrieves the Prolog bridge globals.
 * @return Reference to the Prolog bridge globals.
 */
auto PROLOG_GLOBALS() -> zpt::safe_access<zpt::json, zpt::locks::spin_mutex>&;
} // namespace zpt
