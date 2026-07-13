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
 * HTTP request/reply grammar (GNU bison). Ported from the original
 * bisonc++ grammar in HTTP.b. Tokens carry no semantic payload - actions
 * read the current match text via ctx->matched(), exactly as the original
 * grammar did via d_scanner.matched(), so HTTPTokenizerLexer's methods
 * (init/version/url/status/add/body) port unchanged.
 *
 * The duplicated end-of-message body-flush block that appeared identically
 * in both alternatives of the original `exp` rule is factored into
 * HTTPTokenizerLexer::finishMessage().
 */

%code requires {
namespace zpt {
class HTTPTokenizerLexer;
}
}

%{
#include <zapata/http/HTTPTokenizerLexer.h>

#define YYSTYPE int

int yylex(YYSTYPE* yylval, zpt::HTTPTokenizerLexer* ctx);
void yyerror(zpt::HTTPTokenizerLexer* ctx, char const* msg);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable="
%}

%define api.pure full
%define api.value.type {int}
%param { zpt::HTTPTokenizerLexer* ctx }

%token METHOD
%token PROTOCOL_VERSION
%token URL
%token STAR
%token STATUS
%token CR_LF
%token COLON
%token STRING
%token SPACE

%%

exp:
    METHOD
    {
        ctx->d_content_length = 0;
        ctx->init(0);
    }
    SPACE resource
    {
        ctx->url();
    }
    SPACE PROTOCOL_VERSION
    {
        ctx->version();
    }
    headers
    {
        ctx->finishMessage();
    }
|
    PROTOCOL_VERSION
    {
        ctx->d_content_length = 0;
        ctx->init(1);
        ctx->version();
    }
    SPACE STATUS
    {
        ctx->status();
    }
    status_description headers
    {
        ctx->finishMessage();
    }
;

resource:
    URL
|
    STAR
;

status_description: %empty | SPACE STRING ;

headers:
    CR_LF headerslist CR_LF
|
    CR_LF headerslist
;

headerslist:
    STRING
    {
        ctx->add();
    }
    COLON STRING
    {
        ctx->add();
    }
|
    headerslist CR_LF STRING
    {
        ctx->add();
    }
    COLON STRING
    {
        ctx->add();
    }
;

%%

int yylex(YYSTYPE* yylval, zpt::HTTPTokenizerLexer* ctx) {
    (void) yylval;
    return ctx->lex();
}

void yyerror(zpt::HTTPTokenizerLexer* ctx, char const* msg) {
    (void) msg;
    throw zpt::SyntaxErrorException(std::string("HTTP: Syntax error in line ") +
                                    std::to_string(ctx->lineNr()));
}

#pragma GCC diagnostic pop
