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

#pragma once

#include <zapata/json.h>

namespace zpt {
namespace programming {

class integration {
  public:
    integration() = default;
    virtual ~integration() = default;

    virtual auto name() const -> std::string = 0;
};

template<typename C, typename O>
class bridge : public zpt::programming::integration {
  public:
    using class_type = C;
    using object_type = O;

    bridge() = default;
    bridge(bridge<C, O> const& _rhs) = delete;
    bridge(bridge<C, O>&& _rhs) = delete;
    virtual ~bridge() = default;

    auto operator=(bridge<C, O> const& _rhs) = delete;
    auto operator=(bridge<C, O>&& _rhs) = delete;

    auto set_options(zpt::json _conf) -> bridge<C, O>&;
    auto options() const -> zpt::json;

    auto add_module(std::string _external_path, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    template<typename Callback>
    auto add_module(Callback _callback, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    template<typename Lambda>
    auto add_lambda(Lambda _lambda, zpt::json _conf = zpt::undefined) -> bridge<C, O>&;
    auto init() -> bridge<C, O>&;

    auto locate(zpt::json _to_locate) -> object_type;

    auto json_to_object(zpt::json _to_convert) -> object_type;
    auto object_to_json(object_type _to_convert) -> zpt::json;

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
