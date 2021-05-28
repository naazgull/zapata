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

class cpp_bridge
  : public zpt::bridge<cpp_bridge, zpt::json> {
  public:
    cpp_bridge() { this->__modules.insert(std::make_pair("::", zpt::undefined)); }

    auto setup_configuration(zpt::json _conf) -> cpp_bridge& {
        zlog(_conf, zpt::info);
        return (*this);
    }
    auto setup_module(zpt::json _conf, std::string _module_path) -> cpp_bridge& {
        this->__modules.insert(std::make_pair(_conf["name"]->string() + std::string{ "::" },
                                              _conf + zpt::json{ "load_path", _module_path }));
        return (*this);
    }

    auto setup_module(zpt::json _conf, lambda_type _lambda) -> cpp_bridge& {
        auto [_it, _] = this->__modules.insert(
          std::make_pair(_conf["name"]->string() + std::string{ "::" }, _conf));
        _it->second += _lambda(_conf, *this);
        return (*this);
    }

    auto setup_lambda(zpt::json _conf, lambda_type _lambda) -> cpp_bridge& {
        std::string _package =
          (_conf["module"]->ok() ? _conf["module"]->string() : std::string{ "" }) +
          std::string{ "::" };
        zlog(_package, zpt::info);
        expect(this->__modules.find(_package) != this->__modules.end(), "no such module", 500, 0);
        this->__lambdas.insert(std::make_pair(_package + _conf["name"]->string(), _lambda));
        return (*this);
    }

    auto evaluate(std::string _to_evaluate, zpt::json _conf) -> zpt::json {
        auto _found = this->__lambdas.find(_to_evaluate);
        expect(
          _found != this->__lambdas.end(), "unknown expression '" << _to_evaluate << "'", 500, 0);
        return _found->second(_conf, *this);
    }

    auto to_json(zpt::json _to_convert) -> zpt::json { return _to_convert; }

  private:
    std::map<std::string, lambda_type> __lambdas;
    std::map<std::string, zpt::json> __modules;
};

auto
init_module_x(zpt::json _conf, cpp_bridge& _bridge) -> zpt::json {
    _bridge.add_lambda(
      [](zpt::json _a, cpp_bridge&) -> zpt::json {
          return { "a", _a };
      },
      { "module", "x", "name", "to_a" });
    return zpt::undefined;
}

auto
main(int argc, char* argv[]) -> int {
    cpp_bridge _bridge;
    auto _result = _bridge                                       //
                     .set_config({ "flags", "SFGT" })            //
                     .add_module(init_module_x, { "name", "x" }) //
                     .eval("x::to_a", zpt::json{ "field", "xpto" });
    zlog(_result, zpt::info);
    return 0;
}
