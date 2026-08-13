#include <sstream>
#include <zapata/ast/cpp.h>

/**
 * @file cpp.cpp
 * @brief C++ code generation AST class implementations.
 *
 * Provides concrete serialization logic for C++-specific AST node types,
 * generating valid C++ syntax including class declarations, function
 * definitions, variable declarations, and raw code instructions.
 */

/** @brief Constructs a C++ class AST node.
 * @param _name Class name.
 * @param _extends Optional base class for inheritance. */
zpt::ast::cpp_class::cpp_class(std::string const& _name, std::string const& _extends)
  : zpt::ast::basic_class{ _name, _extends } {}

/** @brief Generates the C++ class declaration string.
 * @return C++ class declaration as a string with inheritance, visibility sections, and members. */
auto zpt::ast::cpp_class::to_string() const -> std::string {
    std::ostringstream _oss;
    std::string _modifier_indent(AST_INDENTATION_SPACES / 2, ' ');

    _oss << this->get_indentation() << "class " << this->__name
         << (this->__extends.length() != 0 ? std::string{ " : " } + this->__extends : "") << " {"
         << std::endl
         << this->get_indentation() << _modifier_indent << "public:" << std::endl;
    for (auto& _element : this->__public) {
        std::visit([&_oss](auto&& arg) -> void { _oss << *arg << std::endl; }, _element);
    }
    _oss << this->get_indentation() << _modifier_indent << "protected:" << std::endl;
    for (auto& _element : this->__protected) {
        std::visit([&_oss](auto&& arg) -> void { _oss << *arg << std::endl; }, _element);
    }
    _oss << this->get_indentation() << _modifier_indent << "private:" << std::endl;
    for (auto& _element : this->__private) {
        std::visit([&_oss](auto&& arg) -> void { _oss << *arg << std::endl; }, _element);
    }
    _oss << this->get_indentation() << "};" << std::flush;
    return _oss.str();
}

/** @brief Constructs a C++ code block.
 * @param _prefix Optional prefix before opening brace (e.g., "else"). */
zpt::ast::cpp_code_block::cpp_code_block(std::string const& _prefix)
  : zpt::ast::basic_code_block{ _prefix } {}

/** @brief Generates the C++ code block string.
 * @return C++ code block as a string with braces, prefix, and nested elements. */
auto zpt::ast::cpp_code_block::to_string() const -> std::string {
    std::ostringstream _oss;
    _oss << (this->new_line() ? this->get_indentation() : "") << this->__prefix;
    if (this->__elements.size() != 0) {
        _oss << (this->__prefix.length() != 0 ? " " : "") << "{" << std::endl;
        for (auto& _element : this->__elements) {
            std::visit([&_oss](auto&& arg) -> void { _oss << *arg << std::endl; }, _element);
        }
        _oss << this->get_indentation() << "}";
    }
    else { _oss << ";"; }
    _oss << std::flush;
    return _oss.str();
}

/** @brief Constructs a C++ function AST node.
 * @param _name Function name.
 * @param _return_type Return type (trailing return syntax).
 * @param _modifiers Bitwise OR of VIRTUAL, CONST, OVERRIDE, etc. */
zpt::ast::cpp_function::cpp_function(std::string const& _name,
                                     std::string const& _return_type,
                                     int _modifiers)
  : zpt::ast::basic_function{ _name, _return_type, _modifiers } {}

/** @brief Generates the C++ function declaration/definition string.
 * @return C++ function declaration or definition with modifiers, parameters, return type, and body.
 */
auto zpt::ast::cpp_function::to_string() const -> std::string {
    std::ostringstream _oss;
    _oss << this->get_indentation();
    if ((this->__modifiers & zpt::ast::VIRTUAL) == zpt::ast::VIRTUAL) { _oss << "virtual "; }
    if ((this->__modifiers & zpt::ast::FRIEND) == zpt::ast::FRIEND) { _oss << "friend "; }
    if ((this->__modifiers & zpt::ast::EXTERN) == zpt::ast::EXTERN) { _oss << "extern "; }
    if ((this->__modifiers & zpt::ast::EXTERNC) == zpt::ast::EXTERNC) { _oss << "extern \"C\" "; }
    if (this->__return_type.length() != 0) { _oss << "auto "; }
    _oss << this->__name << "(";
    bool _first{ true };
    for (auto& _param : this->__parameters) {
        if (!_first) { _oss << ", "; }
        _first = false;
        _param->set_modifiers(zpt::ast::PARAMETER);
        _oss << *_param;
    }
    _oss << ")";
    if ((this->__modifiers & zpt::ast::CONST) == zpt::ast::CONST) { _oss << " const"; }
    if (this->__return_type.length() != 0) { _oss << " -> " << this->__return_type; }
    if ((this->__modifiers & zpt::ast::OVERRIDE) == zpt::ast::OVERRIDE) { _oss << " override"; }
    if ((this->__modifiers & zpt::ast::FINAL) == zpt::ast::FINAL) { _oss << " final"; }
    if ((this->__modifiers & zpt::ast::DEFAULT) == zpt::ast::DEFAULT) { _oss << " = default"; }
    if ((this->__modifiers & zpt::ast::DELETE) == zpt::ast::DELETE) { _oss << " = delete"; }
    if ((this->__modifiers & zpt::ast::ABSTRACT) == zpt::ast::ABSTRACT) { _oss << " = 0"; }
    if (this->__body != nullptr) { _oss << " " << *this->__body << std::endl; }
    else { _oss << ";"; }
    _oss << std::flush;
    return _oss.str();
}

/** @brief Constructs a C++ variable AST node.
 * @param _name Variable name.
 * @param _type Variable type.
 * @param _modifiers Bitwise OR of CONST, EXTERN, etc. */
zpt::ast::cpp_variable::cpp_variable(std::string const& _name,
                                     std::string const& _type,
                                     int _modifiers)
  : zpt::ast::basic_variable{ _name, _type, _modifiers } {}

/** @brief Generates the C++ variable declaration string.
 * @return C++ variable declaration with type, name, optional initialization, and semicolon. */
auto zpt::ast::cpp_variable::to_string() const -> std::string {
    std::ostringstream _oss;
    _oss << this->get_indentation();
    if ((this->__modifiers & zpt::ast::CONST) == zpt::ast::CONST) { _oss << "const "; }
    _oss << this->__type << " " << this->__name;
    if (this->__initialization != nullptr) { _oss << " = " << *this->__initialization; }
    if ((this->__modifiers & zpt::ast::PARAMETER) == 0) { _oss << ";"; }
    _oss << std::flush;
    return _oss.str();
}

/** @brief Constructs a C++ instruction AST node.
 * @param _code Raw C++ code string.
 * @param _no_end_of_line If true, omits trailing semicolon. */
zpt::ast::cpp_instruction::cpp_instruction(std::string const& _code, bool _no_end_of_line)
  : zpt::ast::basic_instruction{ _code }
  , __no_endl_of_line{ _no_end_of_line } {}

/** @brief Generates the C++ instruction string.
 * @return C++ code statement with indentation, optional body block, and conditional semicolon. */
auto zpt::ast::cpp_instruction::to_string() const -> std::string {
    std::ostringstream _oss;
    _oss << this->get_indentation() << this->__instruction;
    if (this->__body != nullptr) { _oss << *this->__body; }
    else if (!this->__no_endl_of_line && this->__instruction.find("#") != 0 &&
             this->__instruction.length() != 0) {
        _oss << ";";
    }
    _oss << std::flush;
    return _oss.str();
}

/** @brief Constructs a CMake instruction AST node.
 * @param _code CMake command string. */
zpt::ast::cmake_instruction::cmake_instruction(std::string const& _code)
  : zpt::ast::basic_instruction{ _code } {}

/** @brief Generates the CMake command string.
 * @return CMake command with optional body block. */
auto zpt::ast::cmake_instruction::to_string() const -> std::string {
    std::ostringstream _oss;
    _oss << this->__instruction;
    if (this->__body != nullptr) { _oss << *this->__body; }
    _oss << std::flush;
    return _oss.str();
}
