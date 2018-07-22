/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#define DEBUG_JSON

#include <ostream>
#include <vector>
#include <map>
#include <memory>
#include <zapata/base/assertz.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

#ifndef CRLF
#define CRLF "\r\n"
#endif

namespace zpt {

extern string nil_header;
extern const char* method_names[];
extern const char* status_names[];

enum HTTPType { HTTPRequest, HTTPReply };

enum HTTPStatus {
	HTTP100 = 100,
	HTTP101 = 101,
	HTTP102 = 102,
	HTTP200 = 200,
	HTTP201 = 201,
	HTTP202 = 202,
	HTTP203 = 203,
	HTTP204 = 204,
	HTTP205 = 205,
	HTTP206 = 206,
	HTTP207 = 207,
	HTTP208 = 208,
	HTTP226 = 226,
	HTTP300 = 300,
	HTTP301 = 301,
	HTTP302 = 302,
	HTTP303 = 303,
	HTTP304 = 304,
	HTTP305 = 305,
	HTTP306 = 306,
	HTTP307 = 307,
	HTTP308 = 308,
	HTTP400 = 400,
	HTTP401 = 401,
	HTTP402 = 402,
	HTTP403 = 403,
	HTTP404 = 404,
	HTTP405 = 405,
	HTTP406 = 406,
	HTTP407 = 407,
	HTTP408 = 408,
	HTTP409 = 409,
	HTTP410 = 410,
	HTTP411 = 411,
	HTTP412 = 412,
	HTTP413 = 413,
	HTTP414 = 414,
	HTTP415 = 415,
	HTTP416 = 416,
	HTTP417 = 417,
	HTTP422 = 422,
	HTTP423 = 423,
	HTTP424 = 424,
	HTTP425 = 425,
	HTTP426 = 426,
	HTTP427 = 427,
	HTTP428 = 428,
	HTTP429 = 429,
	HTTP430 = 430,
	HTTP431 = 431,
	HTTP451 = 451,
	HTTP500 = 500,
	HTTP501 = 501,
	HTTP502 = 502,
	HTTP503 = 503,
	HTTP504 = 504,
	HTTP505 = 505,
	HTTP506 = 506,
	HTTP507 = 507,
	HTTP508 = 508,
	HTTP509 = 509,
	HTTP510 = 510,
	HTTP511 = 511
};

typedef map<string, string> HeaderMap;
typedef map<string, string> ParameterMap;

class HTTPObj {
      public:
	HTTPObj();
	virtual ~HTTPObj();

	string& body();
	void body(string);
	HeaderMap& headers();
	string header(const char* _name);
	void header(const char* _name, const char* _value);
	void header(const char* _name, string _value);
	void header(string _name, string _value);

	operator string();

	virtual std::string to_string();
	virtual void stringify(string& _out) = 0;
	virtual void stringify(ostream& _out) = 0;

      protected:
	string __body;
	HeaderMap __headers;
};

class HTTPPtr : public shared_ptr<HTTPObj> {
      public:
	HTTPPtr(HTTPObj* _target);
	virtual ~HTTPPtr();
};

class HTTPReqT : public HTTPObj {
      public:
	HTTPReqT();
	virtual ~HTTPReqT();

	zpt::performative method();
	void method(zpt::performative);
	string& url();
	void url(string);
	string& query();
	void query(string);
	ParameterMap& params();
	string param(const char* _name);
	void param(const char* _name, const char* _value);
	void param(const char* _name, string _value);
	void param(string _name, string _value);

	virtual void stringify(string& _out);
	virtual void stringify(ostream& _out);

      private:
	string __url;
	string __query;
	zpt::performative __method;
	ParameterMap __params;
};

class HTTPRepT : public HTTPObj {
      public:
	HTTPRepT();
	virtual ~HTTPRepT();

	HTTPStatus status();
	void status(HTTPStatus);

	virtual void stringify(string& _out);
	virtual void stringify(ostream& _out);

      private:
	HTTPStatus __status;
};

class HTTPReq : public shared_ptr<HTTPReqT> {
      public:
	HTTPReq();
	HTTPReq(HTTPReqT* _target);
	virtual ~HTTPReq();

	virtual void parse(istream& _in);

	operator string();
	friend ostream& operator<<(ostream& _out, HTTPReq& _in) {
		_in->stringify(_out);
		return _out;
	};
	friend istream& operator>>(istream& _in, HTTPReq& _out) {
		_out.parse(_in);
		return _in;
	};
};

class HTTPRep : public shared_ptr<HTTPRepT> {
      public:
	HTTPRep();
	HTTPRep(HTTPRepT* _target);
	virtual ~HTTPRep();

	virtual void parse(istream& _in);

	operator string();
	friend ostream& operator<<(ostream& _out, HTTPRep& _in) {
		_in->stringify(_out);
		return _out;
	};

	friend istream& operator>>(istream& _in, HTTPRep& _out) {
		_out.parse(_in);
		return _in;
	};
};

namespace http {
typedef zpt::HTTPReq req;
typedef zpt::HTTPRep rep;
}

void init(HTTPReq& _out);
void init(HTTPRep& _out);
}
