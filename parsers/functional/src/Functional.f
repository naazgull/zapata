%baseclass-header = "FunctionalLexerbase.h"
%class-header = "FunctionalLexer.h"
%implementation-header = "FunctionalLexerimpl.h"
%class-name = "FunctionalLexer"
%lex-source = "FunctionalLexer.cpp"

%namespace = "zpt"

//%debug
%no-lines

%x quoted number
%%

[0-9] {
    more();
    begin(StartCondition_::number);
}
([^()\", 0-9\n\r\f\t]+) {
    return zpt::functional::lex::VARIABLE;
}
"(" {
    return zpt::functional::lex::LPAREN;
}
")" {
    return zpt::functional::lex::RPAREN;
}
"," {
    return zpt::functional::lex::COMMA;
}
"\"" {
    begin(StartCondition_::quoted);
}
[ \n\r\f\t] {
}

<quoted> {
    "\"" {
        std::string _content = matched();
        setMatched(_content.substr(0, _content.length() - 1));
        begin(StartCondition_::INITIAL);
        return zpt::functional::lex::STRING;
    }
    ([^\"]+) {
        more();
    }
}

<number> {
    ([0-9.]*) {
        begin(StartCondition_::INITIAL);
        return zpt::functional::lex::NUMBER;
    }
}
