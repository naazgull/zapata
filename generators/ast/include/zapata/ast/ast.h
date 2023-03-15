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

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include <filesystem>

inline std::uint16_t AST_INDENTATION_SPACES{ 4 };

namespace zpt {
namespace ast {
static constexpr int PUBLIC{ 0 };
static constexpr int PROTECTED{ 1 };
static constexpr int PRIVATE{ 2 };

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

class basic_module;
class basic_file;
class basic_class;
class basic_code_block;
class basic_function;
class basic_variable;
class basic_instruction;

class basic_element : public std::enable_shared_from_this<basic_element> {
  public:
    std::shared_ptr<basic_element> __parent{ nullptr };

    basic_element() = default;
    virtual ~basic_element() = default;

    virtual auto to_string() const -> std::string = 0;
    virtual auto get_indentation() const -> std::string final;
    virtual auto new_line() const -> bool final;
    virtual auto set_new_line(bool _value) -> basic_element& final;
    friend auto operator<<(std::ostream& _out, basic_element& _in) -> std::ostream& {
        _out << _in.to_string();
        return _out;
    }

  private:
    mutable std::uint16_t __indentation{ 0 };
    bool __newline{ true };
};

template<typename T>
concept BasicASTElement = requires(T _t) {
    requires std::derived_from<T, basic_element>;
};

class basic_module {
  public:
    using allowed_type = std::shared_ptr<basic_file>;

    basic_module(std::string const& _module_name);
    ~basic_module() = default;

    auto name() const -> std::string const&;
    auto add(std::shared_ptr<basic_file> _to_add) -> basic_module&;
    template<typename... Args>
    auto add(Args... _args) -> basic_module&;
    auto dump() -> basic_module&;
    auto dump(std::ostream& _out) -> basic_module&;

  protected:
    std::string __module_name;
    std::vector<allowed_type> __files;
};

class basic_file {
  public:
    using allowed_type = std::variant< //
      std::shared_ptr<basic_class>,
      std::shared_ptr<basic_function>,
      std::shared_ptr<basic_code_block>,
      std::shared_ptr<basic_variable>,
      std::shared_ptr<basic_instruction>>;

    basic_file(std::filesystem::path const& _path);
    ~basic_file() = default;

    auto path() const -> std::filesystem::path const&;
    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_file&;
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_file&;
    auto dump() -> basic_file&;
    auto dump(std::ostream& _out) -> basic_file&;

  protected:
    std::filesystem::path __filename;
    std::vector<allowed_type> __elements;
};

class basic_class : public basic_element {
  public:
    using allowed_type = std::variant< //
      std::shared_ptr<basic_class>,
      std::shared_ptr<basic_function>,
      std::shared_ptr<basic_variable>>;

    basic_class(std::string const& _name, std::string const& _extends = "");
    virtual ~basic_class() override = default;

    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add, int _visibility) -> basic_class&;
    template<BasicASTElement T, typename... Args>
    auto add(int _visibility, Args... _args) -> basic_class&;

  protected:
    std::string __name;
    std::string __extends;
    std::vector<allowed_type> __public;
    std::vector<allowed_type> __protected;
    std::vector<allowed_type> __private;
};

class basic_code_block : public basic_element {
  public:
    using allowed_type = std::variant< //
      std::shared_ptr<basic_class>,
      std::shared_ptr<basic_code_block>,
      std::shared_ptr<basic_variable>,
      std::shared_ptr<basic_instruction>>;

    basic_code_block(std::string const& _prefix = "");
    virtual ~basic_code_block() override = default;

    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_code_block&;
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_code_block&;

  protected:
    std::string __prefix;
    std::vector<allowed_type> __elements;
};

class basic_function : public basic_element {
  public:
    basic_function(std::string const& _name,
                   std::string const& _return_type = "",
                   int _modifiers = 0);
    virtual ~basic_function() override = default;

    template<BasicASTElement T>
    auto add(std::shared_ptr<T> _to_add) -> basic_function&;
    template<BasicASTElement T, typename... Args>
    auto add(Args... _args) -> basic_function&;
    auto set_modifiers(int _modifiers) -> basic_function&;

  protected:
    std::string __name;
    std::vector<std::shared_ptr<basic_variable>> __parameters;
    std::string __return_type;
    int __modifiers{ 0 };
    std::shared_ptr<basic_code_block> __body{ nullptr };
};

class basic_variable : public basic_element {
  public:
    basic_variable(std::string const& _name, std::string const& _type, int _modifiers = 0);
    virtual ~basic_variable() override = default;

    auto add(std::shared_ptr<basic_code_block> _initialization) -> basic_variable&;
    template<typename... Args>
    auto add(Args... _args) -> basic_variable&;
    auto set_modifiers(int _modifiers) -> basic_variable&;

  protected:
    std::string __name;
    std::string __type;
    int __modifiers{ 0 };
    std::shared_ptr<basic_code_block> __initialization{ nullptr };
};

class basic_instruction : public basic_element {
  public:
    basic_instruction(std::string const& _code);
    virtual ~basic_instruction() override = default;

    auto add(std::shared_ptr<basic_code_block> _body) -> basic_instruction&;
    template<typename... Args>
    auto add(Args... _args) -> basic_instruction&;

  protected:
    std::string __instruction;
    std::shared_ptr<basic_code_block> __body{ nullptr };
};
} // namespace ast
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
} // namespace zpt

template<zpt::ast::BasicASTElement T>
auto zpt::ast::basic_file::add(std::shared_ptr<T> _to_add) -> basic_file& {
    this->__elements.push_back(_to_add);
    return (*this);
}

template<zpt::ast::BasicASTElement T, typename... Args>
auto zpt::ast::basic_file::add(Args... _args) -> basic_file& {
    return this->add<T>(std::make_shared<T>(std::forward<Args>(_args)...));
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
