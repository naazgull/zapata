/*
    enum Tokens__
    {
        STRING = 257,
        BOOLEAN = 258,
        INTEGER = 259,
        DOUBLE = 260,
        NIL = 261,
        LAMBDA = 262,
        REGEX = 263,
        LCB = 264,
        RCB = 265,
        LB = 266,
        RB = 267,
        COMMA = 268,
        COLON = 269,
    };
*/

%baseclass-header = "JSONLexerbase.h"
%class-header = "JSONLexer.h"
%implementation-header = "JSONLexerimpl.h"
%class-name = "JSONLexer"
%lex-source = "JSONLexer.cpp"

%namespace = "zpt"

//%debug
%no-lines

%x string string_single escaped unicode number regexp

%%

[\n\r\f \t]+                  // skip white space
"true" return 258;
"false" return 258;
"null" return 261;
"undefined" return 261;
lambda\(([^\)]+)\) return 262;
\{ return 264;
\} return 265;
\[ return 266;
\] return 267;
\, return 268;
\: return 269;
[\-0-9] {
	more();
	begin(StartCondition_::number);
}
\" {
	begin(StartCondition_::string);
}
\' {
	begin(StartCondition_::string_single);
}
\/ {
	begin(StartCondition_::regexp);
}
<number>{
	[0-9\.e\+]* {
		begin(StartCondition_::INITIAL);
		if (matched().find(".") != std::string::npos || matched().find("e+") != std::string::npos) {
			return 260;
		}
		else {
			return 259;
		}
	}
}
<string>{
	\" {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		begin(StartCondition_::INITIAL);
		return 257;
	}
	\\   {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		more();
        d_intermediate_state = StartCondition_::string;
		begin(StartCondition_::escaped);
	}
	[^\\"] {
		more();
	}
}
<string_single>{
	\' {

		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		begin(StartCondition_::INITIAL);
		return 257;
	}
	\\   {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		more();
        d_intermediate_state = StartCondition_::string_single;
		begin(StartCondition_::escaped);
	}
	[^\\'] {
		more();
	}
}
<regexp>{
	\/ {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		begin(StartCondition_::INITIAL);
		return 263;
	}
	\\   {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		more();
        d_intermediate_state = StartCondition_::regexp;
		begin(StartCondition_::escaped);
	}
	[^\\/] {
		more();
	}
}
<escaped> {
	n {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		_out.append("\n");
		setMatched(_out);
		more();
		begin(d_intermediate_state);
	}
	t {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		_out.append("\t");
		setMatched(_out);
		more();
		begin(d_intermediate_state);
	}
	r {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		_out.append("\r");
		setMatched(_out);
		more();
		begin(d_intermediate_state);
	}
	f {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		_out.append("\f");
		setMatched(_out);
		more();
		begin(d_intermediate_state);
	}
	u {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		setMatched(_out);
		more();
		begin(StartCondition_::unicode);
	}
	\\ {
		std::string _out(matched());
		_out.erase(_out.length() - 1, 1);
		_out.append("\\");
		setMatched(_out);
		more();
		begin(d_intermediate_state);
	}
	[^\\ntrfu] {
		more();
		begin(d_intermediate_state);
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
		_out.append(dest);
		setMatched(_out);
		more();

		begin(d_intermediate_state);
	}
}
