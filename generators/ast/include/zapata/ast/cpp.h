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
 * @file cpp.h
 * @brief C++ code generation AST classes.
 *
 * Provides concrete AST implementations for generating C++ source code.
 * Extends the base AST classes with C++-specific syntax formatting.
 *
 * @see zpt::ast::cpp_class
 * @see zpt::ast::cpp_function
 * @see zpt::ast::cpp_variable
 */

#pragma once

#include <zapata/ast/ast.h>

namespace zpt {
namespace ast {

/**
 * @brief C++ class definition AST node.
 *
 * Generates C++ class declarations with proper syntax including
 * inheritance, visibility sections, and member declarations.
 *
 * @par Example Output
 * @code
 * class MyClass : public BaseClass {
 *   public:
 *     void method();
 *   private:
 *     int member_;
 * };
 * @endcode
 */
class cpp_class : public zpt::ast::basic_class {
  public:
    /**
     * @brief Constructs a C++ class AST node.
     * @param _name Class name.
     * @param _extends Optional base class for inheritance.
     */
    cpp_class(std::string const& _name, std::string const& _extends = "");
    ~cpp_class() = default;

    /** @brief Generates C++ class declaration string. */
    auto to_string() const -> std::string override;
};

/**
 * @brief C++ code block AST node.
 *
 * Generates C++ code blocks with braces and proper indentation.
 * Used for function bodies, control flow statements, etc.
 */
class cpp_code_block : public zpt::ast::basic_code_block {
  public:
    /**
     * @brief Constructs a C++ code block.
     * @param _prefix Optional prefix before opening brace (e.g., "else").
     */
    cpp_code_block(std::string const& _prefix = "");
    ~cpp_code_block() = default;

    /** @brief Generates C++ code block string. */
    auto to_string() const -> std::string override;
};

/**
 * @brief C++ function AST node.
 *
 * Generates C++ function declarations/definitions with modifiers,
 * parameters, and body.
 *
 * @par Example Output
 * @code
 * virtual auto my_func(int x) const -> std::string override {
 *     return std::to_string(x);
 * }
 * @endcode
 */
class cpp_function : public zpt::ast::basic_function {
  public:
    /**
     * @brief Constructs a C++ function AST node.
     * @param _name Function name.
     * @param _return_type Return type (trailing return syntax).
     * @param _modifiers Bitwise OR of VIRTUAL, CONST, OVERRIDE, etc.
     */
    cpp_function(std::string const& _name,
                 std::string const& _return_type = "",
                 int _modifiers = 0);
    ~cpp_function() = default;

    /** @brief Generates C++ function declaration/definition string. */
    auto to_string() const -> std::string override;
};

/**
 * @brief C++ variable AST node.
 *
 * Generates C++ variable declarations with type and optional initialization.
 */
class cpp_variable : public zpt::ast::basic_variable {
  public:
    /**
     * @brief Constructs a C++ variable AST node.
     * @param _name Variable name.
     * @param _type Variable type.
     * @param _modifiers Bitwise OR of CONST, EXTERN, etc.
     */
    cpp_variable(std::string const& _name, std::string const& _type, int _modifiers = 0);
    ~cpp_variable() = default;

    /** @brief Generates C++ variable declaration string. */
    auto to_string() const -> std::string override;
};

/**
 * @brief C++ raw instruction AST node.
 *
 * Generates arbitrary C++ code statements with optional body block.
 */
class cpp_instruction : public zpt::ast::basic_instruction {
  public:
    /**
     * @brief Constructs a C++ instruction AST node.
     * @param _code Raw C++ code.
     * @param _no_end_of_line If true, omits trailing newline.
     */
    cpp_instruction(std::string const& _code, bool _no_end_of_line = false);
    ~cpp_instruction() = default;

    /** @brief Generates C++ instruction string. */
    auto to_string() const -> std::string override;

  private:
    bool __no_endl_of_line;
};

/**
 * @brief CMake instruction AST node.
 *
 * Generates CMake commands for CMakeLists.txt generation.
 */
class cmake_instruction : public zpt::ast::basic_instruction {
  public:
    /**
     * @brief Constructs a CMake instruction AST node.
     * @param _code CMake command.
     */
    cmake_instruction(std::string const& _code);
    ~cmake_instruction() = default;

    /** @brief Generates CMake command string. */
    auto to_string() const -> std::string override;
};
} // namespace ast
} // namespace zpt
