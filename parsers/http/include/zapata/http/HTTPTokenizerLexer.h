#pragma once

#include <zapata/http/HTTPLexer.h>
#include <zapata/http/HTTPObj.h>

namespace zpt {

class HTTPTokenizerLexer : public HTTPLexer {
  public:
    HTTPTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~HTTPTokenizerLexer();

    auto switchRoots(zpt::http::basic_request& _root) -> void;
    auto switchRoots(zpt::http::basic_reply& _root) -> void;
    auto justLeave() -> void;

    auto init(int _in_type) -> void;
    auto version() -> void;
    auto body() -> void;
    auto url() -> void;
    auto status() -> void;

    auto add() -> void;

    std::string __header_name;
    zpt::http::basic_request* __root_req;
    zpt::http::basic_reply* __root_rep;
    int __root_type;
};
} // namespace zpt
