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
 * re2c lexer for functional-style expressions (e.g. `gt(integer(0,2),float(1,2,3))`).
 * Ported from the original flexc++ lexer in Functional.f. Each start
 * condition becomes one ordinary (non-`--conditions`) re2c block compiled
 * into one Re2cFunctionalLexer method; YYCURSOR/YYLIMIT/YYMARKER map onto
 * the buffer pointers owned by Re2cFunctionalLexer, and YYFILL maps onto
 * Re2cFunctionalLexer::yyfill().
 *
 * captureMatch() is called as the first statement of every rule action that
 * either returns a token or participates in a more()-chain, mirroring
 * flexc++'s matched_(). Any more() call in an action must come AFTER
 * captureMatch(), since more() affects how the *next* rule's match combines
 * with what was just captured here, not this one. Rules that neither return
 * a token nor build towards a later more()-chain (plain whitespace) skip
 * captureMatch() entirely, matching flexc++'s empty-bodied whitespace rule.
 *
 * Number accumulation chains exactly as in Functional.f: INITIAL's `[0-9]`
 * rule captures the first digit, arms more(), and switches to `number`;
 * `number`'s `[0-9.]*` rule (which may legitimately match zero characters)
 * then appends the rest and returns NUMBER. lexNumber() intentionally has
 * no catch-all `* {}` rule - adding one would let re2c's longest-match
 * policy prefer consuming one stray byte (length 1) over the legitimate
 * zero-length match of `[0-9.]*`, corrupting whatever follows the number.
 * Since `[0-9.]*` already matches every possible input (including zero
 * characters), no catch-all is needed or safe here.
 *
 * Every greedy `[^...]+` exclusion class excludes `\x00`, for the same
 * reason established in URI.re: yyfill() zero-pads the buffer with
 * synthetic NUL bytes once the underlying stream is exhausted, and this
 * grammar (like URI, unlike JSON/HTTP) has no explicit "value complete"
 * signal short of real stream EOF.
 */

#include <sstream>
#include <zapata/functional/Re2cFunctionalLexer.h>
#include <FunctionalParser.bison.h>

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

auto zpt::Re2cFunctionalLexer::lexInitial() -> int {
    /*!re2c
        [0-9] {
            this->captureMatch();
            this->more();
            this->begin(re2c_functional_cond::number);
            return 0;
        }
        [^\x00()", 0-9\n\r\f\t]+ {
            this->captureMatch();
            return VARIABLE;
        }
        "(" {
            this->captureMatch();
            return LPAREN;
        }
        ")" {
            this->captureMatch();
            return RPAREN;
        }
        "," {
            this->captureMatch();
            return COMMA;
        }
        "\"" {
            this->begin(re2c_functional_cond::quoted);
            return 0;
        }
        [ \n\r\f\t] {
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cFunctionalLexer::lexQuoted() -> int {
    /*!re2c
        "\"" {
            this->captureMatch();
            std::string _content(this->matched());
            this->setMatched(_content.substr(0, _content.length() - 1));
            this->begin(re2c_functional_cond::INITIAL);
            return STRING;
        }
        [^\x00"]+ {
            this->captureMatch();
            this->more();
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cFunctionalLexer::lexNumber() -> int {
    /*!re2c
        [0-9.]* {
            this->captureMatch();
            this->begin(re2c_functional_cond::INITIAL);
            return NUMBER;
        }
    */
}
