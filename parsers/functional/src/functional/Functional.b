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

%left STRING NUMBER VARIABLE LPAREN RPAREN COMMA

%%

exp:
	token params
;

params:

|
    LPAREN param_list RPAREN
;

param_list:

|
    exp
    {
        d_scanner.add_param();
    }
    param_list
|
    COMMA exp
    {
        d_scanner.add_param();
    }
    param_list
;

token:
    STRING
    {
        d_scanner.set_string();
    }
|
    NUMBER
    {
        d_scanner.set_number();
    }
|
    VARIABLE
    {
        d_scanner.set_variable();
    }
;