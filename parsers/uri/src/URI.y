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
class URITokenizerLexer;
}
}

%{
#include <zapata/exceptions/SyntaxErrorException.h>
#include <zapata/json/json.h>
#include <zapata/uri/URITokenizerLexer.h>

#define YYSTYPE int

int yylex(YYSTYPE* yylval, zpt::URITokenizerLexer* ctx);
void yyerror(zpt::URITokenizerLexer* ctx, char const* msg);
%}

%define api.pure full
%define api.value.type {int}
%param { zpt::URITokenizerLexer* ctx }

%token STAR STRING DOUBLE_DOT SLASH AT QMARK EQ E CARDINAL DOT DOT_DOT

%%

exp :
    scheme object params anchor
|
    object params anchor
|
    STAR
    {
        (**ctx) << "is_wildcard" << true << "is_relative" << false << "is_absolute" << false;
    }
;

scheme :
    STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            std::string _scheme{ctx->matched()};
            if (!ctx->d_part_is_placeholder) {
                auto _idx = _scheme.find("+");
                if (_idx != std::string::npos) {
                    (**ctx) << "scheme_options" << zpt::split(_scheme.substr(_idx + 1), "+", true);
                    _scheme.assign(_scheme.substr(0, _idx));
                }
            }
            (**ctx) << "scheme" << _scheme;
        }
        else {
            (**ctx) << ctx->matched();
        }
    }
    DOUBLE_DOT
;

object :
    server path
|
    path
;

server :
    SLASH SLASH user STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx) << "domain" << ctx->matched();
        }
        else {
            (**ctx) << ctx->matched();
        }
    } port
|
    SLASH SLASH STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx) << "domain" << ctx->matched();
        }
        else {
            (**ctx) << ctx->matched();
        }
    } port
;

user :
    STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx) << "user" << zpt::json{ "name", ctx->matched() };
        }
        else {
            (**ctx) << ctx->matched();
        }
    }
    AT
;

port :

|
    DOUBLE_DOT STRING
    {
        int _port{0};
        std::istringstream _iss;
        _iss.str(ctx->matched());
        _iss >> _port;
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx) << "port" << _port;
        }
        else {
            (**ctx) << _port;
        }
    }
;

path :

|
    SLASH
    {
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx) << "is_relative" << false;
        }
    }
|
    SLASH STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            if (!(**ctx)("path")->ok()) {
                (**ctx) << "path" << zpt::json::array();
                (**ctx) << "raw_path" << "";
                (**ctx) << "is_relative" << false;
            }
            (**ctx)["raw_path"]->string().append("/");
            (**ctx)["raw_path"]->string().append(ctx->matched());
            (**ctx)["path"] << zpt::url::r_decode(ctx->matched());
        }
        else {
            (**ctx) << zpt::url::r_decode(ctx->matched());
        }
    }
    path
|
    DOT
    {
        if ((*ctx)->type() == zpt::JSObject) {
            if (!(**ctx)("path")->ok()) {
                (**ctx) << "path" << zpt::json::array();
                (**ctx) << "raw_path" << "";
                (**ctx) << "is_relative" << true;
            }
            (**ctx)["raw_path"]->string().append("/.");
            (**ctx)["path"] << ".";
        }
        else {
            (**ctx) << ".";
        }
    }
    path
|
    DOT_DOT
    {
        if ((*ctx)->type() == zpt::JSObject) {
            if (!(**ctx)("path")->ok()) {
                (**ctx) << "path" << zpt::json::array();
                (**ctx) << "raw_path" << "";
                (**ctx) << "is_relative" << true;
            }
            (**ctx)["raw_path"]->string().append("/..");
            (**ctx)["path"] << "..";
        }
        else {
            (**ctx) << "..";
        }
    }
    path
;

params :

|
    QMARK paramslist
;

paramslist :
    STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            if (!(**ctx)("params")->ok()) {
                (**ctx) << "params" << zpt::json::object();
            }
            (**ctx) << "__aux" << ctx->matched();
        }
        else {
            (**ctx) << ctx->matched();
        }
    }
    EQ paramvalue
|
    paramslist E STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx) << "__aux" << ctx->matched();
        }
        else {
            (**ctx) << ctx->matched();
        }
    }
    EQ paramvalue
;

paramvalue :

    {
        auto __name = static_cast<std::string>((**ctx)["__aux"]);
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx)["params"] << __name << zpt::undefined;
        }
        else {
            (**ctx) << ctx->matched();
        }
    }
|
    STRING
    {
        auto __name = static_cast<std::string>((**ctx)["__aux"]);
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx)["params"] << __name << zpt::url::r_decode(ctx->matched());
        }
        else {
            (**ctx) << zpt::url::r_decode(ctx->matched());
        }
    }
;

anchor :

|
    CARDINAL STRING
    {
        if ((*ctx)->type() == zpt::JSObject) {
            (**ctx) << "anchor" << zpt::url::r_decode(ctx->matched());
        }
        else {
            (**ctx) << zpt::url::r_decode(ctx->matched());
        }
    }
;

%%

int yylex(YYSTYPE*, zpt::URITokenizerLexer* ctx) {
    return ctx->lex();
}

void yyerror(zpt::URITokenizerLexer* ctx, char const* msg) {
    throw zpt::SyntaxErrorException(std::string("URI: Syntax error in line ") +
                                     std::to_string(ctx->lineNr()) + std::string{msg});
}
