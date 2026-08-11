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

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace zpt {

enum class re2c_uri_cond {
    INITIAL,
    scheme,
    server_path,
    server,
    path,
    params,
    placeholder,
    function,
    anchor
};

class Re2cURILexer {
  public:
    explicit Re2cURILexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~Re2cURILexer();

    Re2cURILexer(Re2cURILexer const&) = delete;
    auto operator=(Re2cURILexer const&) -> Re2cURILexer& = delete;

    /**
     * @brief Returns the next token, or a value <= 0 at end-of-message/EOF.
     * @return Token value (positive integer), or 0 or negative at end-of-message/EOF.
     */
    auto lex() -> int;

    /**
     * @brief Text of the most recently completed token (mirrors flexc++'s matched()).
     * @return Const reference to the matched string.
     */
    auto matched() const -> std::string const&;
    /**
     * @brief Overwrites the current matched text (mirrors flexc++'s setMatched()).
     * @param _text New matched text.
     */
    auto setMatched(std::string const& _text) -> void;
    /**
     * @brief Marks that the next match should append to, not replace, matched().
     * @return void (internal state flag set).
     */
    auto more() -> void;

    /**
     * @brief Current lexer start condition.
     * @return Current start condition enum value.
     */
    auto startCondition() const -> re2c_uri_cond;
    /**
     * @brief Switches the lexer start condition.
     * @param _condition New start condition.
     */
    auto begin(re2c_uri_cond _condition) -> void;

    /**
     * @brief Marks lexing as finished for this message; lex() will return <= 0 from now on.
     * @param _retValue The value that lex() will return when called after this.
     */
    auto leave(int _retValue) -> void;

    /**
     * @brief 1-based input line number, for diagnostics.
     * @return Current line number in the input stream.
     */
    auto lineNr() const -> std::size_t;

    /**
     * @brief Re-targets the lexer at a fresh input/output stream pair, resetting all state.
     * @param _in Input stream to read from.
     * @param _out Output stream for errors.
     */
    auto switchStreams(std::istream& _in = std::cin, std::ostream& _out = std::cout) -> void;

    /**
     * @brief Pushes back onto the input stream any buffered-but-unconsumed bytes.
     * @return void (internal buffer state adjusted).
     */
    auto syncBackToStream() -> void;

    /**
     * @brief Lexing function for the INITIAL start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexInitial() -> int;
    /**
     * @brief Lexing function for the scheme start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexScheme() -> int;
    /**
     * @brief Lexing function for the server-path start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexServerPath() -> int;
    /**
     * @brief Lexing function for the server start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexServer() -> int;
    /**
     * @brief Lexing function for the path start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexPath() -> int;
    /**
     * @brief Lexing function for the params start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexParams() -> int;
    /**
     * @brief Lexing function for the placeholder start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexPlaceholder() -> int;
    /**
     * @brief Lexing function for the function start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexFunction() -> int;
    /**
     * @brief Lexing function for the anchor start condition.
     * @return Token value or negative on EOF/error.
     */
    auto lexAnchor() -> int;

    std::string d_path_helper;
    re2c_uri_cond d_intermediate_state{ re2c_uri_cond::INITIAL };
    bool d_part_is_placeholder{ false };
    std::string d_server_part;

  protected:
    auto fill(std::size_t _need) -> bool;
    auto yyfill(std::size_t _need) -> void;
    auto captureMatch() -> void;
    auto readRaw(std::size_t _n) -> std::string;

    std::istream* __in;
    std::ostream* __out;

    std::vector<char> __buffer;
    char* __cursor{ nullptr };
    char* __limit{ nullptr };
    char* __marker{ nullptr };
    char* __token_start{ nullptr };
    char* __data_limit{ nullptr };

    re2c_uri_cond __condition{ re2c_uri_cond::INITIAL };
    std::string __matched;
    bool __more{ false };
    bool __left{ false };
    bool __eof{ false };
    int __leave_value{ 0 };
    std::size_t __line_nr{ 1 };

  private:
    auto resetBuffer() -> void;
};

} // namespace zpt
