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
 * re2c lexer for URI parsing. Ported from the original flexc++ lexer in
 * URI.f. Each start condition becomes one ordinary (non-`--conditions`) re2c
 * block compiled into one Re2cURILexer method; YYCURSOR/YYLIMIT/YYMARKER map
 * onto the buffer pointers owned by Re2cURILexer, and YYFILL maps onto
 * Re2cURILexer::yyfill(), which is the only place that ever reads bytes from
 * the underlying istream.
 *
 * captureMatch() is called as the first statement of every rule action that
 * either returns a token or participates in a more()-chain, mirroring
 * flexc++'s matched_(), which updates d_matched for every rule unconditionally
 * before that rule's action code runs. Any more() call in an action must come
 * AFTER captureMatch(), since more() affects how the *next* rule's match
 * combines with what was just captured here, not this one.
 *
 * The `function` start condition from the original flexc++ lexer is dead code
 * (nothing ever begins it); it is intentionally not ported.
 *
 * Pattern differences from the original:
 *  - In INITIAL state, `.` and `..` are explicit rules matched before the
 *    STRING pattern `[^\x00{:/?#.*]+` (which excludes `.`). In path state,
 *    the STRING pattern `[^\x00{/?#]+` does NOT exclude `.` — dots in the middle
 *    of a path segment like `foo.bar` become one STRING token, matching the
 *    original's behavior exactly.
 *  - server_path's `[^/]` rule matches exactly one character (no `+` suffix),
 *    sets matched() via d_path_helper, transitions to path, and returns 0
 *    (no token) to continue lexing — same as flexc++'s one-character-at-a-time
 *    accumulation.
 *  - Every greedy `[^...]+` exclusion class (and `lexAnchor`'s catch-all)
 *    additionally excludes `\x00`. flexc++ read one real byte at a time
 *    straight off the stream and stopped cold at genuine EOF; re2c's
 *    YYFILL-based buffering instead zero-pads the tail of the buffer with
 *    synthetic NUL bytes once the stream is exhausted, so the DFA's
 *    lookahead can still succeed. Since the URI grammar (unlike JSON/HTTP)
 *    has no explicit "value complete" signal and legitimately lexes all the
 *    way to real stream EOF, a `[^...]+` pattern that doesn't exclude NUL
 *    would greedily consume that synthetic padding forever, growing the
 *    buffer without bound.
 */

#include <string>
#include <sstream>
#include <zapata/uri/Re2cURILexer.h>
#include <URIParser.bison.h>

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

auto zpt::Re2cURILexer::lexInitial() -> int {
    /*!re2c
        [^\x00{:/?#.*]+ {
            this->captureMatch();
            this->begin(re2c_uri_cond::scheme);
            this->d_part_is_placeholder = false;
            return STRING;
        }
        "{" {
            this->d_path_helper.assign("{");
            this->d_intermediate_state = re2c_uri_cond::scheme;
            this->begin(re2c_uri_cond::placeholder);
            return 0;
        }
        "/" {
            this->captureMatch();
            this->begin(re2c_uri_cond::path);
            return SLASH;
        }
        "." {
            this->captureMatch();
            this->begin(re2c_uri_cond::path);
            return DOT;
        }
        ".." {
            this->captureMatch();
            this->begin(re2c_uri_cond::path);
            return DOT_DOT;
        }
        "?" {
            this->captureMatch();
            this->begin(re2c_uri_cond::params);
            return QMARK;
        }
        "#" {
            this->captureMatch();
            this->begin(re2c_uri_cond::anchor);
            return CARDINAL;
        }
        "*" {
            this->captureMatch();
            return STAR;
        }
        * { return 0; }
    */
}

auto zpt::Re2cURILexer::lexScheme() -> int {
    /*!re2c
        ":" {
            this->captureMatch();
            return DOUBLE_DOT;
        }
        "." {
            this->captureMatch();
            this->begin(re2c_uri_cond::path);
            return DOT;
        }
        ".." {
            this->captureMatch();
            this->begin(re2c_uri_cond::path);
            return DOT_DOT;
        }
        "/" {
            this->captureMatch();
            this->begin(re2c_uri_cond::server_path);
            return SLASH;
        }
        * { return 0; }
    */
}

auto zpt::Re2cURILexer::lexServerPath() -> int {
    /*!re2c
        [^\x00/] {
            this->captureMatch();
            this->d_path_helper.assign(this->matched());
            this->begin(re2c_uri_cond::path);
            return 0;
        }
        "/" {
            this->captureMatch();
            this->begin(re2c_uri_cond::server);
            return SLASH;
        }
        * { return 0; }
    */
}

auto zpt::Re2cURILexer::lexServer() -> int {
    /*!re2c
        [^\x00:@/{]+ {
            this->captureMatch();
            this->d_server_part.assign(this->matched());
            this->d_part_is_placeholder = false;
            return STRING;
        }
        "/" {
            this->captureMatch();
            this->setMatched(this->d_server_part);
            this->begin(re2c_uri_cond::path);
            return SLASH;
        }
        ":" {
            this->captureMatch();
            this->setMatched(this->d_server_part);
            return DOUBLE_DOT;
        }
        "@" {
            this->captureMatch();
            this->setMatched(this->d_server_part);
            return AT;
        }
        "{" {
            this->d_path_helper.assign("{");
            this->d_intermediate_state = re2c_uri_cond::server;
            this->begin(re2c_uri_cond::placeholder);
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cURILexer::lexPath() -> int {
    /*!re2c
        "/" {
            this->captureMatch();
            this->d_path_helper.assign("");
            return SLASH;
        }
        "{" {
            this->d_path_helper.assign("{");
            this->d_intermediate_state = re2c_uri_cond::path;
            this->begin(re2c_uri_cond::placeholder);
            return 0;
        }
        "?" {
            this->captureMatch();
            this->begin(re2c_uri_cond::params);
            return QMARK;
        }
        "#" {
            this->captureMatch();
            this->begin(re2c_uri_cond::anchor);
            return CARDINAL;
        }
        [^\x00{/?#]+ {
            this->captureMatch();
            std::string _m(this->matched());
            _m.insert(0, this->d_path_helper);
            this->d_path_helper.assign("");
            this->setMatched(_m);
            this->d_part_is_placeholder = false;
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cURILexer::lexParams() -> int {
    /*!re2c
        "=" {
            this->captureMatch();
            return EQ;
        }
        "&" {
            this->captureMatch();
            return E;
        }
        "#" {
            this->captureMatch();
            this->begin(re2c_uri_cond::anchor);
            return CARDINAL;
        }
        [^\x00=&#]+ {
            this->captureMatch();
            this->d_part_is_placeholder = false;
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cURILexer::lexPlaceholder() -> int {
    /*!re2c
        [^\x00}]+ {
            this->captureMatch();
            this->d_path_helper.append(this->matched());
            return 0;
        }
        "}" {
            this->captureMatch();
            this->d_path_helper.append(this->matched());
            this->setMatched(this->d_path_helper);
            this->d_path_helper.assign("");
            this->begin(this->d_intermediate_state);
            this->d_part_is_placeholder = true;
            return STRING;
        }
        * { return 0; }
    */
}

auto zpt::Re2cURILexer::lexFunction() -> int {
    // Dead code — the original flexc++ lexer never transitions into this
    // start condition. Kept as a no-op to match the original API surface.
    return 0;
}

auto zpt::Re2cURILexer::lexAnchor() -> int {
    /*!re2c
        [^\x00]+ {
            this->captureMatch();
            this->d_part_is_placeholder = false;
            return STRING;
        }
        * { return 0; }
    */
}
