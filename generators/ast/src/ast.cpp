#include <fstream>
#include <zapata/ast/ast.h>
#include <zapata/base.h>

/**
 * @file ast.cpp
 * @brief Implementation of base AST classes.
 *
 * Provides concrete implementations for all AST node types including
 * serialization, element addition, and traversal operations.
 */

/** @brief Computes and returns the indentation string for this node's nesting level.
 * @return Indentation string consisting of spaces equal to the current nesting depth. */
auto zpt::ast::basic_element::get_indentation() const -> std::string {
    if (this->__newline) {
        this->__indentation =
          (this->__parent != nullptr ? this->__parent->__indentation + AST_INDENTATION_SPACES : 0);
    }
    else { this->__indentation = (this->__parent != nullptr ? this->__parent->__indentation : 0); }
    return std::string(this->__indentation, ' ');
}

/** @brief Returns whether to emit a newline before this element.
 * @return True if a newline should be emitted before this element. */
auto zpt::ast::basic_element::new_line() const -> bool { return this->__newline; }

/** @brief Sets whether to emit a newline before this element.
 *  @param _value True to emit a newline, false to suppress it.
 *  @return Reference to this element for chaining. */
auto zpt::ast::basic_element::set_new_line(bool _value) -> basic_element& {
    this->__newline = _value;
    return (*this);
}

/** @brief Constructs a module node with the given name.
 * @param _name Module name used for directory and namespace. */
zpt::ast::basic_module::basic_module(std::string const& _name)
  : __module_name{ _name } {}

/** @brief Returns the module name.
 * @return Constant reference to the module name. */
auto zpt::ast::basic_module::name() const -> std::string const& { return this->__module_name; }

/** @brief Adds a file to this module.
 * @param _to_add File to add to the module.
 * @return Reference to this module for chaining. */
auto zpt::ast::basic_module::add(std::shared_ptr<basic_file> _to_add) -> basic_module& {
    this->__files.push_back(_to_add);
    return (*this);
}

/** @brief Serializes all files to their respective paths on disk.
 * @return Reference to this module for chaining. */
auto zpt::ast::basic_module::dump() -> basic_module& {
    for (auto& _file : this->__files) { _file->dump(); }
    return (*this);
}

/** @brief Serializes all files to the given output stream.
 * @param _out Output stream to serialize all file contents to.
 * @return Reference to this module for chaining. */
auto zpt::ast::basic_module::dump(std::ostream& _out) -> basic_module& {
    for (auto& _file : this->__files) { _file->dump(_out); }
    return (*this);
}

/** @brief Constructs a file node at the given output path.
 * @param _path Output file path where the file will be written. */
zpt::ast::basic_file::basic_file(std::filesystem::path const& _path)
  : __filename{ _path } {}

/** @brief Returns the output file path.
 * @return Constant reference to the file path. */
auto zpt::ast::basic_file::path() const -> std::filesystem::path const& { return this->__filename; }

/** @brief Serializes all file elements to the file path on disk.
 * @return Reference to this file for chaining. */
auto zpt::ast::basic_file::dump() -> basic_file& {
    std::ofstream _ofs;
    _ofs.open(this->__filename);
    expect(_ofs.is_open(), "Couldn't open file at '" << this->__filename << "'.");
    return this->dump(_ofs);
}

/** @brief Serializes all file elements to the given output stream.
 * @param _out Output stream to serialize file contents to.
 * @return Reference to this file for chaining. */
auto zpt::ast::basic_file::dump(std::ostream& _out) -> basic_file& {
    for (auto& _element : this->__elements) {
        std::visit([&_out](auto&& arg) -> void { _out << *arg << std::endl; }, _element);
    }
    return (*this);
}

/** @brief Constructs a class node with optional inheritance.
 * @param _name Class name.
 * @param _extends Base class name for inheritance (empty if none). */
zpt::ast::basic_class::basic_class(std::string const& _name, std::string const& _extends)
  : __name{ _name }
  , __extends{ _extends } {}

/** @brief Constructs a code block with an optional prefix.
 * @param _prefix Prefix text emitted before the opening brace (e.g., "else"). */
zpt::ast::basic_code_block::basic_code_block(std::string const& _prefix)
  : __prefix{ _prefix } {}

/** @brief Constructs a function node with return type and modifiers.
 * @param _name Function name.
 * @param _return_type Return type string for trailing return syntax.
 * @param _modifiers Bitwise OR of modifier flags (VIRTUAL, CONST, OVERRIDE, etc.). */
zpt::ast::basic_function::basic_function(std::string const& _name,
                                         std::string const& _return_type,
                                         int _modifiers)
  : __name{ _name }
  , __return_type{ _return_type }
  , __modifiers{ _modifiers } {}

/** @brief Adds modifier flags to the function.
 * @param _modifiers Bitwise OR of modifier flags to add (VIRTUAL, CONST, OVERRIDE, etc.).
 * @return Reference to this function for chaining. */
auto zpt::ast::basic_function::set_modifiers(int _modifiers) -> basic_function& {
    this->__modifiers |= _modifiers;
    return (*this);
}

/** @brief Returns the function body code block.
 * @return Shared pointer to the body code block (null if no body). */
auto zpt::ast::basic_function::body() -> std::shared_ptr<basic_code_block> { return this->__body; }

/** @brief Constructs a variable node with type and modifiers.
 * @param _name Variable name.
 * @param _type Variable type string.
 * @param _modifiers Bitwise OR of modifier flags (CONST, EXTERN, PARAMETER, etc.). */
zpt::ast::basic_variable::basic_variable(std::string const& _name,
                                         std::string const& _type,
                                         int _modifiers)
  : __name{ _name }
  , __type{ _type }
  , __modifiers{ _modifiers } {}

/** @brief Sets the initialization code block for this variable.
 * @param _initialization Code block containing the initialization expression.
 * @return Reference to this variable for chaining. */
auto zpt::ast::basic_variable::add(std::shared_ptr<basic_code_block> _initialization)
  -> basic_variable& {
    this->__initialization = _initialization;
    this->__initialization->__parent = this->shared_from_this();
    this->__initialization->set_new_line(false);
    return (*this);
}

/** @brief Adds modifier flags to the variable.
 * @param _modifiers Bitwise OR of modifier flags to add (CONST, EXTERN, PARAMETER, etc.).
 * @return Reference to this variable for chaining. */
auto zpt::ast::basic_variable::set_modifiers(int _modifiers) -> basic_variable& {
    this->__modifiers |= _modifiers;
    return (*this);
}

/** @brief Returns the initialization code block.
 * @return Shared pointer to the initialization code block (null if no initialization). */
auto zpt::ast::basic_variable::initialization() -> std::shared_ptr<basic_code_block> {
    return this->__initialization;
}

/** @brief Constructs a raw instruction node with literal code.
 * @param _code Literal code string for this instruction. */
zpt::ast::basic_instruction::basic_instruction(std::string const& _code)
  : __instruction{ _code } {}

/** @brief Sets the body code block for this instruction.
 * @param _body Code block representing the instruction body.
 * @return Reference to this instruction for chaining. */
auto zpt::ast::basic_instruction::add(std::shared_ptr<basic_code_block> _body)
  -> basic_instruction& {
    this->__body = _body;
    this->__body->__parent = this->shared_from_this();
    this->__body->set_new_line(false);
    return (*this);
}

/** @brief Returns the body code block.
 * @return Shared pointer to the body code block (null if no body). */
auto zpt::ast::basic_instruction::body() -> std::shared_ptr<basic_code_block> {
    return this->__body;
}
