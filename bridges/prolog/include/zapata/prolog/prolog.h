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
    using mutex_type = zpt::locks::spin_mutex;

    bridge(std::string const& _cmd);
    bridge(bridge&& _rhs) = delete;
    virtual ~bridge() throw();

    auto operator=(bridge const& _rhs) -> zpt::prolog::bridge& = delete;
    auto operator=(bridge&& _rhs) -> zpt::prolog::bridge& = delete;

    /** @brief Returns "prolog". */
    auto name() const -> std::string;
    auto thread_instance() -> bridge&;
    /** @brief Loads a Prolog module from file. */
    auto setup_module(zpt::json _conf, std::string _external_path, bool _persist = true)
      -> zpt::prolog::bridge&;
    /** @brief Registers a C++ callback as a Prolog module. */
    auto setup_module(zpt::json _conf, callback_type _callback, bool _persist = true)
      -> zpt::prolog::bridge&;
    /** @brief Registers a C++ callback as a Prolog function. */
    auto setup_lambda(zpt::json _conf, lambda_type _callback) -> zpt::prolog::bridge&;
    /** @brief Locates a Prolog value by path. */
    auto find(zpt::json _to_locate) -> object_type;
    /** @brief Converts Prolog term to JSON. */
    auto to_json(object_type _to_convert) -> zpt::json;
    /** @brief Pushes JSON value onto Prolog term. */
    auto to_object(zpt::json _to_convert) -> object_type;
    /** @brief Executes a Prolog function with arguments. */
    auto execute(zpt::prolog::term _to_call) -> zpt::prolog::bridge::object_type;

  private:
    std::string __engine_args;
    bool __main_engine{ false };
    std::map<std::string, std::tuple<callback_type, zpt::json>> __builtin_to_load;
    std::map<std::string, zpt::json> __external_to_load;

    bridge(bridge const& _rhs);
    /** @brief Initializes the bridge (loads all modules). */
    auto initialize() -> zpt::prolog::bridge&;
    auto initialize_thread() -> zpt::prolog::bridge&;
};

/** @brief Converts Prolog term to JSON. */
auto to_json(term_t _to_convert) -> zpt::json;
/** @brief Converts JSON to Prolog term. */
auto to_object(zpt::json _to_convert) -> zpt::prolog::term;
auto get_name_arity(term_t _term) -> std::tuple<std::string, size_t>;
} // namespace prolog

/**
 * @brief Returns the global Prolog bridge instance.
 * @return Reference to the thread-local Prolog bridge.
 */
auto PROLOG_BRIDGE(std::string const& _cmd = "") -> zpt::prolog::bridge&;
} // namespace zpt
