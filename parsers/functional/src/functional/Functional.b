%namespace zpt

%baseclass-preinclude Functionalinc.h
%baseclass-header FunctionalTokenizerbase.h
%class-header FunctionalTokenizer.h
%implementation-header FunctionalTokenizerimpl.h
%class-name FunctionalTokenizer
%scanner-class-name FunctionalScanner
%parsefun-source FunctionalTokenizer.cpp

%scanner FunctionalLexer.h

//%debug
%no-lines

%left TOKEN LPAREN RPAREN COMMA

%%

exp :
	TOKEN params
;

params:

|
    LPAREN param_list RPAREN
;

param_list:

|
    exp param_list
|
    COMMA exp param_list
;
