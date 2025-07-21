
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

%x request reply headers headerval crlf plain_body chunked_body statustext contentlengthval transferencodingval trailerval
%%

//<<EOF>>
[\n\r\f\t ]
"GET" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"PUT" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"POST" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"DELETE" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"HEAD" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"TRACE" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"OPTIONS" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"PATCH" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"CONNECT" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"M-SEARCH" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"NOTIFY" {
	begin(StartCondition_::request);
	return zpt::http::lex::METHOD;
}
"HTTP/1.0" {
	begin(StartCondition_::reply);
	return zpt::http::lex::PROTOCOL_VERSION;
}
"HTTP/1.1" {
	begin(StartCondition_::reply);
	return zpt::http::lex::PROTOCOL_VERSION;
}
"UPNP/1.0" {
	begin(StartCondition_::reply);
	return zpt::http::lex::PROTOCOL_VERSION;
}
"UPNP/1.1" {
	begin(StartCondition_::reply);
	return zpt::http::lex::PROTOCOL_VERSION;
}

<request>{
	"HTTP/1.0" {
		return zpt::http::lex::PROTOCOL_VERSION;
	}
	"HTTP/1.1" {
		return zpt::http::lex::PROTOCOL_VERSION;
	}
	"UPNP/1.0" {
		return zpt::http::lex::PROTOCOL_VERSION;
	}
	"UPNP/1.1" {
		return zpt::http::lex::PROTOCOL_VERSION;
	}
	"\r\n"   {
		begin(StartCondition_::headers);
		return zpt::http::lex::CR_LF;
	}
	[\n]   {
		begin(StartCondition_::headers);
		return zpt::http::lex::CR_LF;
	}
	([^\r\n* ]+) {
		return zpt::http::lex::URL;
	}
	[*] {
		return zpt::http::lex::STAR;
	}
	[ ] {
		return zpt::http::lex::SPACE;
	}
}

<reply>{
	[0-9]{3} {
		return zpt::http::lex::STATUS;
	}
	"\r\n" {
		begin(StartCondition_::headers);
		return zpt::http::lex::CR_LF;
	}
	[\n] {
		begin(StartCondition_::headers);
		return zpt::http::lex::CR_LF;
	}
	[^\r\n ] {
		more();
		begin(StartCondition_::statustext);
	}
        [ ] {
		return zpt::http::lex::SPACE;
	}
}

<headers> {
	":" {
		begin(StartCondition_::headerval);
		return zpt::http::lex::COLON;
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
		return zpt::http::lex::CR_LF;
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
		return zpt::http::lex::CR_LF;
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
		return zpt::http::lex::STRING;
	}
}

<headerval>{
	([^\n\r]+) {
		begin(StartCondition_::headers);
		return zpt::http::lex::STRING;
	}
}

<contentlengthval>{
	":" {
		return zpt::http::lex::COLON;
	}
	([^:\n\r]+) {
		std::string _s(matched());
		zpt::fromstr(_s, &d_content_length);
		begin(StartCondition_::headers);
		return zpt::http::lex::STRING;
	}
}

<transferencodingval>{
	":" {
		return zpt::http::lex::COLON;
	}
	([^:\n\r]+) {
		d_chunked_body = (matched() == std::string(" chunked"));
		begin(StartCondition_::headers);
		return zpt::http::lex::STRING;
	}
}

<trailerval>{
	":" {
		return zpt::http::lex::COLON;
	}
	([^:\n\r]+) {
		d_chunked_trailer = matched();
		begin(StartCondition_::headers);
		return zpt::http::lex::STRING;
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
		return zpt::http::lex::CR_LF;
	}
	"\n" {
		begin(StartCondition_::headers);
		return zpt::http::lex::CR_LF;
	}
	([^\r\n]+) {
		return zpt::http::lex::STRING;
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
