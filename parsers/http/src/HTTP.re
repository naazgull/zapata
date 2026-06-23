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

/*
 * re2c lexer for HTTP/UPnP request and reply messages. Ported from the
 * original flexc++ lexer in HTTP.f. Each start condition becomes one
 * ordinary (non-`--conditions`) re2c block compiled into one Re2cHTTPLexer
 * method; YYCURSOR/YYLIMIT/YYMARKER map onto the buffer pointers owned by
 * Re2cHTTPLexer, and YYFILL maps onto Re2cHTTPLexer::yyfill(), which is the
 * only place that ever reads bytes from the underlying istream.
 *
 * captureMatch() is called as the first statement of every rule action that
 * either returns a token or participates in a more()-chain, mirroring
 * flexc++'s matched_(), which updates d_matched for every rule unconditionally
 * before that rule's action code runs. Any more() call in an action must come
 * AFTER captureMatch(), since more() affects how the *next* rule's match
 * combines with what was just captured here, not this one.
 *
 * The original flexc++ lexer also declared a `crlf` start condition that
 * nothing ever transitioned into (confirmed dead code; see HTTP.f's absence
 * of any `begin(StartCondition_::crlf)`); it is intentionally not ported.
 *
 * Preserved-as-is quirks (do not "fix" without separately deciding to):
 *  - header name matching against content-length/transfer-encoding/trailer
 *    is case-insensitive, but the *original-case* name is what gets stored
 *    (only the local comparison is lowercased).
 *  - the chunked-encoding detection only recognizes the literal value
 *    " chunked" (note the single leading space, since the lexer-level value
 *    text includes everything right after the colon, before any trimming)
 *    - this is fragile against extra whitespace or compound encodings,
 *    exactly as it was originally.
 *  - chunk-extensions (the optional `;key=value` suffix after the hex chunk
 *    size) are not handled/stripped - they would be fed directly into the
 *    hex parse, same as before.
 *  - only a single chunked trailer header is supported, and its *value* is
 *    read off the wire and discarded, never stored anywhere - this mirrors
 *    the original exactly (d_chunked_trailer only ever holds the trailer's
 *    declared *name* from the headers phase; the body-phase trailer line is
 *    consumed purely to advance past it).
 *  - chunk data is appended to d_chunked together with its trailing CRLF,
 *    and then the *entire* accumulated d_chunked is passed through
 *    zpt::trim() after every chunk - exactly as flexc++'s original
 *    `d_chunked.insert(...); zpt::trim(d_chunked);` did. In practice this
 *    strips the just-appended trailing CRLF (the intended effect) but, like
 *    the original, would also strip any incidental leading/trailing
 *    whitespace bytes that happen to be actual chunk content.
 */

#include <algorithm>
#include <sstream>
#include <zapata/base.h>
#include <zapata/http/HTTPTokenizerLexer.h>
#include <HTTPParser.bison.h>

namespace {
/*!re2c
    re2c:api:style = free-form;
    re2c:define:YYCTYPE = "char";
    re2c:define:YYCURSOR = "this->__cursor";
    re2c:define:YYLIMIT = "this->__limit";
    re2c:define:YYMARKER = "this->__marker";
    re2c:define:YYFILL = "this->yyfill(@@);";
*/
} // namespace

auto zpt::Re2cHTTPLexer::lexInitial() -> int {
    /*!re2c
        [\n\r\f\t ] { return 0; }
        "GET" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "PUT" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "POST" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "DELETE" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "HEAD" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "TRACE" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "OPTIONS" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "PATCH" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "CONNECT" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "M-SEARCH" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "NOTIFY" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::request);
            return METHOD;
        }
        "HTTP/1.0" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::reply);
            return PROTOCOL_VERSION;
        }
        "HTTP/1.1" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::reply);
            return PROTOCOL_VERSION;
        }
        "UPNP/1.0" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::reply);
            return PROTOCOL_VERSION;
        }
        "UPNP/1.1" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::reply);
            return PROTOCOL_VERSION;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexRequest() -> int {
    /*!re2c
        "HTTP/1.0" {
            this->captureMatch();
            return PROTOCOL_VERSION;
        }
        "HTTP/1.1" {
            this->captureMatch();
            return PROTOCOL_VERSION;
        }
        "UPNP/1.0" {
            this->captureMatch();
            return PROTOCOL_VERSION;
        }
        "UPNP/1.1" {
            this->captureMatch();
            return PROTOCOL_VERSION;
        }
        "\r\n" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::headers);
            return CR_LF;
        }
        [^\r\n* ]+ {
            this->captureMatch();
            return URL;
        }
        "*" {
            this->captureMatch();
            return STAR;
        }
        " " {
            this->captureMatch();
            return SPACE;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexReply() -> int {
    /*!re2c
        [0-9]{3} {
            this->captureMatch();
            return STATUS;
        }
        "\r\n" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::headers);
            return CR_LF;
        }
        [^\r\n ] {
            // Seeds matched() with this one character, then arms more() so
            // statustext's upcoming STRING rule appends to it instead of
            // replacing it - order matters: captureMatch() must run before
            // more(), since more() only affects how the *next* rule's match
            // combines with what was just captured here.
            this->captureMatch();
            this->more();
            this->begin(zpt::re2c_cond::statustext);
            return 0;
        }
        " " {
            this->captureMatch();
            return SPACE;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexHeaders() -> int {
    /*!re2c
        ":" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::headerval);
            return COLON;
        }
        "\r\n" "\r\n" {
            // End of headers: this single rule consumes the *entire*
            // terminating blank-line sequence (2-4 bytes, in any CRLF/LF
            // combination) atomically. This is the structural fix for the
            // historical trailing-byte bug: the original flexc++ rule only
            // matched one line-ending, then speculatively peeked a second
            // raw byte via get_() to decide whether headers were done, and
            // one of its branches (the no-body case, leave(-1) immediately)
            // consumed that peeked byte without a matching pushback or
            // second get_(), leaving it stuck in the stream. Here, no peek
            // exists - the DFA's longest-match already accounts for both
            // line endings before any branch runs.
            this->captureMatch();
            if (this->d_chunked_body) {
                this->d_chunked_length = -1;
                this->begin(zpt::re2c_cond::chunked_body);
            }
            else if (this->d_content_length != 0) {
                this->begin(zpt::re2c_cond::plain_body);
            }
            else {
                this->leave(-1);
            }
            return CR_LF;
        }
        "\r\n" {
            // A single line ending: headers continue (this is not the
            // terminating blank line - the rule above already claimed that
            // case via longest-match). Stay in `headers` and let the next
            // STRING rule match the following header's name.
            this->captureMatch();
            return CR_LF;
        }
        ([^:\n\r]+) {
            this->captureMatch();
            std::string _m(this->matched());
            std::transform(_m.begin(), _m.end(), _m.begin(), ::tolower);
            if (_m == std::string("content-length")) {
                this->begin(zpt::re2c_cond::contentlengthval);
            }
            else if (_m == std::string("transfer-encoding")) {
                this->begin(zpt::re2c_cond::transferencodingval);
            }
            else if (_m == std::string("trailer")) {
                this->begin(zpt::re2c_cond::trailerval);
            }
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexHeaderval() -> int {
    /*!re2c
        [^\n\r]+ {
            this->captureMatch();
            this->begin(zpt::re2c_cond::headers);
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexStatustext() -> int {
    /*!re2c
        "\r\n" {
            this->captureMatch();
            this->begin(zpt::re2c_cond::headers);
            return CR_LF;
        }
        [^\r\n]+ {
            // If reached directly from lexReply's one-character transition
            // rule, captureMatch() here appends to the single character
            // already captured there (more() was armed by that rule).
            this->captureMatch();
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexContentLengthVal() -> int {
    /*!re2c
        ":" {
            this->captureMatch();
            return COLON;
        }
        [^:\n\r]+ {
            this->captureMatch();
            std::string _s(this->matched());
            zpt::fromstr(_s, &this->d_content_length);
            this->begin(zpt::re2c_cond::headers);
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexTransferEncodingVal() -> int {
    /*!re2c
        ":" {
            this->captureMatch();
            return COLON;
        }
        [^:\n\r]+ {
            this->captureMatch();
            this->d_chunked_body = (this->matched() == std::string(" chunked"));
            this->begin(zpt::re2c_cond::headers);
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexTrailerVal() -> int {
    /*!re2c
        ":" {
            this->captureMatch();
            return COLON;
        }
        [^:\n\r]+ {
            this->captureMatch();
            this->d_chunked_trailer = this->matched();
            this->begin(zpt::re2c_cond::headers);
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cHTTPLexer::lexPlainBody() -> int {
    // Content-length bodies are an exact, known byte count, not a regex
    // pattern (the bytes may be arbitrary, including CR/LF), so this bypasses
    // the DFA entirely via readRaw() rather than porting flexc++'s
    // one-byte-at-a-time `.|\n` accumulation rule.
    std::string _body = this->readRaw(this->d_content_length);
    this->setMatched(_body);
    this->leave(-1);
    return 0;
}

auto zpt::Re2cHTTPLexer::lexChunkedBody() -> int {
    /*!re2c
        [^\r\n]* "\r\n" {
            // Matches one full text line up to and including its line
            // ending: either a chunk-size line (hex digit run, optionally
            // followed by chunk-extensions that are not stripped, matching
            // the original's lack of chunk-extension support), or - when
            // d_chunked_length == -2 - the trailer value line, whose content
            // is intentionally discarded (see the preserved-quirks note
            // above this file's header comment).
            this->captureMatch();

            if (this->d_chunked_length == -1) {
                std::istringstream _is;
                _is.str(this->matched());
                _is >> std::hex >> this->d_chunked_length;
                this->setMatched("");

                if (this->d_chunked_length == 0) {
                    if (this->d_chunked_trailer.length() == 0) {
                        this->setMatched(this->d_chunked);
                        this->readRaw(2); // discard the final blank-line CRLF
                        this->leave(-1);
                    }
                    else {
                        // Wait for one more line (the trailer's value, to be
                        // discarded) before finalizing - matches the
                        // original's d_chunked_length = -2 sentinel.
                        this->d_chunked_length = -2;
                    }
                }
                else if (this->d_chunked_length > 0) {
                    std::string _chunk_data =
                      this->readRaw(static_cast<std::size_t>(this->d_chunked_length));
                    std::string _chunk_crlf = this->readRaw(2); // mandatory trailing CRLF
                    this->d_chunked.insert(this->d_chunked.length(), _chunk_data);
                    this->d_chunked.insert(this->d_chunked.length(), _chunk_crlf);
                    zpt::trim(this->d_chunked);
                    this->d_chunked_length = -1;
                }
            }
            else if (this->d_chunked_length == -2) {
                this->d_chunked_length = -1;
                this->setMatched(this->d_chunked);
                this->readRaw(2); // discard the final blank-line CRLF
                this->leave(-1);
            }
            return 0;
        }
        * { return 0; }
    */
}
