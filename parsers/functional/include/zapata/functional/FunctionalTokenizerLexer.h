#pragma once

#include <zapata/json.h>
#include <zapata/functional/FunctionalLexer.h>

namespace zpt {

class FunctionalTokenizerLexer : public FunctionalLexer {
  public:
    FunctionalTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~FunctionalTokenizerLexer();

    auto switchRoots(zpt::json& _root) -> void;
    auto justLeave() -> void;
    auto clear() -> void;

    auto set_string() -> void;
    auto set_number() -> void;
    auto set_variable() -> void;
    auto push_expression() -> void;
    auto add_param() -> void;

  private:
    zpt::json __root;
    std::stack<zpt::json> __stack;
};
} // namespace zpt
