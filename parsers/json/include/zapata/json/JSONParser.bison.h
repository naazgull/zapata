/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSERS_JSON_INCLUDE_ZAPATA_JSON_JSONPARSER_BISON_H_INCLUDED
#define YY_YY_PARSERS_JSON_INCLUDE_ZAPATA_JSON_JSONPARSER_BISON_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 31 "parsers/json/src/JSON.y"

namespace zpt {
class JSONTokenizerLexer;
}

#line 55 "parsers/json/include/zapata/json/JSONParser.bison.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype {
    YYEMPTY = -2,
    YYEOF = 0,     /* "end of file"  */
    YYerror = 256, /* error  */
    YYUNDEF = 257, /* "invalid token"  */
    STRING = 258,  /* STRING  */
    BOOLEAN = 259, /* BOOLEAN  */
    INTEGER = 260, /* INTEGER  */
    DOUBLE = 261,  /* DOUBLE  */
    NIL = 262,     /* NIL  */
    LAMBDA = 263,  /* LAMBDA  */
    REGEX = 264,   /* REGEX  */
    LCB = 265,     /* LCB  */
    RCB = 266,     /* RCB  */
    LB = 267,      /* LB  */
    RB = 268,      /* RB  */
    COMMA = 269,   /* COMMA  */
    COLON = 270    /* COLON  */
};
typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif

int yyparse(zpt::JSONTokenizerLexer* ctx);

#endif /* !YY_YY_PARSERS_JSON_INCLUDE_ZAPATA_JSON_JSONPARSER_BISON_H_INCLUDED  */
