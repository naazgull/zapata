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
 * @file Re2cHTTPLexer.h
 * @brief Hand-written re2c-backed replacement for the generated flexc++ HTTPLexer/HTTPLexerBase.
 *
 * Owns the re2c buffer/cursor state (YYCURSOR/YYLIMIT/YYMARKER-equivalent pointers)
 * and is the only place that ever reads bytes from the underlying std::istream.
 * This is what structurally fixes the historical trailing-byte bug: lookahead is
 * DFA pointer arithmetic over an internally-owned buffer, never a speculative
 * single-byte stream read followed by a maybe-inconsistent pushback. After a
 * parse completes, sync_back_to_stream() gives back to the istream any buffered
 * bytes that were fetched but never consumed by the grammar, so the stream's
 * read position ends up exactly at the first byte after the HTTP message.
 *
 * @see zpt::HTTPTokenizerLexer
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace zpt {

/**
 * @brief re2c lexer start conditions (replaces flexc++'s StartCondition_ enum).
 *
 * The original flexc++ lexer also declared a `crlf` condition; it is omitted here
 * because nothing in HTTP.f ever transitioned into it (confirmed dead code).
 */
enum class re2c_cond {
    INITIAL,
    request,
    reply,
    headers,
    headerval,
    plain_body,
    chunked_body,
    statustext,
    contentlengthval,
    transferencodingval,
    trailerval,
};

/**
 * @brief Buffer-owning re2c lexer base class.
 *
 * Replaces the generated `HTTPLexerBase`/`HTTPLexer` pair. Unlike those, this
 * class is hand-written once and stays stable across builds; only the DFA body
 * inside lex_() is produced by re2c (as an included code fragment), not the
 * surrounding class.
 */
class Re2cHTTPLexer {
  public:
    explicit Re2cHTTPLexer(std::istream& _in = std::cin, std::ostream& _out = std::cout);
    virtual ~Re2cHTTPLexer();

    Re2cHTTPLexer(Re2cHTTPLexer const&) = delete;
    auto operator=(Re2cHTTPLexer const&) -> Re2cHTTPLexer& = delete;

    /** @brief Returns the next token, or a value <= 0 at end-of-message/EOF. */
    auto lex() -> int;

    /** @brief Text of the most recently completed token (mirrors flexc++'s matched()). */
    auto matched() const -> std::string const&;
    /** @brief Overwrites the current matched text (mirrors flexc++'s setMatched()). */
    auto setMatched(std::string const& _text) -> void;
    /** @brief Marks that the next match should append to, not replace, matched(). */
    auto more() -> void;

    /** @brief Current lexer start condition. */
    auto startCondition() const -> re2c_cond;
    /** @brief Switches the lexer start condition. */
    auto begin(re2c_cond _condition) -> void;

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
     * @brief Reads exactly `_n` raw bytes regardless of content, bypassing the DFA.
     *
     * Used for content-length bodies and chunked chunk-data, where the byte count
     * is known up front and the bytes themselves may be arbitrary (including CR/LF).
     */
    auto readRaw(std::size_t _n) -> std::string;

    /**
     * @brief Pushes back onto the input stream any buffered-but-unconsumed bytes.
     *
     * Must be called once after parse() returns (success or failure) so the
     * istream's read position ends up exactly at the first byte after the HTTP
     * message - this is the fix for the historical trailing-byte bug.
     */
    auto syncBackToStream() -> void;

    // Body/chunk decoding state. Names and types are preserved from the
    // generated HTTPLexerBase so the bison grammar actions (which read/write
    // these fields directly on the scanner/context object) port unchanged.
    std::size_t d_content_length{ 0 };
    long d_chunked_length{ 0 };
    bool d_chunked_body{ false };
    std::string d_chunked_trailer;
    std::string d_chunked;

  protected:
    /**
     * @brief Ensures at least `_need` real bytes are available between cursor and limit.
     *
     * Loops over istream reads (handling short reads). Returns false if true
     * stream EOF is reached before `_need` real bytes could be supplied - the
     * caller (readRaw()) must then stop, since there is no more real content
     * to give back as body/chunk data.
     */
    auto fill(std::size_t _need) -> bool;

    /**
     * @brief The YYFILL primitive used by the generated DFA code.
     *
     * Calls fill(); if real bytes run out before `_need` is satisfied (true
     * stream EOF), zero-pads up to `_need` bytes so the DFA's bounds checks
     * can still succeed - this lets a message that ends exactly at EOF (e.g.
     * a no-body reply on a closing connection) match correctly instead of
     * forcing an artificial failure. syncBackToStream() never pushes back
     * these synthetic padding bytes (tracked via __data_limit).
     */
    auto yyfill(std::size_t _need) -> void;

    /**
     * @brief Records [__token_start, __cursor) as the current match.
     *
     * Mirrors flexc++'s more()/matched() contract: if more() was called since
     * the last match, the new text is appended instead of replacing matched().
     * Called once per completed rule, from each lexXxx() method, immediately
     * before returning a token - this is the single point where the DFA's
     * pointer-range result becomes a std::string, replacing flexc++'s
     * incrementally-built d_matched.
     */
    auto captureMatch() -> void;

    // Per-start-condition DFA dispatch methods. Their bodies are produced by
    // re2c from HTTP.re (one ordinary, non-`--conditions` re2c block per
    // method); only the bodies are generated, this class and its method
    // declarations are hand-written and stable across builds. Each returns
    // a positive token code, 0 to mean "matched but produced no token, call
    // lex() again" (e.g. whitespace skipping), or a value <= 0 supplied by a
    // prior leave() call.
    auto lexInitial() -> int;
    auto lexRequest() -> int;
    auto lexReply() -> int;
    auto lexHeaders() -> int;
    auto lexHeaderval() -> int;
    auto lexStatustext() -> int;
    auto lexContentLengthVal() -> int;
    auto lexTransferEncodingVal() -> int;
    auto lexTrailerVal() -> int;
    auto lexPlainBody() -> int;
    auto lexChunkedBody() -> int;

    std::istream* __in;
    std::ostream* __out;

    std::vector<char> __buffer;
    char* __cursor{ nullptr };
    char* __limit{ nullptr };
    char* __marker{ nullptr };
    char* __token_start{ nullptr };
    // Real bytes end here; [__data_limit, __limit) is synthetic zero padding
    // appended once the istream has reached true EOF, so the DFA's bounds
    // checks (YYLIMIT - YYCURSOR < n) can still succeed when a message ends
    // exactly at end-of-stream. syncBackToStream() must never push padding
    // bytes back onto the istream, only real ones (i.e. only up to __data_limit).
    char* __data_limit{ nullptr };

    re2c_cond __condition{ re2c_cond::INITIAL };
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
