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

/**
 * @file Re2cJSONLexer.h
 * @brief Hand-written re2c-backed replacement for the generated flexc++ JSONLexer/JSONLexerBase.
 *
 * Owns the re2c buffer/cursor state (YYCURSOR/YYLIMIT/YYMARKER-equivalent pointers)
 * and is the only place that ever reads bytes from the underlying std::istream.
 * Mirrors zpt::Re2cHTTPLexer's buffer-management design exactly; see that class
 * for the rationale behind fill()/yyfill()/captureMatch()/syncBackToStream().
 *
 * @see zpt::JSONTokenizerLexer
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace zpt {

/**
 * @brief re2c lexer start conditions (replaces flexc++'s StartCondition_ enum).
 *
 * The original flexc++ lexer also declared a `number` start condition; it is
 * omitted here because the number rule never calls begin(StartCondition_::number)
 * in JSON.f - numbers are matched and returned directly from INITIAL.
 *
 * kw_true/kw_false/kw_null/kw_undefined/kw_lambda exist so that "true",
 * "false", "null", "undefined" and "lambda(...)" are not matched as flat
 * literals directly inside lexInitial()'s shared dispatch. re2c computes one
 * YYFILL bound per dispatch point, sized to the longest literal reachable
 * from it - if these keywords stayed in lexInitial, every other branch
 * sharing that entry point (including the single-byte "}"/"]" that close a
 * top-level value) would be forced to wait for up to 9 bytes ("undefined")
 * before the DFA even looks at the first character. Over a live socket that
 * stays open after sending a short, complete value, that wait never ends.
 * Splitting each keyword into its own start condition, entered via a
 * single-byte trigger ("t"/"f"/"n"/"u"/"l") from lexInitial, isolates its
 * bound to its own method so lexInitial's shared entry only ever needs 1 byte.
 */
enum class re2c_json_cond {
    INITIAL,
    string,
    string_single,
    escaped,
    unicode,
    regexp,
    kw_true,
    kw_false,
    kw_null,
    kw_undefined,
    kw_lambda,
};

/**
 * @brief Buffer-owning re2c lexer base class.
 *
 * Replaces the generated `JSONLexerBase`/`JSONLexer` pair. Unlike those, this
 * class is hand-written once and stays stable across builds; only the DFA body
 * inside each lexXxx() method is produced by re2c (as an included code
 * fragment), not the surrounding class.
 */
class Re2cJSONLexer {
  public:
    explicit Re2cJSONLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~Re2cJSONLexer();

    Re2cJSONLexer(Re2cJSONLexer const&) = delete;
    auto operator=(Re2cJSONLexer const&) -> Re2cJSONLexer& = delete;

    /** @brief Returns the next token, or a value <= 0 at end-of-message/EOF. */
    auto lex() -> int;

    /** @brief Text of the most recently completed token (mirrors flexc++'s matched()). */
    auto matched() const -> std::string const&;
    /** @brief Overwrites the current matched text (mirrors flexc++'s setMatched()). */
    auto setMatched(std::string const& _text) -> void;
    /** @brief Marks that the next match should append to, not replace, matched(). */
    auto more() -> void;

    /** @brief Current lexer start condition. */
    auto startCondition() const -> re2c_json_cond;
    /** @brief Switches the lexer start condition. */
    auto begin(re2c_json_cond _condition) -> void;

    /**
     * @brief Marks lexing as finished for this message; lex() will return <= 0 from now on.
     *
     * Unlike flexc++'s exception-based leave() (which threw to unwind out of a
     * shared state-machine loop), this is a plain flag: since each start
     * condition is now an ordinary C++ method, a rule that wants to stop simply
     * returns directly. justLeave() (called once by the grammar after the
     * top-level rule reduces) just makes that final/idempotent.
     */
    auto leave(int _retValue) -> void;

    /** @brief 1-based input line number, for diagnostics. */
    auto lineNr() const -> std::size_t;

    /** @brief Re-targets the lexer at a fresh input/output stream pair, resetting all state. */
    auto switchStreams(std::istream& _in = std::cin, std::ostream& _out = std::cout) -> void;

    /**
     * @brief Pushes back onto the input stream any buffered-but-unconsumed bytes.
     *
     * Must be called once after parse() returns (success or failure) so the
     * istream's read position ends up exactly at the first byte after the
     * parsed JSON value.
     */
    auto syncBackToStream() -> void;

    // Quirk preserved from the generated JSONLexerBase: incremented on `{`/`[`,
    // decremented on `}`/`]`. Nothing in the grammar reads it, but it is kept
    // as public lexer state in case external code or future grammar rules rely
    // on it (matches the original's visibility).
    std::size_t d_paren_count{ 0 };

    // Tracks which caller start condition (string/string_single/regexp) to
    // return to once an escaped/unicode sequence has been fully decoded.
    re2c_json_cond d_intermediate_state{ re2c_json_cond::INITIAL };

  protected:
    /**
     * @brief Ensures at least `_need` real bytes are available between cursor and limit.
     *
     * Loops over istream reads (handling short reads). Returns false if true
     * stream EOF is reached before `_need` real bytes could be supplied.
     */
    auto fill(std::size_t _need) -> bool;

    /**
     * @brief The YYFILL primitive used by the generated DFA code.
     *
     * Calls fill(); if real bytes run out before `_need` is satisfied (true
     * stream EOF), zero-pads up to `_need` bytes so the DFA's bounds checks
     * can still succeed. __data_limit tracks where the real bytes end, so
     * syncBackToStream() never pushes these synthetic bytes back.
     */
    auto yyfill(std::size_t _need) -> void;

    /**
     * @brief Records [__token_start, __cursor) as the current match.
     *
     * Mirrors flexc++'s more()/matched() contract: if more() was called since
     * the last match, the new text is appended instead of replacing matched().
     */
    auto captureMatch() -> void;

    /**
     * @brief Calls leave(0) iff the just-returned terminal completed the
     * top-level JSON value (d_paren_count back to 0).
     *
     * Bison's LALR parser needs one token of lookahead after the final
     * terminal of the top-level value before it can reduce/accept - without
     * this, that lookahead fetch would read into whatever follows in the
     * stream (e.g. a second back-to-back JSON value), since the grammar's
     * own justLeave() actions only run *after* that lookahead has already
     * been fetched. Calling this from the lexer, at the point the
     * completing terminal is returned, arms leave() one call early so the
     * *next* lex() call (the lookahead) short-circuits without touching the
     * stream.
     */
    auto leaveIfComplete() -> void;

    // Per-start-condition DFA dispatch methods. Their bodies are produced by
    // re2c from JSON.re; this class and its method declarations are
    // hand-written and stable across builds.
    auto lexInitial() -> int;
    auto lexString() -> int;
    auto lexStringSingle() -> int;
    auto lexEscaped() -> int;
    auto lexUnicode() -> int;
    auto lexRegexp() -> int;
    auto lexKwTrue() -> int;
    auto lexKwFalse() -> int;
    auto lexKwNull() -> int;
    auto lexKwUndefined() -> int;
    auto lexKwLambda() -> int;

    std::istream* __in;
    std::ostream* __out;

    std::vector<char> __buffer;
    char* __cursor{ nullptr };
    char* __limit{ nullptr };
    char* __marker{ nullptr };
    char* __token_start{ nullptr };
    // Real bytes end here; [__data_limit, __limit) is synthetic zero padding
    // appended once the istream has reached true EOF.
    char* __data_limit{ nullptr };

    re2c_json_cond __condition{ re2c_json_cond::INITIAL };
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
