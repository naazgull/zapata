#pragma once

#include <zapata/http/config.h>

#include <zapata/http/HTTPLexer.h>
#include <zapata/http/HTTPObj.h>

namespace zpt {

class HTTPTokenizerLexer : public HTTPLexer {
  public:
    HTTPTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~HTTPTokenizerLexer();

    auto switchRoots(HTTPReq& _root) -> void;
    auto switchRoots(HTTPRep& _root) -> void;
    auto justLeave() -> void;

    auto init(zpt::http::message_type _in_type) -> void;
    auto version() -> void;
    auto body() -> void;
    auto url() -> void;
    auto status() -> void;

    auto add() -> void;
    auto name() -> void;
    auto value() -> void;

    std::string __header_name;
    std::string __param_name;
    HTTPReqT* __root_req;
    HTTPRepT* __root_rep;
    zpt::http::message_type __root_type;
};
} // namespace zpt
