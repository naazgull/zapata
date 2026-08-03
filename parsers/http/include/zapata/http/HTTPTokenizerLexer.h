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

#include <zapata/http/HTTPObj.h>
#include <zapata/http/Re2cHTTPLexer.h>

namespace zpt {

class HTTPTokenizerLexer : public Re2cHTTPLexer {
  public:
    /** @brief Constructs with given input and output streams. */
    HTTPTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    /** @brief Destructor. */
    virtual ~HTTPTokenizerLexer();

    /** @brief Sets the request object to populate during parsing. */
    auto switchRoots(zpt::http::basic_request& _root) -> void;
    /** @brief Sets the reply object to populate during parsing. */
    auto switchRoots(zpt::http::basic_reply& _root) -> void;
    /** @brief Calls leave(0) to signal lexing completion. */
    auto justLeave() -> void;

    /** @brief Initializes the request/reply type based on the message. */
    auto init(int _in_type) -> void;
    /** @brief Processes the HTTP version token. */
    auto version() -> void;
    /** @brief Processes body content from the lexer. */
    auto body() -> void;
    /** @brief Processes the URL/request-target token. */
    auto url() -> void;
    /** @brief Processes the status code token. */
    auto status() -> void;

    /** @brief Adds the current token to the appropriate data structure. */
    auto add() -> void;

    /**
     * @brief Flushes any pending body/chunked content and leaves the lexer.
     *
     * Called once by the grammar after `headers` reduces. Factors out the
     * duplicated end-of-message block that appeared identically in both
     * alternatives of HTTP.b's `exp` rule.
     */
    auto finishMessage() -> void;

    /** @brief Current header name being parsed. */
    std::string __header_name;
    /** @brief Pointer to the request object being populated during parsing. */
    zpt::http::basic_request* __root_req{ nullptr };
    /** @brief Pointer to the reply object being populated during parsing. */
    zpt::http::basic_reply* __root_rep{ nullptr };
    /** @brief Type identifier for the current root (request or reply). */
    int __root_type;
};
} // namespace zpt
