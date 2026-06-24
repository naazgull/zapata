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
 * re2c lexer for JSON values. Ported from the original flexc++ lexer in
 * JSON.f. Each start condition becomes one ordinary (non-`--conditions`)
 * re2c block compiled into one Re2cJSONLexer method; YYCURSOR/YYLIMIT/
 * YYMARKER map onto the buffer pointers owned by Re2cJSONLexer, and YYFILL
 * maps onto Re2cJSONLexer::yyfill().
 *
 * captureMatch() is called as the first statement of every rule action that
 * either returns a token or participates in a more()-chain, mirroring
 * flexc++'s matched_(). Any more() call in an action must come AFTER
 * captureMatch(), since more() affects how the *next* rule's match combines
 * with what was just captured here, not this one.
 *
 * String/regex bodies chain through start conditions exactly as in JSON.f:
 * `string`/`string_single`/`regexp` each strip their own closing-quote-style
 * delimiter char off of matched() when they see a backslash, hand off to
 * `escaped` (recording the caller condition in d_intermediate_state so
 * `escaped`/`unicode` know where to return), and `escaped`/`unicode` replace
 * the trailing escape-sequence bytes in matched() with the decoded character
 * before returning to the caller condition via more().
 */

#include <sstream>
#include <zapata/json/JSONTokenizerLexer.h>
#include <JSONParser.bison.h>

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

auto zpt::Re2cJSONLexer::lexInitial() -> int {
    /*!re2c
        [\n\r\f\t ]+ { return 0; }
        "true" {
            this->captureMatch();
            this->leaveIfComplete();
            return BOOLEAN;
        }
        "false" {
            this->captureMatch();
            this->leaveIfComplete();
            return BOOLEAN;
        }
        "null" {
            this->captureMatch();
            this->leaveIfComplete();
            return NIL;
        }
        "undefined" {
            this->captureMatch();
            this->leaveIfComplete();
            return NIL;
        }
        "lambda(" [^)]+ ")" {
            this->captureMatch();
            this->leaveIfComplete();
            return LAMBDA;
        }
        "{" {
            this->captureMatch();
            ++this->d_paren_count;
            return LCB;
        }
        "}" {
            this->captureMatch();
            --this->d_paren_count;
            this->leaveIfComplete();
            return RCB;
        }
        "[" {
            this->captureMatch();
            ++this->d_paren_count;
            return LB;
        }
        "]" {
            this->captureMatch();
            --this->d_paren_count;
            this->leaveIfComplete();
            return RB;
        }
        "," {
            this->captureMatch();
            return COMMA;
        }
        ":" {
            this->captureMatch();
            return COLON;
        }
        [0-9.e+\-]+ {
            this->captureMatch();
            this->leaveIfComplete();
            if (this->matched().find(".") != std::string::npos ||
                this->matched().find("e+") != std::string::npos) {
                return DOUBLE;
            }
            return INTEGER;
        }
        "\"" {
            this->begin(zpt::re2c_json_cond::string);
            return 0;
        }
        "'" {
            this->begin(zpt::re2c_json_cond::string_single);
            return 0;
        }
        "/" {
            this->begin(zpt::re2c_json_cond::regexp);
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cJSONLexer::lexString() -> int {
    /*!re2c
        "\"" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            this->setMatched(_out);
            this->begin(zpt::re2c_json_cond::INITIAL);
            this->leaveIfComplete();
            return STRING;
        }
        "\\" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            this->setMatched(_out);
            this->more();
            this->d_intermediate_state = zpt::re2c_json_cond::string;
            this->begin(zpt::re2c_json_cond::escaped);
            return 0;
        }
        [^\\"] {
            this->captureMatch();
            this->more();
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cJSONLexer::lexStringSingle() -> int {
    /*!re2c
        "'" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            this->setMatched(_out);
            this->begin(zpt::re2c_json_cond::INITIAL);
            this->leaveIfComplete();
            return STRING;
        }
        "\\" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            this->setMatched(_out);
            this->more();
            this->d_intermediate_state = zpt::re2c_json_cond::string_single;
            this->begin(zpt::re2c_json_cond::escaped);
            return 0;
        }
        [^\\'] {
            this->captureMatch();
            this->more();
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cJSONLexer::lexRegexp() -> int {
    /*!re2c
        "/" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            this->setMatched(_out);
            this->begin(zpt::re2c_json_cond::INITIAL);
            this->leaveIfComplete();
            return REGEX;
        }
        "\\" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            this->setMatched(_out);
            this->more();
            this->d_intermediate_state = zpt::re2c_json_cond::regexp;
            this->begin(zpt::re2c_json_cond::escaped);
            return 0;
        }
        [^\\/] {
            this->captureMatch();
            this->more();
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cJSONLexer::lexEscaped() -> int {
    /*!re2c
        "n" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            _out.append("\n");
            this->setMatched(_out);
            this->more();
            this->begin(this->d_intermediate_state);
            return 0;
        }
        "t" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            _out.append("\t");
            this->setMatched(_out);
            this->more();
            this->begin(this->d_intermediate_state);
            return 0;
        }
        "r" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            _out.append("\r");
            this->setMatched(_out);
            this->more();
            this->begin(this->d_intermediate_state);
            return 0;
        }
        "f" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            _out.append("\f");
            this->setMatched(_out);
            this->more();
            this->begin(this->d_intermediate_state);
            return 0;
        }
        "u" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            this->setMatched(_out);
            this->more();
            this->begin(zpt::re2c_json_cond::unicode);
            return 0;
        }
        "\\" {
            this->captureMatch();
            std::string _out(this->matched());
            _out.erase(_out.length() - 1, 1);
            _out.append("\\");
            this->setMatched(_out);
            this->more();
            this->begin(this->d_intermediate_state);
            return 0;
        }
        [^\\ntrfu] {
            this->captureMatch();
            this->more();
            this->begin(this->d_intermediate_state);
            return 0;
        }
        * { return 0; }
    */
}

auto zpt::Re2cJSONLexer::lexUnicode() -> int {
    /*!re2c
        .{4} {
            this->captureMatch();
            std::string _out(this->matched());

            std::stringstream ss;
            ss << _out[_out.length() - 4] << _out[_out.length() - 3] << _out[_out.length() - 2]
               << _out[_out.length() - 1];
            int c;
            ss >> std::hex >> c;

            wchar_t w = (wchar_t) c;
            std::string dest("");

            if (w <= 0x7f) {
                dest.insert(dest.begin(), w);
            }
            else if (w <= 0x7ff) {
                dest.insert(dest.end(), 0xc0 | ((w >> 6) & 0x1f));
                dest.insert(dest.end(), 0x80 | (w & 0x3f));
            }
            else if (w <= 0xffff) {
                dest.insert(dest.end(), 0xe0 | ((w >> 12) & 0x0f));
                dest.insert(dest.end(), 0x80 | ((w >> 6) & 0x3f));
                dest.insert(dest.end(), 0x80 | (w & 0x3f));
            }
            else if (w <= 0x10ffff) {
                dest.insert(dest.end(), 0xf0 | ((w >> 18) & 0x07));
                dest.insert(dest.end(), 0x80 | ((w >> 12) & 0x3f));
                dest.insert(dest.end(), 0x80 | ((w >> 6) & 0x3f));
                dest.insert(dest.end(), 0x80 | (w & 0x3f));
            }
            else {
                dest.insert(dest.end(), '?');
            }

            _out.assign(_out.substr(0, _out.length() - 4));
            _out.append(dest);
            this->setMatched(_out);
            this->more();

            this->begin(this->d_intermediate_state);
            return 0;
        }
        * { return 0; }
    */
}
