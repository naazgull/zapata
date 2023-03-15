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

#include <zapata/bridge/bridge.h>
#include <zapata/base.h>

class cpp_bridge : public zpt::programming::bridge<cpp_bridge, zpt::json> {
  public:
    using lambda_type = std::function<object_type(object_type, class_type&)>;

    cpp_bridge() { this->__modules.insert(std::make_pair("::", zpt::undefined)); }

    auto name() const -> std::string { return "cpp_example"; }

    auto setup_module(zpt::json _conf, lambda_type _lambda) -> cpp_bridge& {
        auto [_it, _] = this->__modules.insert(
          std::make_pair(_conf("name")->string() + std::string{ "::" }, _conf));
        _it->second += _lambda(_conf, *this);
        return (*this);
    }

    auto setup_lambda(zpt::json _conf, lambda_type _lambda) -> cpp_bridge& {
        std::string _package =
          (_conf("module")->ok() ? _conf("module")->string() : std::string{ "" }) +
          std::string{ "::" };
        zlog(_package, zpt::info);
        expect(this->__modules.find(_package) != this->__modules.end(), "no such module");
        this->__lambdas.insert(std::make_pair(_package + _conf("name")->string(), _lambda));
        return (*this);
    }

    auto execute(std::string _to_evaluate, object_type _arg, class_type&) -> zpt::json {
        auto _found = this->__lambdas.find(_to_evaluate);
        expect(_found != this->__lambdas.end(), "unknown expression '" << _to_evaluate << "'");
        return _found->second(_arg, *this);
    }

    auto to_object(zpt::json _to_convert) const -> object_type { return _to_convert; }
    auto to_json(object_type _to_convert) const -> zpt::json { return _to_convert; }

  private:
    std::map<std::string, lambda_type> __lambdas;
    std::map<std::string, zpt::json> __modules;
};

auto init_module_x(cpp_bridge::object_type, cpp_bridge& _bridge) -> zpt::json {
    _bridge.add_lambda(
      [](cpp_bridge::object_type _a, cpp_bridge&) -> zpt::json {
          return { "a", _a };
      },
      { "module", "x", "name", "to_a" });
    return zpt::undefined;
}

auto main(int, char**) -> int {
    cpp_bridge _bridge;
    auto _result = _bridge                                       //
                     .set_options({ "flags", "SFGT" })           //
                     .add_module(init_module_x, { "name", "x" }) //
                     .call<std::string, zpt::json, cpp_bridge&>(
                       "x::to_a", zpt::json{ "field", "xpto" }, _bridge);
    zlog(_result, zpt::info);
    return 0;
}
