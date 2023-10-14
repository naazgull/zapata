%baseclass-header = "URILexerbase.h"
%class-header = "URILexer.h"
%implementation-header = "URILexerimpl.h"
%class-name = "URILexer"
%lex-source = "URILexer.cpp"

%namespace = "zpt"

//%debug
%no-lines

%x scheme server_path server path params placeholder function anchor
%%

([^{:/?#.]+) {
    begin(StartCondition_::scheme);
    d_part_is_placeholder = false;
    return zpt::uri::lex::STRING;
}
"{" {
    d_path_helper.assign("{");
    d_intermediate_state = StartCondition_::scheme;
    begin(StartCondition_::placeholder);
}
"/" {
    begin(StartCondition_::path);
    return zpt::uri::lex::SLASH;
}
"." {
    begin(StartCondition_::path);
    return zpt::uri::lex::DOT;
}
".." {
    begin(StartCondition_::path);
    return zpt::uri::lex::DOT_DOT;
}
"?" {
    begin(StartCondition_::params);
    return zpt::uri::lex::QMARK;
}
"#" {
    begin(StartCondition_::anchor);
    return zpt::uri::lex::CARDINAL;
}
"*" {
    return zpt::uri::lex::STAR;
}

<scheme> {
    ":" {
        return zpt::uri::lex::DOUBLE_DOT;
    }
    "." {
        begin(StartCondition_::path);
        return zpt::uri::lex::DOT;
    }
    ".." {
        begin(StartCondition_::path);
        return zpt::uri::lex::DOT_DOT;
    }
    "/" {
        begin(StartCondition_::server_path);
        return zpt::uri::lex::SLASH;
    }
}

<server_path> {
    [^/] {
        d_path_helper.assign(matched());
        begin(StartCondition_::path);
    }
    "/" {
        begin(StartCondition_::server);
        return zpt::uri::lex::SLASH;
    }
}

<server> {
    ([^:@/{]+) {
        d_server_part.assign(matched());
        d_part_is_placeholder = false;
        return zpt::uri::lex::STRING;
    }
    "/" {
        setMatched(d_server_part);
        begin(StartCondition_::path);
        return zpt::uri::lex::SLASH;
    }
    ":" {
        setMatched(d_server_part);
        return zpt::uri::lex::DOUBLE_DOT;
    }
    "@" {
        setMatched(d_server_part);
        return zpt::uri::lex::AT;
    }
    "{" {
        d_path_helper.assign("{");
        d_intermediate_state = StartCondition_::server;
        begin(StartCondition_::placeholder);
    }
}

<path> {
    "/" {
        d_path_helper.assign("");
        return zpt::uri::lex::SLASH;
    }
    "{" {
        d_path_helper.assign("{");
       d_intermediate_state = StartCondition_::path;
        begin(StartCondition_::placeholder);
    }
    "?" {
        begin(StartCondition_::params);
        return zpt::uri::lex::QMARK;
    }
    "#" {
        begin(StartCondition_::anchor);
        return zpt::uri::lex::CARDINAL;
    }
    ([^{/?#]+) {
        std::string _matched{matched()};
        _matched.insert(0, d_path_helper);
        d_path_helper.assign("");
        setMatched(_matched);
        d_part_is_placeholder = false;
        return zpt::uri::lex::STRING;
    }
}

<params> {
    "=" {
        return zpt::uri::lex::EQ;
    }
    "&"   {
        return zpt::uri::lex::E;
    }
    "#" {
        begin(StartCondition_::anchor);
        return zpt::uri::lex::CARDINAL;
    }
    ([^=&#]+) {
        d_part_is_placeholder = false;
        return zpt::uri::lex::STRING;
    }
}

<placeholder> {
    ([^}]+) {
        d_path_helper += matched();
    }
    "}" {
        d_path_helper += matched();
        setMatched(d_path_helper);
        d_path_helper.assign("");
        begin(d_intermediate_state);
        d_part_is_placeholder = true;
        return zpt::uri::lex::STRING;
    }
}

<anchor> {
    (.+) {
        d_part_is_placeholder = false;
        return zpt::uri::lex::STRING;
    }
}
