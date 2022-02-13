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

    auto operator->() -> zpt::json&;
    auto operator*() -> zpt::json&;

    auto parse_param(std::string const& _param) -> zpt::json;

  private:
    zpt::json __root;
};
} // namespace zpt
