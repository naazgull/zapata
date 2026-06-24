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
 * JSON value grammar (GNU bison). Ported from the original bisonc++ grammar
 * in JSON.b. Tokens carry no semantic payload - actions read the current
 * match text via ctx->matched(), exactly as the original grammar did via
 * d_scanner.matched(), so JSONTokenizerLexer's methods (init/result/finish/
 * justLeave/add) port unchanged.
 */

%code requires {
namespace zpt {
class JSONTokenizerLexer;
}
}

%{
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/json/JSONTokenizerLexer.h>

#define YYSTYPE int

int yylex(YYSTYPE* yylval, zpt::JSONTokenizerLexer* ctx);
void yyerror(zpt::JSONTokenizerLexer* ctx, char const* msg);
%}

%define api.pure full
%define api.value.type {int}
%param { zpt::JSONTokenizerLexer* ctx }

%token STRING BOOLEAN INTEGER DOUBLE NIL LAMBDA REGEX
%token LCB RCB LB RB COMMA COLON

%%

exp :
    object
    {
        ctx->result(zpt::JSObject);
        ctx->justLeave();
    }
|
    array
    {
        ctx->result(zpt::JSArray);
        ctx->justLeave();
    }
|
    STRING
    {
        ctx->result(zpt::JSString);
        std::string _out(ctx->matched());
        ctx->init(_out);
        ctx->justLeave();
    }
|
    BOOLEAN
    {
        ctx->result(zpt::JSBoolean);
        bool _out;
        std::string _in(ctx->matched());
        zpt::fromstr(_in, &_out);
        ctx->init(_out);
        ctx->justLeave();
    }
|
    INTEGER
    {
        ctx->result(zpt::JSInteger);
        long long _out;
        std::string _in(ctx->matched());
        zpt::fromstr(_in, &_out);
        ctx->init(_out);
        ctx->justLeave();
    }
|
    DOUBLE
    {
        ctx->result(zpt::JSDouble);
        double _out;
        std::string _in(ctx->matched());
        zpt::fromstr(_in, &_out);
        ctx->init(_out);
        ctx->justLeave();
    }
|
    LAMBDA
    {
        ctx->result(zpt::JSLambda);
        std::string _in(ctx->matched());
        zpt::lambda _out(_in);
        ctx->init(_out);
        ctx->justLeave();
    }
|
    REGEX
    {
        ctx->result(zpt::JSRegex);
        std::string _in(ctx->matched());
        zpt::regex _out(_in);
        ctx->init(_out);
        ctx->justLeave();
    }
|
    NIL
    {
        ctx->init();
        ctx->justLeave();
    }
;

object :
    LCB
    {
        ctx->init(zpt::JSObject);
    }
    opt_pairlist RCB
    {
        ctx->finish(zpt::JSObject);
    }
;

array :
    LB
    {
        ctx->init(zpt::JSArray);
    }
    opt_valuelist RB
    {
        ctx->finish(zpt::JSArray);
    }
;

opt_pairlist :
    %empty
|
    pairlist
    {
    }
;

pairlist :
    STRING
    {
        ctx->init(zpt::JSObject, ctx->matched());
    }
    COLON value
    {
        ctx->add();
    }
|
    pairlist COMMA STRING
    {
        ctx->init(zpt::JSObject, ctx->matched());
    }
    COLON value
    {
        ctx->add();
    }
;

opt_valuelist :
    %empty
|
    valuelist
    {
    }
;

valuelist :
    value
    {
        ctx->add();
    }
|
    valuelist COMMA value
    {
        ctx->add();
    }
;

value :
    object
    {
    }
|
    array
    {
    }
|
    STRING
    {
        std::string _out(ctx->matched());
        ctx->init(_out);
    }
|
    BOOLEAN
    {
        bool _out;
        std::string _in(ctx->matched());
        zpt::fromstr(_in, &_out);
        ctx->init(_out);
    }
|
    INTEGER
    {
        long long _out;
        std::string _in(ctx->matched());
        zpt::fromstr(_in, &_out);
        ctx->init(_out);
    }
|
    DOUBLE
    {
        double _out;
        std::string _in(ctx->matched());
        zpt::fromstr(_in, &_out);
        ctx->init(_out);
    }
|
    LAMBDA
    {
        std::string _in(ctx->matched());
        zpt::lambda _out(_in);
        ctx->init(_out);
    }
|
    REGEX
    {
        std::string _in(ctx->matched());
        zpt::regex _out(_in);
        ctx->init(_out);
    }
|
    NIL
    {
        ctx->init();
    }
;

%%

int yylex(YYSTYPE*, zpt::JSONTokenizerLexer* ctx) {
    return ctx->lex();
}

void yyerror(zpt::JSONTokenizerLexer* ctx, char const* msg) {
    throw zpt::SyntaxErrorException(std::string("JSON: Syntax error in line ") +
                                     std::to_string(ctx->lineNr()) + std::string{msg});
}
