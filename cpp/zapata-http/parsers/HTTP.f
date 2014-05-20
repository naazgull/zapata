/*
    enum Tokens__
    {
        METHOD = 257,
        VERSION = 258,
        URL = 259,
        STATUS = 260,
        CR_LF = 261,
        COLON = 262,
        STRING = 263,
        SPACE = 264,
        BODY = 265,
        QMARK = 266,
        EQ = 267,
        E = 268,
    };
*/

%baseclass-header = "HTTPLexerbase.h"
%class-header = "HTTPLexer.h"
%implementation-header = "HTTPLexerimpl.h"
%class-name = "HTTPLexer"
%lex-source = "HTTPLexer.cpp"

%namespace = "zapata"

//%debug
%no-lines

%x request reply headers headerval crlf plain_body statustext contentlengthval params
%%

[\f\t\n\r ]+                  // skip white space
<<EOF>>
"GET" {
	begin(StartCondition__::request);
	return 257;
}
"PUT" {
	begin(StartCondition__::request);
	return 257;
}
"POST" {
	begin(StartCondition__::request);
	return 257;
}
"DELETE" {
	begin(StartCondition__::request);
	return 257;
}
"HEAD" {
	begin(StartCondition__::request);
	return 257;
}
"TRACE" {
	begin(StartCondition__::request);
	return 257;
}
"OPTIONS" {
	begin(StartCondition__::request);
	return 257;
}
"PATCH" {
	begin(StartCondition__::request);
	return 257;
}
"CONNECT" {
	begin(StartCondition__::request);
	return 257;
}
"HTTP/1.1" {
	begin(StartCondition__::reply);
	return 258;
}

<request>{
	"HTTP/1.1" {
		return 258;
	}
	"\r\n"   {
		begin(StartCondition__::headers);
		return 261;
	}
	[\n]   {
		begin(StartCondition__::headers);
		return 261;
	}
	[?]   {
		begin(StartCondition__::params);
		return 266;
	}
	([^?\r\n ]+) {
		return 259;
	}
	[ ] {
		return 264;
	}
}

<reply>{
	[0-9]{3} {
		return 260;
	}
	"\r\n" {
		begin(StartCondition__::headers);
		return 261;
	}
	[\n] {
		begin(StartCondition__::headers);
		return 261;
	}
	[^\r\n ] {
		more();
		begin(StartCondition__::statustext);
	}
	[ ] {
		return 264;
	}
}

<params> {
	"=" {
		return 267;
	}
	"&"   {
		return 268;
	}
	[ ] {
		begin(StartCondition__::request);
		return 264;
	}
	([^=& ]+) {
		return 263;
	}
}

<headers> {
	":" {
		begin(StartCondition__::headerval);
		return 262;
	}
	"\r\n"   {
		char _c = get__();
		if (_c == '\n' || _c == '\r') {
			if (d_content_length == 0) {
				leave(-1);
			}
			else {
				get__();
				begin(StartCondition__::plain_body);
			}
		}
		else {
			push(_c);
		}
		return 261;
	}
	"\n"  {
		char _c = get__();
		if (_c == '\n' || _c == '\r') {
			if (d_content_length == 0) {
				leave(-1);
			}
			else {
				begin(StartCondition__::plain_body);
			}
		}
		else {
			push(_c);
		}
		return 261;
	}
	([^:\n\r]+) {
		if (matched() == string("Content-Length")) {
			begin(StartCondition__::contentlengthval);
		}
		return 263;
	}
}

<headerval>{
	([^\n\r]+) {
		begin(StartCondition__::headers);
		return 263;
	}
}

<contentlengthval>{
	":" {
		return 262;
	}
	([^:\n\r]+) {
		std::string _s(matched());
		zapata::fromstr(_s, &d_content_length);
		begin(StartCondition__::headers);
		return 263;
	}
}

<crlf>{
	"\r\n" {
	}
	"\n" {
	}
	[^\r\n] {
		more();
		begin(StartCondition__::headers);
	}
}

<statustext>{
	"\r\n" {
		begin(StartCondition__::headers);
		return 261;
	}
	"\n" {
		begin(StartCondition__::headers);
		return 261;
	}
	([^\r\n]+) {
		return 263;
	}
}

<plain_body>{
	[[:alnum:][:alpha:][:blank:][:cntrl:][:digit:][:graph:][:lower:][:print:][:punct:][:space:][:upper:][:xdigit:]] {
		more();
		if (matched().length() == d_content_length - 1) {
			string _out(matched());
			_out.push_back(get__());
			setMatched(_out);
			leave(-1);
		}
	}
}
