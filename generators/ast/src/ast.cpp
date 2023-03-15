#include <zapata/ast/ast.h>
#include <zapata/base.h>
#include <fstream>

auto zpt::ast::basic_element::get_indentation() const -> std::string {
    if (this->__newline) {
        this->__indentation =
          (this->__parent != nullptr ? this->__parent->__indentation + AST_INDENTATION_SPACES : 0);
    }
    else { this->__indentation = (this->__parent != nullptr ? this->__parent->__indentation : 0); }
    return std::string(this->__indentation, ' ');
}

auto zpt::ast::basic_element::new_line() const -> bool { return this->__newline; }

auto zpt::ast::basic_element::set_new_line(bool _value) -> basic_element& {
    this->__newline = _value;
    return (*this);
}

zpt::ast::basic_module::basic_module(std::string const& _name)
  : __module_name{ _name } {}

auto zpt::ast::basic_module::name() const -> std::string const& { return this->__module_name; }

auto zpt::ast::basic_module::add(std::shared_ptr<basic_file> _to_add) -> basic_module& {
    this->__files.push_back(_to_add);
    return (*this);
}

auto zpt::ast::basic_module::dump() -> basic_module& {
    for (auto& _file : this->__files) { _file->dump(); }
    return (*this);
}

auto zpt::ast::basic_module::dump(std::ostream& _out) -> basic_module& {
    for (auto& _file : this->__files) { _file->dump(_out); }
    return (*this);
}

zpt::ast::basic_file::basic_file(std::filesystem::path const& _path)
  : __filename{ _path } {}

auto zpt::ast::basic_file::path() const -> std::filesystem::path const& { return this->__filename; }

auto zpt::ast::basic_file::dump() -> basic_file& {
    std::ofstream _ofs;
    _ofs.open(this->__filename);
    expect(_ofs.is_open(), "Couldn't open file at '" << this->__filename << "'.");
    return this->dump(_ofs);
}

auto zpt::ast::basic_file::dump(std::ostream& _out) -> basic_file& {
    for (auto& _element : this->__elements) {
        std::visit([&_out](auto&& arg) -> void { _out << *arg << std::endl; }, _element);
    }
    return (*this);
}

zpt::ast::basic_class::basic_class(std::string const& _name, std::string const& _extends)
  : __name{ _name }
  , __extends{ _extends } {}

zpt::ast::basic_code_block::basic_code_block(std::string const& _prefix)
  : __prefix{ _prefix } {}

zpt::ast::basic_function::basic_function(std::string const& _name,
                                         std::string const& _return_type,
                                         int _modifiers)
  : __name{ _name }
  , __return_type{ _return_type }
  , __modifiers{ _modifiers } {}

auto zpt::ast::basic_function::set_modifiers(int _modifiers) -> basic_function& {
    this->__modifiers |= _modifiers;
    return (*this);
}

zpt::ast::basic_variable::basic_variable(std::string const& _name,
                                         std::string const& _type,
                                         int _modifiers)
  : __name{ _name }
  , __type{ _type }
  , __modifiers{ _modifiers } {}

auto zpt::ast::basic_variable::add(std::shared_ptr<basic_code_block> _initialization)
  -> basic_variable& {
    this->__initialization = _initialization;
    this->__initialization->__parent = this->shared_from_this();
    this->__initialization->set_new_line(false);
    return (*this);
}

auto zpt::ast::basic_variable::set_modifiers(int _modifiers) -> basic_variable& {
    this->__modifiers |= _modifiers;
    return (*this);
}

zpt::ast::basic_instruction::basic_instruction(std::string const& _code)
  : __instruction{ _code } {}

auto zpt::ast::basic_instruction::add(std::shared_ptr<basic_code_block> _body)
  -> basic_instruction& {
    this->__body = _body;
    this->__body->__parent = this->shared_from_this();
    this->__body->set_new_line(false);
    return (*this);
}
