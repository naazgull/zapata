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

#include <zapata/json/JSONClass.h>
#include <zapata/json/Re2cJSONLexer.h>

namespace zpt {
class JSONTokenizerLexer : public Re2cJSONLexer {
  public:
    /** @brief Pointer to the root JSON element being populated during parsing. */
    zpt::JSONElementT* __root{ nullptr };
    /** @brief Type of the root element. */
    zpt::JSONType __root_type;
    /** @brief Parent element for nested structures. */
    zpt::JSONElementT* __parent{ nullptr };

    /** @brief Constructs with given input and output streams. */
    JSONTokenizerLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    /** @brief Destructor. */
    virtual ~JSONTokenizerLexer();

    /** @brief Sets the root JSON object to populate during parsing. */
    auto switchRoots(zpt::json& _root) -> void;
    /** @brief Calls leave(0) to signal lexing completion. */
    auto justLeave() -> void;

    /** @brief Creates and pushes a new element of the given type onto the stack. */
    auto result(zpt::JSONType _in) -> void;
    /** @brief Finishes the current element and attaches it to parent. */
    auto finish(zpt::JSONType _in) -> void;

    /** @brief Initializes an element from a type and string value. */
    auto init(zpt::JSONType _in_type, const std::string _in_str) -> void;
    /** @brief Initializes an element of the given type (null value). */
    auto init(zpt::JSONType _in_type) -> void;
    /** @brief Initializes a boolean element. */
    auto init(bool _in) -> void;
    /** @brief Initializes an integer element. */
    auto init(long long _in) -> void;
    /** @brief Initializes a floating-point element. */
    auto init(double _in) -> void;
    /** @brief Initializes a string element. */
    auto init(std::string const& _in) -> void;
    /** @brief Initializes a lambda element. */
    auto init(zpt::lambda _in) -> void;
    /** @brief Initializes a regex element. */
    auto init(zpt::regex _in) -> void;
    /** @brief Initializes a null element. */
    auto init() -> void;

    /** @brief Adds the current element to its parent container. */
    auto add() -> void;
};
} // namespace zpt
