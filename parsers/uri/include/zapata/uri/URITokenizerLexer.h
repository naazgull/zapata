#pragma once

#include <zapata/json.h>
#include <zapata/events/URILexer.h>

namespace zpt {

class URITokenizerLexer : public URILexer {
  public:
    URITokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~URITokenizerLexer();

    auto switchRoots(zpt::json& _root) -> void;
    auto justLeave() -> void;

    auto init() -> void;
    auto body() -> void;
    auto url() -> void;
    auto status() -> void;

    auto add() -> void;
    auto name() -> void;
    auto value() -> void;

};
} // namespace zpt
