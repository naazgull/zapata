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

%left STRING DOUBLE_DOT SLASH AT QMARK EQ E FUNCTION_PARAM CARDINAL DOT DOT_DOT

%%

exp :
	scheme object params anchor
|
	object params anchor
;

scheme :
    STRING
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            std::string _scheme{d_scanner.matched()};
            if (!d_scanner.d_part_is_placeholder) {
                auto _idx = _scheme.find("+");
                if (_idx != std::string::npos) {
                    (*d_scanner) << "scheme_options" << zpt::split(_scheme.substr(_idx + 1), "+", true);
                    _scheme.assign(_scheme.substr(0, _idx));
                }
            }
            (*d_scanner) << "scheme" << _scheme;
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
    DOUBLE_DOT
;

object :
    server path
|
    path
;

server :
    SLASH SLASH user STRING
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner) << "domain" << d_scanner.matched();
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    } port
|
    SLASH SLASH STRING
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner) << "domain" << d_scanner.matched();
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    } port
;

user :
    STRING
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner) << "user" << zpt::json{ "name", d_scanner.matched() };
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
    AT
;

port :

|
    DOUBLE_DOT STRING
    {
        int _port{0};
        std::istringstream _iss;
        _iss.str(d_scanner.matched());
        _iss >> _port;
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner) << "port" << _port;
        }
        else {
            (*d_scanner) << _port;
        }
    }
;

path :

|
    SLASH
|
    SLASH STRING
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            if ((*d_scanner)["path"] == zpt::undefined) {
                (*d_scanner) << "path" << zpt::json::array();
                (*d_scanner) << "is_relative" << false;
            }
            (*d_scanner)["path"] << d_scanner.matched();
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
    path
|
    DOT
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            if ((*d_scanner)["path"] == zpt::undefined) {
                (*d_scanner) << "path" << zpt::json::array();
                (*d_scanner) << "is_relative" << true;
            }
            (*d_scanner)["path"] << ".";
        }
        else {
            (*d_scanner) << ".";
        }
    }
    path
|
    DOT_DOT
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            if ((*d_scanner)["path"] == zpt::undefined) {
                (*d_scanner) << "path" << zpt::json::array();
                (*d_scanner) << "is_relative" << true;
            }
            (*d_scanner)["path"] << "..";
        }
        else {
            (*d_scanner) << "..";
        }
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
        if ((*d_scanner)->type() == zpt::JSObject) {
            if ((*d_scanner)["params"] == zpt::undefined) {
                (*d_scanner) << "params" << zpt::json::object();
            }
            (*d_scanner) << "__aux" << d_scanner.matched();
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
    EQ paramvalue
|
	paramslist E STRING
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner) << "__aux" << d_scanner.matched();
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
    EQ paramvalue
;

paramvalue :

    {
        auto __name = static_cast<std::string>((*d_scanner)["__aux"]);
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner)["params"] << __name << zpt::undefined;
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
|
    STRING
    {
        auto __name = static_cast<std::string>((*d_scanner)["__aux"]);
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner)["params"] << __name << d_scanner.matched();
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
    function_parameters
;

function_parameters :

|
    FUNCTION_PARAM
    {
        auto __name = static_cast<std::string>((*d_scanner)["__aux"]);
        if ((*d_scanner)->type() == zpt::JSObject) {
            auto __temp = (*d_scanner)["params"][__name];
            if (__temp->type() != zpt::JSObject) {
                (*d_scanner)["params"] << __name << zpt::json{ "name", __temp, "args", zpt::json::array() };
            }
            (*d_scanner)["params"][__name]["args"] << d_scanner.matched();
        }
        else {
            zpt::json __temp = (*d_scanner)[(*d_scanner)->size() - 1];
            if (__temp->type() != zpt::JSObject) {
                (*d_scanner)->array()->pop((*d_scanner)->size() - 1);
                (*d_scanner) << zpt::json{ "name", __temp, "args", zpt::json::array() };
            }
            (*d_scanner)[(*d_scanner)->size() - 1]["args"] << d_scanner.matched();
        }
    }
    function_parameters
;

anchor :

|
	CARDINAL STRING
    {
        if ((*d_scanner)->type() == zpt::JSObject) {
            (*d_scanner) << "anchor" << d_scanner.matched();
        }
        else {
            (*d_scanner) << d_scanner.matched();
        }
    }
;

