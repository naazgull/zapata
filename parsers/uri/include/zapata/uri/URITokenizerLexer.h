#pragma once

#include <zapata/json/JSONClass.h>
#include <zapata/uri/URILexer.h>

namespace zpt {

class URITokenizerLexer : public URILexer {
  public:
    URITokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~URITokenizerLexer();

    auto switchRoots(zpt::json& _root) -> void;
    auto justLeave() -> void;

    auto operator->() -> zpt::json&;
    auto operator*() -> zpt::json&;

    auto parse_param(std::string const& _param) -> zpt::json;

  private:
    zpt::json __root;
};
} // namespace zpt
