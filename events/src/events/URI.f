/*
    enum Tokens__
    {
      SCHEME = 257,
      DOUBLE_DOT = 258,
      SLASH = 259,
      PATH_PART = 260,
      QMARK = 261,
      STRING = 262,
      EQ = 263,
      E = 264
    };
*/

%baseclass-header = "URILexerbase.h"
%class-header = "URILexer.h"
%implementation-header = "URILexerimpl.h"
%class-name = "URILexer"
%lex-source = "URILexer.cpp"

%namespace = "zpt"

//%debug
%no-lines

%x scheme path params placeholder
%%

([^:/]+) {
	begin(StartCondition_::scheme);
	return 257;
}
"/" {
    begin(StartCondition_::path);
    return 259;
}

<scheme>{
    ":" {
        std::string _out(matched());
        _out.erase(_out.length() - 1, 1);
        setMatched(_out);
        return 258;
    }
    "/" {
        begin(StartCondition_::path);
        return 259;
    }
}

<path>{
    "/" {
        return 259;
    }
    "{" {
        begin(StartCondition_::placeholder);
    }
    "?" {
        begin(StartCondition_::params);
        return 261;
    }
    ([^{/?]+) {
        return 260;
    }
}

<params> {
	"=" {
		return 263;
	}
	"&"   {
		return 264;
	}
	([^=&]+) {
		return 262;
    }
}

<placeholder> {
    "}" {
        begin(StartCondition_::path);
    }
}
