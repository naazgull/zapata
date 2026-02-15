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
 * @file bridge.h
 * @brief Core bridge template for language integration.
 *
 * Provides the CRTP base template for implementing language bridges.
 * Derived classes must implement the required interface methods.
 */

#pragma once

#include <zapata/json.h>

namespace zpt {
namespace programming {

/**
 * @brief Abstract base for language integrations.
 */
class integration {
  public:
    integration() = default;
    virtual ~integration() = default;

    /** @brief Returns the integration name (e.g., "lua", "prolog"). */
    virtual auto name() const -> std::string = 0;
};

/**
 * @brief CRTP base template for language bridges.
 *
 * Provides a common interface for integrating scripting languages.
 * Derived classes implement the actual language-specific operations.
 *
 * @tparam C The concrete bridge class (CRTP).
 * @tparam O The language's native object type.
 *
 * @par Required Methods for Derived Class
 * The derived class C must implement:
 * - `setup_module(zpt::json, std::string)` - Load external module
 * - `setup_module(zpt::json, Callback)` - Register callback module
 * - `setup_lambda(zpt::json, Lambda)` - Register lambda
 * - `initialize()` - Initialize the bridge
 * - `find(zpt::json)` - Locate an object by path
 * - `to_object(zpt::json)` - Convert JSON to native object
 * - `to_json(O)` - Convert native object to JSON
 * - `execute(Term, Args...)` - Execute a function
 *
 * @par Example
 * @code
 * auto& lua = zpt::LUA_BRIDGE();
 * lua.add_module("my_module.lua");
 * lua.init();
 * auto result = lua.call({ "function", "my_func" }, { "arg1", 42 });
 * @endcode
 */
template<typename C, typename O>
class bridge : public zpt::programming::integration {
  public:
    using class_type = C;    ///< The concrete bridge type
    using object_type = O;   ///< The language's native object type

    bridge() = default;
    bridge(bridge<C, O> const& _rhs) = delete;
    bridge(bridge<C, O>&& _rhs) = delete;
    virtual ~bridge() = default;

    auto operator=(bridge<C, O> const& _rhs) = delete;
    auto operator=(bridge<C, O>&& _rhs) = delete;

    /** @brief Sets bridge configuration options. */
    auto set_options(zpt::json _conf) -> bridge<C, O>&;
    /** @brief Returns current configuration options. */
    auto options() const -> zpt::json;

    /** @brief Adds an external module from a file path. */
    auto add_module(std::string _external_path, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    /** @brief Adds a module via callback. */
    template<typename Callback>
    auto add_module(Callback _callback, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    /** @brief Registers a lambda function. */
    template<typename Lambda>
    auto add_lambda(Lambda _lambda, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    /** @brief Initializes the bridge after configuration. */
    auto init() -> bridge<C, O>&;

    /** @brief Locates an object by path. */
    auto locate(zpt::json _to_locate) -> object_type;

    /** @brief Converts JSON to native object. */
    auto json_to_object(zpt::json _to_convert) -> object_type;
    /** @brief Converts native object to JSON. */
    auto object_to_json(object_type _to_convert) -> zpt::json;

    /** @brief Calls a function and returns the result as JSON. */
    template<typename Term, typename... Args>
    auto call(Term _to_call, Args... _arg) -> zpt::json;

  private:
    zpt::json __options;
};
} // namespace programming
} // namespace zpt

template<typename C, typename O>
auto zpt::programming::bridge<C, O>::set_options(zpt::json _conf)
  -> zpt::programming::bridge<C, O>& {
    this->__options = _conf;
    return (*this);
}

template<typename C, typename O>
auto zpt::programming::bridge<C, O>::options() const -> zpt::json {
    return this->__options;
}

template<typename C, typename O>
auto zpt::programming::bridge<C, O>::add_module(std::string _external_path, zpt::json _conf)
  -> zpt::programming::bridge<C, O>& {
    static_cast<C*>(this)->setup_module(_conf, _external_path);
    return (*this);
}

template<typename C, typename O>
template<typename Callback>
auto zpt::programming::bridge<C, O>::add_module(Callback _callback, zpt::json _conf)
  -> zpt::programming::bridge<C, O>& {
    static_cast<C*>(this)->setup_module(_conf, _callback);
    return (*this);
}

template<typename C, typename O>
template<typename Lambda>
auto zpt::programming::bridge<C, O>::add_lambda(Lambda _lambda, zpt::json _conf)
  -> zpt::programming::bridge<C, O>& {
    static_cast<C*>(this)->setup_lambda(_conf, _lambda);
    return (*this);
}

template<typename C, typename O>
auto zpt::programming::bridge<C, O>::init() -> zpt::programming::bridge<C, O>& {
    static_cast<C*>(this)->initialize();
    return (*this);
}

template<typename C, typename O>
auto zpt::programming::bridge<C, O>::locate(zpt::json _to_locate) -> object_type {
    return static_cast<C*>(this)->find(_to_locate);
}

template<typename C, typename O>
auto zpt::programming::bridge<C, O>::json_to_object(zpt::json _to_convert) -> object_type {
    return static_cast<C*>(this)->to_object(_to_convert);
}

template<typename C, typename O>
auto zpt::programming::bridge<C, O>::object_to_json(object_type _to_convert) -> zpt::json {
    return static_cast<C*>(this)->to_json(_to_convert);
}

template<typename C, typename O>
template<typename Term, typename... Args>
auto zpt::programming::bridge<C, O>::call(Term _to_call, Args... _arg) -> zpt::json {
    O _ret = static_cast<C*>(this)->execute(_to_call, _arg...);
    auto _json_ret = static_cast<C*>(this)->to_json(_ret);
    return _json_ret;
}
