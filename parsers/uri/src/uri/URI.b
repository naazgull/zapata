%namespace zpt

%baseclass-preinclude URIinc.h
%baseclass-header URITokenizerbase.h
%class-header URITokenizer.h
%implementation-header URITokenizerimpl.h
%class-name URITokenizer
%scanner-class-name URIScanner
%parsefun-source URITokenizer.cpp

%scanner URILexer.h

//%debug
%no-lines

%left DOUBLE_DOT SLASH AT QMARK EQ E STRING

%%

exp :
    scheme server path params
|
	scheme path params
;

scheme :

|
    STRING
    {
        (*d_scanner) << "scheme" << d_scanner.matched();
    }
    DOUBLE_DOT
;

server :
    SLASH SLASH user STRING
    {
        (*d_scanner) << "domain" << d_scanner.matched();
    }
|
    SLASH SLASH STRING
    {
        (*d_scanner) << "domain" << d_scanner.matched();
    }
;

user :
    STRING
    {
        (*d_scanner) << "user" << zpt::json{ "name", d_scanner.matched() };
    }
    AT
|
    STRING
    {
        (*d_scanner) << "user" <<
              zpt::json{ "name", d_scanner.matched() };
    }
    DOUBLE_DOT STRING
    {
        (*d_scanner)["user"] << "password" << d_scanner.matched();
    }
    AT
;

path :

|
    SLASH
|
    SLASH STRING
    {
        if ((*d_scanner)["path"] == zpt::undefined) {
            (*d_scanner) << "path" << zpt::json::array();
        }
        (*d_scanner)["path"] << d_scanner.matched();
    }
    path
;

params :

|
	QMARK paramslist
;

paramslist :
	STRING
    {
        if ((*d_scanner)["params"] == zpt::undefined) {
            (*d_scanner) << "params" << zpt::json::object();
        }
        (*d_scanner) << "__aux" << d_scanner.matched();
    }
    EQ paramvalue
|
	paramslist E STRING
    {
        (*d_scanner) << "__aux" << d_scanner.matched();
    }
    EQ paramvalue
;

paramvalue :

    {
        (*d_scanner)["params"] << static_cast<std::string>((*d_scanner)["__aux"]) << zpt::undefined;
    }
|
    STRING
    {
        (*d_scanner)["params"] << static_cast<std::string>((*d_scanner)["__aux"]) << d_scanner.matched();
    }
;