/*
    enum Tokens__
    {
        METHOD = 257,
        HTTP_VERSION = 258,
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

size_t	d_content_length;
bool	d_chunked_body;
string	d_chunked;

%baseclass-header = "HTTPLexerbase.h"
%class-header = "HTTPLexer.h"
%implementation-header = "HTTPLexerimpl.h"
%class-name = "HTTPLexer"
%lex-source = "HTTPLexer.cpp"

%namespace = "zpt"

//%debug
%no-lines

%x request reply headers headerval crlf plain_body chunked_body statustext contentlengthval transferencodingval trailerval params
%%

//<<EOF>>
[\n\r\f\t ]
"GET" {
	begin(StartCondition_::request);
	return 257;
}
"PUT" {
	begin(StartCondition_::request);
	return 257;
}
"POST" {
	begin(StartCondition_::request);
	return 257;
}
"DELETE" {
	begin(StartCondition_::request);
	return 257;
}
"HEAD" {
	begin(StartCondition_::request);
	return 257;
}
"TRACE" {
	begin(StartCondition_::request);
	return 257;
}
"OPTIONS" {
	begin(StartCondition_::request);
	return 257;
}
"PATCH" {
	begin(StartCondition_::request);
	return 257;
}
"CONNECT" {
	begin(StartCondition_::request);
	return 257;
}
"M-SEARCH" {
	begin(StartCondition_::request);
	return 257;
}
"NOTIFY" {
	begin(StartCondition_::request);
	return 257;
}
"HTTP/1.0" {
	begin(StartCondition_::reply);
	return 258;
}
"HTTP/1.1" {
	begin(StartCondition_::reply);
	return 258;
}

<request>{
	"HTTP/1.0" {
		return 258;
	}
	"HTTP/1.1" {
		return 258;
	}
	"\r\n"   {
		begin(StartCondition_::headers);
		return 261;
	}
	[\n]   {
		begin(StartCondition_::headers);
		return 261;
	}
	[?]   {
		begin(StartCondition_::params);
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
		begin(StartCondition_::headers);
		return 261;
	}
	[\n] {
		begin(StartCondition_::headers);
		return 261;
	}
	[^\r\n ] {
		more();
		begin(StartCondition_::statustext);
	}
        [ ] {
		return 264;
	}
}

<params> {
	"=" {
		return 267;
	}
	"&" {
		return 268;
	}
	[ ] {
		begin(StartCondition_::request);
		return 264;
	}
	([^=& ]+) {
		return 263;
    }
}

<headers> {
	":" {
		begin(StartCondition_::headerval);
		return 262;
	}
	"\r\n"   {
		char _c = get_();
		if (_c == '\n' || _c == '\r') {
			if (d_chunked_body) {
				get_();
				d_chunked_length = -1;
				begin(StartCondition_::chunked_body);
			}
			else if (d_content_length == 0) {
				leave(-1);
			}
			else {
				get_();
				begin(StartCondition_::plain_body);
			}
		}
		else {
			push(_c);
		}
		return 261;
	}
	"\n"  {
		char _c = get_();
		if (_c == '\n' || _c == '\r') {
			if (d_chunked_body) {
				d_chunked_length = -1;
				begin(StartCondition_::chunked_body);
			}
			else if (d_content_length == 0) {
				leave(-1);
			}
			else {
				begin(StartCondition_::plain_body);
			}
		}
		else {
			push(_c);
		}
		return 261;
	}
	([^:\n\r]+) {
		std::string _m(matched());
		std::transform(_m.begin(), _m.end(), _m.begin(), ::tolower);
		if (_m == std::string("content-length")) {
			begin(StartCondition_::contentlengthval);
		}
		else if (_m == std::string("transfer-encoding")) {
			begin(StartCondition_::transferencodingval);
		}
		else if (_m == std::string("trailer")) {
			begin(StartCondition_::trailerval);
		}
		return 263;
	}
}

<headerval>{
	([^\n\r]+) {
		begin(StartCondition_::headers);
		return 263;
	}
}

<contentlengthval>{
	":" {
		return 262;
	}
	([^:\n\r]+) {
		std::string _s(matched());
		zpt::fromstr(_s, &d_content_length);
		begin(StartCondition_::headers);
		return 263;
	}
}

<transferencodingval>{
	":" {
		return 262;
	}
	([^:\n\r]+) {
		d_chunked_body = (matched() == std::string(" chunked"));
		begin(StartCondition_::headers);
		return 263;
	}
}

<trailerval>{
	":" {
		return 262;
	}
	([^:\n\r]+) {
		d_chunked_trailer = matched();
		begin(StartCondition_::headers);
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
		begin(StartCondition_::headers);
	}
}

<statustext>{
	"\r\n" {
		begin(StartCondition_::headers);
		return 261;
	}
	"\n" {
		begin(StartCondition_::headers);
		return 261;
	}
	([^\r\n]+) {
		return 263;
	}
}

<plain_body>{
	.|\n {
		more();
		if (matched().length() == d_content_length - 1) {
			std::string _out(matched());
			_out.push_back(get_());
			setMatched(_out);
			leave(-1);
		}
	}
}

<chunked_body>{
	"\r\n" {
		if (d_chunked_length == -1) {
			std::istringstream _is;
			_is.str(matched());
			_is >> std::hex >> d_chunked_length;
			setMatched("");
		}
		else if (d_chunked_length == -2) {
			d_chunked_length = -1;
			setMatched(d_chunked);
			get_();
			get_();
			leave(-1);
		}

		if (d_chunked_length == 0) {
			if (d_chunked_trailer.length() == 0) {
				setMatched(d_chunked);
				get_();
				get_();
				leave(-1);
			}
			else {
				d_chunked_length = -2;
				more();
			}
		}
		else if (matched().length() - 2 == (size_t) d_chunked_length) {
			d_chunked.insert(d_chunked.length(), matched());
			zpt::trim(d_chunked);
			setMatched("");
			d_chunked_length = -1;
		}
		else {
			more();
		}
	}
	"\r" {
		more();
	}
	"\n" {
		more();
	}
	[^\r\n] {
		more();
	}
}
