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

%code requires {
namespace zpt {
class FunctionalTokenizerLexer;
}
}

%{
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/functional/FunctionalTokenizerLexer.h>

#define YYSTYPE int

int yylex(YYSTYPE* yylval, zpt::FunctionalTokenizerLexer* ctx);
void yyerror(zpt::FunctionalTokenizerLexer* ctx, char const* msg);
%}

%define api.pure full
%define api.value.type {int}
%param { zpt::FunctionalTokenizerLexer* ctx }

%token STRING NUMBER VARIABLE LPAREN RPAREN COMMA

%%

exp :
    token params
;

params :
    %empty
|
    LPAREN param_list RPAREN
;

param_list :
    %empty
|
    exp
    {
        ctx->add_param();
    }
    param_list
|
    COMMA exp
    {
        ctx->add_param();
    }
    param_list
;

token :
    STRING
    {
        ctx->set_string();
    }
|
    NUMBER
    {
        ctx->set_number();
    }
|
    VARIABLE
    {
        ctx->set_variable();
    }
;

%%

int yylex(YYSTYPE*, zpt::FunctionalTokenizerLexer* ctx) {
    return ctx->lex();
}

void yyerror(zpt::FunctionalTokenizerLexer* ctx, char const* msg) {
    throw zpt::SyntaxErrorException(std::string("Functional: Syntax error in line ") +
                                     std::to_string(ctx->lineNr()) + std::string{msg});
}
