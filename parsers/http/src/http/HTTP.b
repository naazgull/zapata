%namespace zpt

%baseclass-preinclude HTTPinc.h
%baseclass-header HTTPTokenizerbase.h
%class-header HTTPTokenizer.h
%implementation-header HTTPTokenizerimpl.h
%class-name HTTPTokenizer
%scanner-class-name HTTPScanner
%parsefun-source HTTPTokenizer.cpp

%scanner HTTPLexer.h

//%debug
%no-lines

%left METHOD HTTP_VERSION
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
		d_scanner.d_content_length = 0;
		d_scanner.init(zpt::http::message_type::request);
	}
	SPACE URL
	{
		d_scanner.url();
	}
	params SPACE HTTP_VERSION headers rest
	{
		if (d_scanner.d_content_length != 0) {
			d_scanner.body();
			d_scanner.d_content_length = 0;
		}
		else if (d_scanner.d_chunked.length() != 0) {
			d_scanner.body();
			d_scanner.d_chunked_length = -1;
			d_scanner.d_chunked.assign("");
		}
	}
|
	HTTP_VERSION
	{
		d_scanner.d_content_length = 0;
		d_scanner.init(zpt::http::message_type::reply);
	}
	SPACE STATUS
	{
		d_scanner.status();
	}
	SPACE STRING
	{
	}
	headers rest
	{
		if (d_scanner.d_content_length != 0) {
			d_scanner.body();
			d_scanner.d_content_length = 0;
		}
		else if (d_scanner.d_chunked.length() != 0) {
			d_scanner.body();
			d_scanner.d_chunked_length = -1;
			d_scanner.d_chunked.assign("");
		}
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
	 EQ paramvalue
	 {
		d_scanner.value();
	 }
|
	paramslist E STRING
	 {
		d_scanner.name();
	}
	 EQ paramvalue
	 {
		d_scanner.value();
	 }
;

paramvalue: | STRING;

headers :
	CR_LF headerslist CR_LF
|
	CR_LF headerslist
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
|
	headerslist CR_LF STRING
	{
		d_scanner.add();
	}
	COLON STRING
	{
		d_scanner.add();
	}
;

rest :
	BODY
	{
		d_scanner.body();
		d_scanner.d_content_length = 0;
		d_scanner.d_chunked_length = -1;
		d_scanner.d_chunked.assign("");
	}
|

;