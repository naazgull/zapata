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

#include <iostream>
#include <string>
#include <zapata/functional.h>

namespace {
auto check(bool _cond, std::string const& _what) -> void {
    if (!_cond) {
        std::cerr << "FAILED: " << _what << std::endl;
        std::exit(1);
    }
    std::cout << "ok: " << _what << std::endl;
}
} // namespace

int main() {
    // Bare tokens (no call syntax)
    {
        auto _j = zpt::functional::parse("xpto");
        check(static_cast<std::string>(_j["functor"]) == "xpto", "bare variable token");
    }

    // Single-level calls with mixed argument types
    {
        auto _j = zpt::functional::parse("gt(integer(0.1, \"zpto\"))");
        check(static_cast<std::string>(_j["functor"]) == "gt", "gt(...) functor parses");
        check(_j["params"]->ok() && _j["params"]->size() == 1, "gt(...) has one param");
        auto _inner = _j["params"][0];
        check(static_cast<std::string>(_inner["functor"]) == "integer",
              "nested integer(...) functor parses");
        check(_inner["params"]->size() == 2, "integer(...) has two params");
        check(static_cast<long long>(_inner["params"][0]) == 0, "integer truncates 0.1 to 0");
        check(static_cast<std::string>(_inner["params"][1]) == "zpto",
              "quoted string param parses");
    }
    {
        auto _j = zpt::functional::parse("gt(double(0.1, xpto))");
        check(static_cast<std::string>(_j["functor"]) == "gt", "gt(...) functor parses (double)");
        auto _inner = _j["params"][0];
        check(static_cast<std::string>(_inner["functor"]) == "double",
              "nested double(...) functor parses");
        check(static_cast<double>(_inner["params"][0]) == 0.1, "double param parses");
        check(static_cast<std::string>(_inner["params"][1]) == "xpto",
              "bare variable param collapses to its name");
    }

    // Multiple params at the same level
    {
        auto _j = zpt::functional::parse("ge(integer(0,2),float(1,2,3))");
        check(static_cast<std::string>(_j["functor"]) == "ge", "ge(...) functor parses");
        check(_j["params"]->size() == 2, "ge(...) has two params");
        check(static_cast<std::string>(_j["params"][0]["functor"]) == "integer",
              "first param functor parses");
        check(_j["params"][0]["params"]->size() == 2, "integer(...) has two params");
        check(static_cast<std::string>(_j["params"][1]["functor"]) == "float",
              "second param functor parses");
        check(_j["params"][1]["params"]->size() == 3, "float(...) has three params");
    }

    // Deeply nested calls
    {
        auto _j = zpt::functional::parse("lower(upper(trim(\"  Hi  \")))");
        check(static_cast<std::string>(_j["functor"]) == "lower", "outer functor parses");
        check(static_cast<std::string>(_j["params"][0]["functor"]) == "upper",
              "middle functor parses");
        check(static_cast<std::string>(_j["params"][0]["params"][0]["functor"]) == "trim",
              "inner functor parses");
        check(static_cast<std::string>(_j["params"][0]["params"][0]["params"][0]) == "  Hi  ",
              "innermost quoted string with whitespace parses verbatim");
    }

    std::cout << "all tests passed" << std::endl;
    return 0;
}
