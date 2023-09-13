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

#include <zapata/ast/ast.h>

namespace zpt {
namespace ast {
class cpp_class : public zpt::ast::basic_class {
  public:
    cpp_class(std::string const& _name, std::string const& _extends = "");
    ~cpp_class() = default;

    auto to_string() const -> std::string override;
};

class cpp_code_block : public zpt::ast::basic_code_block {
  public:
    cpp_code_block(std::string const& _prefix = "");
    ~cpp_code_block() = default;

    auto to_string() const -> std::string override;
};

class cpp_function : public zpt::ast::basic_function {
  public:
    cpp_function(std::string const& _name,
                 std::string const& _return_type = "",
                 int _modifiers = 0);
    ~cpp_function() = default;

    auto to_string() const -> std::string override;
};

class cpp_variable : public zpt::ast::basic_variable {
  public:
    cpp_variable(std::string const& _name, std::string const& _type, int _modifiers = 0);
    ~cpp_variable() = default;

    auto to_string() const -> std::string override;
};

class cpp_instruction : public zpt::ast::basic_instruction {
  public:
    cpp_instruction(std::string const& _code, bool _no_end_of_line = false);
    ~cpp_instruction() = default;

    auto to_string() const -> std::string override;

  private:
    bool __no_endl_of_line;
};

class cmake_instruction : public zpt::ast::basic_instruction {
  public:
    cmake_instruction(std::string const& _code);
    ~cmake_instruction() = default;

    auto to_string() const -> std::string override;
};
} // namespace ast
} // namespace zpt
