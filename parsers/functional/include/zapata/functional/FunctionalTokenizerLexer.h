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

#include <stack>
#include <zapata/functional/Re2cFunctionalLexer.h>
#include <zapata/json/JSONClass.h>

namespace zpt {

class FunctionalTokenizerLexer : public Re2cFunctionalLexer {
  public:
    /** @brief Constructs with given input and output streams. */
    FunctionalTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    /** @brief Destructor. */
    virtual ~FunctionalTokenizerLexer();

    /** @brief Sets the JSON root node to populate during parsing. */
    auto switchRoots(zpt::json& _root) -> void;
    /** @brief Calls leave(0) to signal lexing completion. */
    auto justLeave() -> void;
    /** @brief Empties the working value stack. */
    auto clear() -> void;

    /** @brief Pushes a string literal (the de-quoted matched() text) onto the stack. */
    auto set_string() -> void;
    /** @brief Pushes an integer or floating-point literal parsed from matched(). */
    auto set_number() -> void;
    /** @brief Pushes a function-call node (or the root, for the first token) with `functor` set. */
    auto set_variable() -> void;
    /** @brief Pops the top of the stack and appends it as a param of the new top. */
    auto add_param() -> void;

  private:
    /** @brief The JSON root node being populated during parsing. */
    zpt::json __root;
    /** @brief Stack for building the JSON AST during tokenization. */
    std::stack<zpt::json> __stack;
};
} // namespace zpt
