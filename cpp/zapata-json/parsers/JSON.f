/*
    enum Tokens__
    {
        STRING = 257,
        BOOLEAN = 258,
        INTEGER = 259,
        DOUBLE = 260,
        NIL = 261,
        LCB = 262,
        RCB = 263,
        LB = 264,
        RB = 265,
        COMMA = 266,
        COLON = 267,
    };
*/

%baseclass-header = "JSONLexerbase.h"
%class-header = "JSONLexer.h"
%implementation-header = "JSONLexerimpl.h"
%class-name = "JSONLexer"
%lex-source = "JSONLexer.cpp"

%namespace = "zapata"

//%debug
%no-lines

%x string escaped unicode

%%

[\n\r\f \t]+                  // skip white space
"true" return 258;
"false" return 258;
"null" return 261;
"undefined" return 261;
\{ return 262;
\} return 263;
\[ return 264;
\] return 265;
\: return 267;
\, return 266;
[0-9]+ return 259;
\" {
	begin(StartCondition__::string);
}
<string>{
	\" {

		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);

		// echo();
		// std::cout << std::endl;

		begin(StartCondition__::INITIAL);
		return 257;
	}
	\\   {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		more();
		begin(StartCondition__::escaped);
	}
	[^\\"] {
		more();
	}
}
<escaped> {
	u {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		more();
		begin(StartCondition__::unicode);
	}
	[^u] {
		more();
		begin(StartCondition__::string);
	}
}
<unicode> {
	.{4} {
		std::string _out(matched());

		std::stringstream ss;
		ss << _out[_out.length() - 4] << _out[_out.length() - 3] << _out[_out.length() - 2] << _out[_out.length() - 1];
		int c;
		ss >> std::hex >> c;

		wchar_t w = (wchar_t) c;
		std::string dest("");

		if (w <= 0x7f) {
			dest.insert(dest.begin(), w);
		}
		else if (w <= 0x7ff) {
			dest.insert(dest.end(), 0xc0 | ((w >> 6) & 0x1f));
			dest.insert(dest.end(), 0x80 | (w & 0x3f));
		}
		else if (w <= 0xffff) {
			dest.insert(dest.end(), 0xe0 | ((w >> 12) & 0x0f));
			dest.insert(dest.end(), 0x80 | ((w >> 6) & 0x3f));
			dest.insert(dest.end(), 0x80 | (w & 0x3f));
		}
		else if (w <= 0x10ffff) {
			dest.insert(dest.end(), 0xf0 | ((w >> 18) & 0x07));
			dest.insert(dest.end(), 0x80 | ((w >> 12) & 0x3f));
			dest.insert(dest.end(), 0x80 | ((w >> 6) & 0x3f));
			dest.insert(dest.end(), 0x80 | (w & 0x3f));
		}
		else {
			dest.insert(dest.end(), '?');
		}

		_out.assign(_out.substr(0, _out.length() - 4));
		_out.insert(_out.length(), dest);
		setMatched(_out);
		more();

		begin(StartCondition__::string);
	}
}
