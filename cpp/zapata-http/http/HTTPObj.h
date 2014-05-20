#pragma once

#define DEBUG_JSON

#include <zapata/core.h>
#include <ostream>
#include <vector>

using namespace std;
using namespace __gnu_cxx;

#ifndef CRLF
#define CRLF "\r\n"
#endif

namespace zapata {

	extern string nil_header;
	extern const char* method_names[];
	extern const char* status_names[];

	enum HTTPType {
		HTTPRequest,
		HTTPReply
	};

	enum HTTPMethod {
		HTTPGet = 0,
		HTTPPut = 1,
		HTTPPost = 2,
		HTTPDelete = 3,
		HTTPHead = 4,
		HTTPTrace = 5,
		HTTPOptions = 6,
		HTTPPatch = 7,
		HTTPConnect = 8
	};

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

	void fromstr(string& _in, HTTPMethod* _out);

	class HTTPElement {
		public:
			HTTPElement();
			virtual ~HTTPElement();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out, short _flags = 0, string _tabs = "");

			virtual string& get(size_t _idx) = 0;
			virtual string& get(const char* _idx) = 0;

			virtual string& header(const char* _idx) = 0;

			operator string();

			HTTPElement& operator<<(const char* _in);
			HTTPElement& operator<<(bool _in);
			HTTPElement& operator<<(int _in);
			HTTPElement& operator<<(long _in);
			HTTPElement& operator<<(long long _in);
			HTTPElement& operator<<(double _in);
			HTTPElement& operator<<(string _in);
			HTTPElement& operator<<(ObjectOp _in);

			HTTPElement& operator>>(long long _in);
			HTTPElement& operator>>(const char* _in);
			HTTPElement& operator>>(string _in);
			HTTPElement& operator>>(ObjectOp _in);

			friend ostream& operator<<(ostream& _out, HTTPElement& _in) {
				_in.stringify(_out, _in.__flags, "");
				return _out;
			}

		protected:
			short __flags;

			virtual void put(int _in) = 0;
			virtual void put(long _in) = 0;
			virtual void put(long long _in) = 0;
			virtual void put(double _in) = 0;
			virtual void put(bool _in) = 0;
			virtual void put(string _in) = 0;
			virtual void put(ObjectOp _in);

			virtual void unset(long long _in) = 0;
			virtual void unset(string _in) = 0;
			virtual void unset(ObjectOp _in);

	};

	class HTTPReqRef: public str_map<string*>, public HTTPElement {
		private:
			string* __name;
			string __url;
			string __query;
			string __body;
			HTTPMethod __method;
			str_map<string*> __params;

		public:
			HTTPReqRef();
			virtual ~HTTPReqRef();

			HTTPMethod method();
			void method(HTTPMethod);
			string& url();
			void url(string);
			string& body();
			void body(string);
			string& query();
			void query(string);
			string& host();
			string& header(const char* _idx);
			string& param(const char* _idx);
			str_map<string*>& params();

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out, short _flags = 0, string _tabs = "");

			virtual string& get(size_t _idx);
			virtual string& get(const char* _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);

			virtual void unset(long long _in);
			virtual void unset(string _in);


	};

	class HTTPRepRef: public str_map<string*>, public HTTPElement {
		private:
			string* __name;
			string __body;
			HTTPStatus __status;

		public:
			HTTPRepRef();
			virtual ~HTTPRepRef();

			HTTPStatus status();
			void status(HTTPStatus);
			string& body();
			void body(string);
			string& header(const char* _idx);

			virtual void stringify(string& _out, short _flags = 0, string _tabs = "");
			virtual void stringify(ostream& _out, short _flags = 0, string _tabs = "");

			virtual string& get(size_t _idx);
			virtual string& get(const char* _idx);

		protected:
			virtual void put(int _in);
			virtual void put(long _in);
			virtual void put(long long _in);
			virtual void put(double _in);
			virtual void put(bool _in);
			virtual void put(string _in);

			virtual void unset(long long _in);
			virtual void unset(string _in);

	};

	typedef smart_ptr<HTTPReqRef> HTTPReq;
	typedef smart_ptr<HTTPRepRef> HTTPRep;
}
