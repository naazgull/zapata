%namespace zpt

%baseclass-preinclude JSONinc.h
%baseclass-header JSONTokenizerbase.h
%class-header JSONTokenizer.h
%implementation-header JSONTokenizerimpl.h
%class-name JSONTokenizer
%parsefun-source JSONTokenizer.cpp

%scanner JSONLexer.h

//%debug
%no-lines

%token STRING BOOLEAN INTEGER DOUBLE NIL
%left LCB RCB
%left LB RB
%left COMMA
%left COLON

%%

exp :
	object
	{
		d_scanner.result(zpt::JSObject);
	}
|
	array
	{
		d_scanner.result(zpt::JSArray);
	}
;

object :
	LCB
	{
		d_scanner.init(zpt::JSObject);
	}
	opt_pairlist RCB
	{
		d_scanner.finish(zpt::JSObject);
	}
;

array :
	LB
	{
		d_scanner.init(zpt::JSArray);
	}
	opt_valuelist RB
	{
		d_scanner.finish(zpt::JSArray);
	}
;

opt_pairlist :

|
	pairlist
	{
	}
;

pairlist :
	STRING
	{
		d_scanner.init(zpt::JSObject, d_scanner.matched());
	}
	COLON value
	{
		d_scanner.add();
	}
|
	pairlist COMMA STRING
	{
		d_scanner.init(zpt::JSObject, d_scanner.matched());
	}
	COLON value
	{
		d_scanner.add();
	}
;

opt_valuelist :

|
	valuelist
	{
	}
;

valuelist :
	value
	{
		d_scanner.add();
	}
|
	valuelist COMMA value
	{
		d_scanner.add();
	}
;

value :
	object
	{
	}
|
	array
	{
	}
|
	STRING
	{
		string _out(d_scanner.matched());
		d_scanner.init(_out);
	}
|
	BOOLEAN
	{
		bool _out;
		string _in(d_scanner.matched());
		zpt::fromstr(_in, &_out);
		d_scanner.init(_out);
	}
|
	INTEGER
	{
		long long _out;
		string _in(d_scanner.matched());
		zpt::fromstr(_in, &_out);
		d_scanner.init(_out);
	}
|
	DOUBLE
	{
		double _out;
		string _in(d_scanner.matched());
		zpt::fromstr(_in, &_out);
		d_scanner.init(_out);
	}
|
	NIL
	{
		d_scanner.init();
	}
;
