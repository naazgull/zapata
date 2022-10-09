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
