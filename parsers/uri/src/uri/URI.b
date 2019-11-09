%namespace zpt

%baseclass-preinclude URIinc.h
%baseclass-header URITokenizerbase.h
%class-header URITokenizer.h
%implementation-header URITokenizerimpl.h
%class-name URITokenizer
%parsefun-source URITokenizer.cpp

%scanner URILexer.h

%debug
%no-lines

%left SCHEME DOUBLE_DOT SLASH PATH_PART
%left QMARK EQ E STRING

%%

exp :
	scheme
    {
        std::cout << d_scanner.matched() << std::endl << std::flush;
    }
    path
    {
        std::cout << d_scanner.matched() << std::endl << std::flush;
    }
    params
    {
        std::cout << d_scanner.matched() << std::endl << std::flush;
    }
;

scheme :

|
    SCHEME DOUBLE_DOT SLASH
;

path :

|
    SLASH PATH_PART path
;

params :

|
	QMARK paramslist
;

paramslist :
	STRING EQ paramvalue
|
	paramslist E STRING EQ paramvalue
;

paramvalue: | STRING;
