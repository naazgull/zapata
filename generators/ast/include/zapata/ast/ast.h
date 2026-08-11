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
 * @file ast.h
 * @brief Base AST classes for code generation.
 *
 * Provides abstract syntax tree building blocks for generating
 * source code. Used by the REST code generator and can be extended
 * for custom code generation needs.
 *
 * @par AST Hierarchy
 * - basic_module - Contains files
 * - basic_file - Contains classes, functions, variables
 * - basic_class - Contains members and methods
 * - basic_function - Function with parameters and body
 * - basic_variable - Variable declaration
 * - basic_instruction - Raw code instruction
 * - basic_code_block - Block of statements
 */

#pragma once

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

/** @brief Number of spaces per indentation level. */
inline std::uint16_t AST_INDENTATION_SPACES{ 4 };

namespace zpt {
namespace ast {

/** @name Visibility Constants */
///@{
static constexpr int PUBLIC{ 0 };
static constexpr int PROTECTED{ 1 };
static constexpr int PRIVATE{ 2 };
///@}

/** @name Function/Variable Modifiers */
///@{
static constexpr int VIRTUAL{ 1 };
static constexpr int FRIEND{ 2 };
static constexpr int CONST{ 4 };
static constexpr int OVERRIDE{ 8 };
static constexpr int FINAL{ 16 };
static constexpr int DEFAULT{ 32 };
static constexpr int DELETE{ 64 };
static constexpr int ABSTRACT{ 128 };
static constexpr int PARAMETER{ 256 };
static constexpr int EXTERN{ 512 };
static constexpr int EXTERNC{ 1024 };
///@}

class basic_module;
class basic_file;
class basic_class;
class basic_code_block;
class basic_function;
class basic_variable;
class basic_instruction;

/**
 * @brief Base class for all AST nodes.
 *
 * Provides indentation tracking and serialization interface.
 * All AST elements (classes, functions, variables, etc.) derive from this.
 */
class basic_element : public std::enable_shared_from_this<basic_element> {
  public:
    std::shared_ptr<basic_element> __parent{ nullptr }; ///< Parent element for indentation.

    /** @brief Default-constructs a basic_element. */
    basic_element() = default;
    virtual ~basic_element() = default;

    /** @brief Serializes this element to a string.
     * @return Serialized string representation. */
    virtual auto to_string() const -> std::string = 0;
    /** @brief Returns the indentation string for this nesting level.
     * @return Indentation string. */
    virtual auto get_indentation() const -> std::string final;
    /** @brief Returns whether to emit a newline before this element.
     * @return True if newline should be emitted. */
    virtual auto new_line() const -> bool final;
    /** @brief Sets whether to emit a newline before this element.
     *  @param _value True to emit a newline, false to suppress it.
     *  @return Reference to this element for chaining. */
    virtual auto set_new_line(bool _value) -> basic_element& final;
    /** @brief Stream insertion operator that calls to_string().
     *  @param _out Output stream.
     *  @param _in Element to output.
     *  @return Reference to the output stream. */
    friend auto operator<<(std::ostream& _out, basic_element& _in) -> std::ostream& {
        _out << _in.to_string();
        return _out;
    }

  private:
    /** @brief Current indentation level in spaces. */
    mutable std::uint16_t __indentation{ 0 };
    /** @brief Whether to emit a newline before this element. */
    bool __newline{ true };
};

/** @brief Constrains types to those derived from basic_element. */
template<typename T>
concept BasicASTElement = requires(T _t) { requires std::derived_from<T, basic_element>; };

/**
 * @brief Top-level AST node representing a source module (set of files).
 *
 * Groups one or more basic_file nodes under a named module. Provides
 * serialization (dump) and traversal over contained files.
 */
class basic_module {
  public:
    /** @brief Shared pointer to a basic_module. */
    using ptr = std::shared_ptr<basic_module>;
    /** @brief Allowed type for elements in a module (shared_ptr<basic_file>). */
    using allowed_type = std::shared_ptr<basic_file>;

    /** @brief Constructs a module node with the given name.
     *  @param _module_name Module name (used for directory and namespace). */
    basic_module(std::string const& _module_name);
    ~basic_module() = default;

    /** @brief Returns the module name.
     * @return Module name. */
    auto name() const -> std::string const&;
    /** @brief Adds a file to this module.
     * @param _to_add File to add.
     * @return Reference to this module. */
    auto add(std::shared_ptr<basic_file> _to_add) -> basic_module&;
    /** @brief Constructs and adds a file from arguments.
     * @param _args Arguments forwarded to basic_file constructor.
     * @return Reference to this module. */
    template<typename... Args>
    auto add(Args... _args) -> basic_module&;
    /** @brief Serializes all files to their respective paths.
     * @return Reference to this module. */
    auto dump() -> basic_module&;
    /** @brief Serializes all files to the given stream.
     * @param _out Output stream.
     * @return Reference to this module. */
    auto dump(std::ostream& _out) -> basic_module&;
    /** @brief Invokes a callback for each contained file.
     * @param _callback Callback function receiving each file. */
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_module&;

  protected:
    /** @brief The module name. */
    std::string __module_name;
    /** @brief Files contained in this module. */
    std::vector<allowed_type> __files;
};

/**
 * @brief AST node representing a single source file.
 *
 * Contains top-level elements such as classes, functions, variables,
 * code blocks, and raw instructions. Serializable to an output stream.
 */
class basic_file {
  public:
    /** @brief Shared pointer to a basic_file. */
    using ptr = std::shared_ptr<basic_file>;
    /** @brief Variant of allowed element types within a file. */
    using allowed_type = std::variant< //
      std::shared_ptr<basic_class>,
      std::shared_ptr<basic_function>,
      std::shared_ptr<basic_code_block>,
      std::shared_ptr<basic_variable>,
      std::shared_ptr<basic_instruction>>;

    /** @brief Constructs a file node at the given path.
     *  @param _path Output file path. */
    basic_file(std::filesystem::path const& _path);
    ~basic_file() = default;

    /** @brief Returns the output file path.
     * @return Output file path. */
    auto path() const -> std::filesystem::path const&;
    /** @brief Adds an AST element to this file.
     * @param _to_add AST element to add.
     * @return Reference to this file. */
    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_file&;
    /** @brief Constructs and adds an AST element from arguments.
     * @param _args Arguments forwarded to AST element constructor.
     * @return Reference to this file. */
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_file&;
    /** @brief Serializes all elements to the file path.
     * @return Reference to this file. */
    auto dump() -> basic_file&;
    /** @brief Serializes all elements to the given stream.
     * @param _out Output stream.
     * @return Reference to this file. */
    auto dump(std::ostream& _out) -> basic_file&;
    /** @brief Invokes a callback for each contained element.
     * @param _callback Callback function receiving each element. */
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_file&;

  protected:
    /** @brief The output file path. */
    std::filesystem::path __filename;
    /** @brief AST elements contained in this file. */
    std::vector<allowed_type> __elements;
};

/**
 * @brief AST node representing a class definition.
 *
 * Holds public, protected, and private members (nested classes,
 * functions, variables). Supports optional base class inheritance.
 */
class basic_class : public basic_element {
  public:
    /** @brief Shared pointer to a basic_class. */
    using ptr = std::shared_ptr<basic_class>;
    /** @brief Variant of allowed member types within a class. */
    using allowed_type = std::variant< //
      std::shared_ptr<basic_class>,
      std::shared_ptr<basic_function>,
      std::shared_ptr<basic_variable>,
      std::shared_ptr<basic_instruction>>;

    /** @brief Constructs a basic_class node.
     *  @param _name Class name.
     *  @param _extends Optional base class name for inheritance. */
    basic_class(std::string const& _name, std::string const& _extends = "");
    virtual ~basic_class() override = default;

    /** @brief Adds a member element with the given visibility (PUBLIC/PROTECTED/PRIVATE).
     * @param _to_add Member element to add.
     * @param _visibility Visibility level (PUBLIC, PROTECTED, or PRIVATE).
     * @return Reference to this class. */
    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add, int _visibility) -> basic_class&;
    /** @brief Constructs and adds a member element with the given visibility.
     * @param _visibility Visibility level (PUBLIC, PROTECTED, or PRIVATE).
     * @param _args Arguments forwarded to member element constructor.
     * @return Reference to this class. */
    template<BasicASTElement T, typename... Args>
    auto add(int _visibility, Args... _args) -> basic_class&;
    /** @brief Invokes a callback for each member across all visibility sections.
     * @param _callback Callback function receiving each member element. */
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_class&;

  protected:
    /** @brief The class name. */
    std::string __name;
    /** @brief The base class name for inheritance (empty if none). */
    std::string __extends;
    /** @brief Members declared in the public section. */
    std::vector<allowed_type> __public;
    /** @brief Members declared in the protected section. */
    std::vector<allowed_type> __protected;
    /** @brief Members declared in the private section. */
    std::vector<allowed_type> __private;
};

/**
 * @brief AST node representing a braced code block.
 *
 * Contains a sequence of statements (classes, nested blocks, variables,
 * instructions). An optional prefix string appears before the opening brace.
 */
class basic_code_block : public basic_element {
  public:
    /** @brief Shared pointer to a basic_code_block. */
    using ptr = std::shared_ptr<basic_code_block>;
    /** @brief Variant of allowed statement types within a code block. */
    using allowed_type = std::variant< //
      std::shared_ptr<basic_class>,
      std::shared_ptr<basic_code_block>,
      std::shared_ptr<basic_variable>,
      std::shared_ptr<basic_instruction>>;

    /** @brief Constructs a code block with an optional prefix (e.g., "else").
     *  @param _prefix Text emitted before the opening brace. */
    basic_code_block(std::string const& _prefix = "");
    virtual ~basic_code_block() override = default;

    /** @brief Adds a statement element to this block.
     * @param _to_add Statement element to add.
     * @return Reference to this code block. */
    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_code_block&;
    /** @brief Constructs and adds a statement element from arguments.
     * @param _args Arguments forwarded to statement element constructor.
     * @return Reference to this code block. */
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_code_block&;
    /** @brief Invokes a callback for each statement in the block.
     * @param _callback Callback function receiving each statement element. */
    template<typename Callback>
    auto traverse_elements(Callback _callback) -> basic_code_block&;

  protected:
    /** @brief Prefix text emitted before the opening brace. */
    std::string __prefix;
    /** @brief Statements contained within the code block. */
    std::vector<allowed_type> __elements;
};

/**
 * @brief AST node representing a function or method declaration.
 *
 * Holds the function name, return type, modifier flags (VIRTUAL, CONST, etc.),
 * parameter list, and an optional body code block.
 */
class basic_function : public basic_element {
  public:
    /** @brief Shared pointer to a basic_function. */
    using ptr = std::shared_ptr<basic_function>;

    /** @brief Constructs a function AST node.
     *  @param _name Function name.
     *  @param _return_type Return type string (for trailing return syntax).
     *  @param _modifiers Bitwise OR of VIRTUAL, CONST, OVERRIDE, FINAL, DEFAULT, DELETE, ABSTRACT.
     */
    basic_function(std::string const& _name,
                   std::string const& _return_type = "",
                   int _modifiers = 0);
    virtual ~basic_function() override = default;

    /** @brief Adds a parameter (basic_variable) or body (basic_code_block).
     * @param _to_add Parameter or body element to add.
     * @return Reference to this function. */
    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_function&;
    /** @brief Constructs and adds a parameter or body from arguments.
     * @param _args Arguments forwarded to element constructor.
     * @return Reference to this function. */
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_function&;
    /** @brief Sets modifier flags (VIRTUAL, CONST, OVERRIDE, etc.).
     * @param _modifiers Bitwise OR of modifier constants.
     * @return Reference to this function. */
    auto set_modifiers(int _modifiers) -> basic_function&;
    /** @brief Returns the function body code block (may be null).
     * @return Shared pointer to body code block. */
    auto body() -> std::shared_ptr<basic_code_block>;

  protected:
    /** @brief The function name. */
    std::string __name;
    /** @brief Function parameters. */
    std::vector<std::shared_ptr<basic_variable>> __parameters;
    /** @brief Return type string (used with trailing return syntax). */
    std::string __return_type;
    /** @brief Modifier flags (VIRTUAL, CONST, OVERRIDE, etc.). */
    int __modifiers{ 0 };
    /** @brief Function body code block (null for declarations without body). */
    std::shared_ptr<basic_code_block> __body{ nullptr };
};

/**
 * @brief AST node representing a variable or parameter declaration.
 *
 * Holds the variable name, type string, modifier flags, and an optional
 * initialization code block.
 */
class basic_variable : public basic_element {
  public:
    /** @brief Shared pointer to a basic_variable. */
    using ptr = std::shared_ptr<basic_variable>;

    /** @brief Constructs a variable AST node.
     *  @param _name Variable name.
     *  @param _type Variable type.
     *  @param _modifiers Bitwise OR of CONST, EXTERN, PARAMETER, etc. */
    basic_variable(std::string const& _name, std::string const& _type, int _modifiers = 0);
    virtual ~basic_variable() override = default;

    /** @brief Sets the initialization code block.
     * @param _initialization Code block for variable initialization.
     * @return Reference to this variable. */
    auto add(std::shared_ptr<basic_code_block> _initialization) -> basic_variable&;
    /** @brief Constructs and sets the initialization code block from arguments.
     * @param _args Arguments forwarded to code block constructor.
     * @return Reference to this variable. */
    template<typename... Args>
    auto add(Args... _args) -> basic_variable&;
    /** @brief Sets modifier flags (CONST, EXTERN, PARAMETER, etc.).
     * @param _modifiers Bitwise OR of modifier constants.
     * @return Reference to this variable. */
    auto set_modifiers(int _modifiers) -> basic_variable&;
    /** @brief Returns the initialization code block (may be null).
     * @return Shared pointer to initialization code block. */
    auto initialization() -> std::shared_ptr<basic_code_block>;

  protected:
    /** @brief The variable name. */
    std::string __name;
    /** @brief The variable type. */
    std::string __type;
    /** @brief Modifier flags (CONST, EXTERN, PARAMETER, etc.). */
    int __modifiers{ 0 };
    /** @brief Initialization code block (null if no initialization). */
    std::shared_ptr<basic_code_block> __initialization{ nullptr };
};

/**
 * @brief AST node representing a raw code instruction or statement.
 *
 * Holds a literal code string and an optional body code block
 * (for constructs like if/for that have a trailing block).
 */
class basic_instruction : public basic_element {
  public:
    /** @brief Shared pointer to a basic_instruction. */
    using ptr = std::shared_ptr<basic_instruction>;

    /** @brief Constructs a raw instruction AST node.
     *  @param _code Literal code string. */
    basic_instruction(std::string const& _code);
    virtual ~basic_instruction() override = default;

    /** @brief Sets the body code block for this instruction.
     * @param _body Code block for the instruction body.
     * @return Reference to this instruction. */
    auto add(std::shared_ptr<basic_code_block> _body) -> basic_instruction&;
    /** @brief Constructs and sets the body code block from arguments.
     * @param _args Arguments forwarded to code block constructor.
     * @return Reference to this instruction. */
    template<typename... Args>
    auto add(Args... _args) -> basic_instruction&;
    /** @brief Returns the body code block (may be null).
     * @return Shared pointer to body code block. */
    auto body() -> std::shared_ptr<basic_code_block>;

  protected:
    /** @brief The literal code string. */
    std::string __instruction;
    /** @brief Body code block (null if no body). */
    std::shared_ptr<basic_code_block> __body{ nullptr };
};
} // namespace ast

/** @name AST Factory Functions
 *  Convenience functions for constructing AST nodes as shared pointers.
 */
///@{
template<typename T, typename... Args>
auto make_module(Args... _args) -> std::shared_ptr<ast::basic_module>;
template<typename T, typename... Args>
auto make_file(Args... _args) -> std::shared_ptr<ast::basic_file>;
template<zpt::ast::BasicASTElement T, typename... Args>
auto make_class(Args... _args) -> std::shared_ptr<ast::basic_class>;
template<zpt::ast::BasicASTElement T, typename... Args>
auto make_code_block(Args... _args) -> std::shared_ptr<ast::basic_code_block>;
template<zpt::ast::BasicASTElement T, typename... Args>
auto make_function(Args... _args) -> std::shared_ptr<ast::basic_function>;
template<zpt::ast::BasicASTElement T, typename... Args>
auto make_variable(Args... _args) -> std::shared_ptr<ast::basic_variable>;
template<zpt::ast::BasicASTElement T, typename... Args>
auto make_instruction(Args... _args) -> std::shared_ptr<ast::basic_instruction>;
///@}
} // namespace zpt

template<typename Callback>
auto zpt::ast::basic_module::traverse_elements(Callback _callback) -> basic_module& {
    for (auto& _file : this->__files) { _callback(_file); }
    return (*this);
}

template<zpt::ast::BasicASTElement T>
auto zpt::ast::basic_file::add(std::shared_ptr<T> _to_add) -> basic_file& {
    this->__elements.push_back(_to_add);
    return (*this);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::ast::basic_file::add(Args... _args) -> basic_file& {
    return this->add<T>(std::make_shared<T>(std::forward<Args>(_args)...));
}

template<typename Callback>
auto zpt::ast::basic_file::traverse_elements(Callback _callback) -> basic_file& {
    for (auto& _element : this->__elements) {
        std::visit([&_callback](auto&& arg) -> void { _callback(arg); }, _element);
    }
    return (*this);
}

template<zpt::ast::BasicASTElement T>
auto zpt::ast::basic_class::add(std::shared_ptr<T> _to_add, int _visibility) -> basic_class& {
    switch (_visibility) {
        case zpt::ast::PUBLIC: {
            this->__public.push_back(_to_add);
            break;
        }
        case zpt::ast::PROTECTED: {
            this->__protected.push_back(_to_add);
            break;
        }
        case zpt::ast::PRIVATE: {
            this->__private.push_back(_to_add);
            break;
        }
    }
    _to_add->__parent = this->shared_from_this();
    return (*this);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::ast::basic_class::add(int _visibility, Args... _args) -> basic_class& {
    return this->add<T>(std::make_shared<T>(std::forward<Args>(_args)...), _visibility);
}

template<typename Callback>
auto zpt::ast::basic_class::traverse_elements(Callback _callback) -> basic_class& {
    for (auto& _element : this->__public) {
        std::visit([&_callback](auto&& arg) -> void { _callback(arg); }, _element);
    }
    for (auto& _element : this->__protected) {
        std::visit([&_callback](auto&& arg) -> void { _callback(arg); }, _element);
    }
    for (auto& _element : this->__private) {
        std::visit([&_callback](auto&& arg) -> void { _callback(arg); }, _element);
    }
    return (*this);
}

template<zpt::ast::BasicASTElement T>
auto zpt::ast::basic_code_block::add(std::shared_ptr<T> _to_add) -> basic_code_block& {
    this->__elements.push_back(_to_add);
    _to_add->__parent = this->shared_from_this();
    return (*this);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::ast::basic_code_block::add(Args... _args) -> basic_code_block& {
    return this->add<T>(std::make_shared<T>(std::forward<Args>(_args)...));
}

template<typename Callback>
auto zpt::ast::basic_code_block::traverse_elements(Callback _callback) -> basic_code_block& {
    for (auto& _element : this->__elements) {
        std::visit([&_callback](auto&& arg) -> void { _callback(arg); }, _element);
    }
    return (*this);
}

template<zpt::ast::BasicASTElement T>
auto zpt::ast::basic_function::add(std::shared_ptr<T> _to_add) -> basic_function& {
    if constexpr (std::derived_from<T, basic_variable>) { this->__parameters.push_back(_to_add); }
    if constexpr (std::derived_from<T, basic_code_block>) {
        this->__body = _to_add;
        _to_add->__parent = this->shared_from_this();
        _to_add->set_new_line(false);
    }
    return (*this);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::ast::basic_function::add(Args... _args) -> basic_function& {
    return this->add<T>(std::make_shared<T>(std::forward<Args>(_args)...));
}

template<typename... Args>
auto zpt::ast::basic_variable::add(Args... _args) -> basic_variable& {
    this->__initialization = std::make_shared<basic_code_block>(std::forward<Args>(_args)...);
    this->__initialization->__parent = this->shared_from_this();
    this->__initialization->set_new_line(false);
    return (*this);
}

template<typename... Args>
auto zpt::ast::basic_instruction::add(Args... _args) -> basic_instruction& {
    this->__body = std::make_shared<basic_code_block>(std::forward<Args>(_args)...);
    this->__body->__parent = this->shared_from_this();
    this->__body->set_new_line(false);
    return (*this);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::make_module(Args... _args) -> std::shared_ptr<ast::basic_module> {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::make_file(Args... _args) -> std::shared_ptr<ast::basic_file> {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::make_class(Args... _args) -> std::shared_ptr<ast::basic_class> {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::make_code_block(Args... _args) -> std::shared_ptr<ast::basic_code_block> {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::make_function(Args... _args) -> std::shared_ptr<ast::basic_function> {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::make_variable(Args... _args) -> std::shared_ptr<ast::basic_variable> {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::make_instruction(Args... _args) -> std::shared_ptr<ast::basic_instruction> {
    return std::make_shared<T>(std::forward<Args>(_args)...);
}
