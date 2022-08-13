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

%%

exp :
	METHOD
	{
		d_scanner.d_content_length = 0;
		d_scanner.init(0);
	}
	SPACE URL
	{
		d_scanner.url();
	}
	SPACE HTTP_VERSION
    {
		d_scanner.version();
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
|
	HTTP_VERSION
	{
		d_scanner.d_content_length = 0;
		d_scanner.init(1);
		d_scanner.version();
	}
	SPACE STATUS
	{
		d_scanner.status();
	}
	status_description
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

status_description : | SPACE STRING;

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