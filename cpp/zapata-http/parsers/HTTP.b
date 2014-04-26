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
	SPACE URL
	{
		d_scanner.url();
	}
	params SPACE VERSION headers rest
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
	 headers rest
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

rest :
	BODY
	{
		d_scanner.body();
	}
|

;