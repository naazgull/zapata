%namespace zapata

%baseclass-preinclude HTTPinc.h
%baseclass-header HTTPTokenizerbase.h
%class-header HTTPTokenizer.h
%implementation-header HTTPTokenizerimpl.h
%class-name HTTPTokenizer
%parsefun-source HTTPTokenizer.cpp

%scanner HTTPLexer.h

//%debug
%no-lines

%left METHOD VERSION
%left URL STATUS
%left CR_LF
%left COLON
%left STRING SPACE
%left BODY
%left QMARK EQ E

%%

exp :
	METHOD
	{
		d_scanner.init(zapata::HTTPRequest);
	}
	SPACE URL params
	{
		d_scanner.url();
	}
	SPACE VERSION headers BODY
	{
		d_scanner.body();
	}
|
	VERSION
	{
		d_scanner.init(zapata::HTTPReply);
	}
	SPACE STATUS
	{
		d_scanner.status();
	}
	SPACE STRING
	{
	}
	 headers BODY
	{
		d_scanner.body();
	}
;

params :

|
	QMARK paramslist
	{
	}
;

paramslist :
	STRING
	{
		d_scanner.name();
	}
	 EQ STRING
	 {
		d_scanner.value();
	 }
|
	paramslist E STRING
	 {
		d_scanner.name();
	}
	 EQ STRING
	 {
		d_scanner.value();
	 }
;

headers :
	CR_LF headerslist CR_LF
;

headerslist :
	STRING
	{
		d_scanner.add();
	}
	COLON STRING
	{
		d_scanner.add();
	}
	CR_LF
|
	headerslist STRING
	{
		d_scanner.add();
	}
	COLON STRING
	{
		d_scanner.add();
	}
	CR_LF
;
