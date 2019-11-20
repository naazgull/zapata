#pragma once

#include <zapata/http/config.h>

#include <zapata/http/HTTPLexer.h>
#include <zapata/http/HTTPObj.h>

namespace zpt {

class HTTPTokenizerLexer : public HTTPLexer {
  public:
    HTTPTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~HTTPTokenizerLexer();

    void switchRoots(HTTPReq& _root);
    void switchRoots(HTTPRep& _root);
    void justLeave();

    void init(zpt::http::message_type _in_type);
    void body();
    void url();
    void status();

    void add();
    void name();
    void value();

    std::string __header_name;
    std::string __param_name;
    HTTPReqT* __root_req;
    HTTPRepT* __root_rep;
    zpt::http::message_type __root_type;
};
} // namespace zpt
