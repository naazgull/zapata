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
template<typename C, typename O>
class bridge {
  public:
    using class_type = C;
    using object_type = O;
    using lambda_type = std::function<object_type(object_type, class_type&)>;

    bridge() = default;
    bridge(zpt::bridge<C, O> const& _rhs) = delete;
    bridge(zpt::bridge<C, O>&& _rhs) = delete;
    virtual ~bridge() = default;

    auto operator=(zpt::bridge<C, O> const& _rhs) = delete;
    auto operator=(zpt::bridge<C, O>&& _rhs) = delete;

    auto set_config(zpt::json _conf) -> zpt::bridge<C, O>&;
    auto add_module(std::string _module_path, zpt::json _conf = zpt::undefined)
      -> zpt::bridge<C, O>&;
    auto add_module(lambda_type _lambda, zpt::json _conf = zpt::undefined) -> zpt::bridge<C, O>&;
    auto add_lambda(lambda_type _lambda, zpt::json _conf = zpt::undefined) -> zpt::bridge<C, O>&;
    auto eval(std::string _to_eval, O _arg) -> zpt::json;
};
}

template<typename C, typename O>
auto
zpt::bridge<C, O>::set_config(zpt::json _conf) -> zpt::bridge<C, O>& {
    static_cast<C*>(this)->setup_configuration(_conf);
    return (*this);
}

template<typename C, typename O>
auto
zpt::bridge<C, O>::add_module(std::string _module_path, zpt::json _conf) -> zpt::bridge<C, O>& {
    static_cast<C*>(this)->setup_module(_conf, _module_path);
    return (*this);
}

template<typename C, typename O>
auto
zpt::bridge<C, O>::add_module(lambda_type _lambda, zpt::json _conf) -> zpt::bridge<C, O>& {
    static_cast<C*>(this)->setup_module(_conf, _lambda);
    return (*this);
}

template<typename C, typename O>
auto
zpt::bridge<C, O>::add_lambda(lambda_type _lambda, zpt::json _conf) -> zpt::bridge<C, O>& {
    static_cast<C*>(this)->setup_lambda(_conf, _lambda);
    return (*this);
}

template<typename C, typename O>
auto
zpt::bridge<C, O>::eval(std::string _to_eval, O _arg) -> zpt::json {
    return static_cast<C*>(this)->to_json(static_cast<C*>(this)->evaluate(_to_eval, _arg));
}
