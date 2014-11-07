/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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

#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/exceptions/CastException.h>

namespace zapata {
	string nil_header = "";

	const char* method_names[] = {
	                    "GET",
	                    "PUT",
	                    "POST",
	                    "DELETE",
	                    "HEAD",
	                    "TRACE",
	                    "OPTIONS",
	                    "PATCH",
	                    "CONNECT"
	};

	const char* status_names[] = { nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                    "100 Continue ",
	                    "101 Switching Protocols ",
	                    "102 Processing ",
	                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                    "200 OK ",
	                    "201 Created ",
	                    "202 Accepted ",
	                    "203 Non-Authoritative Information ",
	                    "204 No Content ",
	                    "205 Reset Content ",
	                    "206 Partial Content ",
	                    "207 Multi-Status ",
	                    "208 Already Reported ",
	                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                    "226 IM Used ",
	                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                    "300 Multiple Choices ",
	                    "301 Moved Permanently ",
	                    "302 Found ",
	                    "303 See Other ",
	                    "304 Not Modified ",
	                    "305 Use Proxy ",
	                    "306 (Unused) ",
	                    "307 Temporary Redirect ",
	                    "308 Permanent Redirect ",
	                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                    "400 Bad Request ",
	                    "401 Unauthorized ",
	                    "402 Payment Required ",
	                    "403 Forbidden ",
	                    "404 Not Found ",
	                    "405 Method Not Allowed ",
	                    "406 Not Acceptable ",
	                    "407 Proxy Authentication Required ",
	                    "408 Request Timeout ",
	                    "409 Conflict ",
	                    "410 Gone ",
	                    "411 Length Required ",
	                    "412 Precondition Failed ",
	                    "413 Payload Too Large ",
	                    "414 URI Too Long ",
	                    "415 Unsupported Media Type ",
	                    "416 Requested Range Not Satisfiable ",
	                    "417 Expectation Failed ",
	                    nullptr, nullptr, nullptr, nullptr,
	                    "422 Unprocessable Entity ",
	                    "423 Locked ",
	                    "424 Failed Dependency ",
	                    "425 Unassigned ",
	                    "426 Upgrade Required ",
	                    "427 Unassigned ",
	                    "428 Precondition Required ",
	                    "429 Too Many Requests ",
	                    "430 Unassigned ",
	                    "431 Request Header Fields Too Large ",
	                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                    "500 Internal Server Error ",
	                    "501 Not Implemented ",
	                    "502 Bad Gateway ",
	                    "503 Service Unavailable ",
	                    "504 Gateway Timeout ",
	                    "505 HTTP Version Not Supported ",
	                    "506 Variant Also Negotiates (Experimental) ",
	                    "507 Insufficient Storage ",
	                    "508 Loop Detected ",
	                    "509 Unassigned ",
	                    "510 Not Extended ",
	                    "511 Network Authentication Required ",
	};
}

void zapata::fromstr(string& _in, HTTPMethod* _out) {
	if (_in == string("GET")) {
		*_out = zapata::HTTPGet;
	}
	if (_in == string("PUT")) {
		*_out = zapata::HTTPPut;
	}
	if (_in == string("POST")) {
		*_out = zapata::HTTPPost;
	}
	if (_in == string("DELETE")) {
		*_out = zapata::HTTPDelete;
	}
	if (_in == string("HEAD")) {
		*_out = zapata::HTTPHead;
	}
	if (_in == string("TRACE")) {
		*_out = zapata::HTTPTrace;
	}
	if (_in == string("OPTIONS")) {
		*_out = zapata::HTTPOptions;
	}
	if (_in == string("PATCH")) {
		*_out = zapata::HTTPPatch;
	}
	if (_in == string("CONNECT")) {
		*_out = zapata::HTTPConnect;
	}
}

void zapata::init(HTTPReq& _req) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	string _url(_req.url());
	if (_url != "") {
		size_t _b = _url.find("://") + 3;
		size_t _e = _url.find("/", _b);
		string _domain(_url.substr(_b, _e - _b));
		string _path(_url.substr(_e));
		_req["headers"] << "Host" << _domain;
		_req.url(_path);
	}

	_req.method(zapata::HTTPGet);
	_req["headers"] <<
		"Connection" << "close" <<
		"User-Agent" << "zapata rest-ful server" <<
		"Cache-Control" << "max-age=3600" <<
		"Vary" << "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag" <<
		"Date" << string(_buffer_date);

}

void zapata::init(HTTPRep& _rep) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	_rep.status(zapata::HTTP404);
	_rep["headers"] <<
		"Connection" << "close" <<
		"Server" << "zapata rest-ful server" <<
		"Cache-Control" << "max-age=3600" <<
		"Vary" << "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag" <<
		"Date" << string(_buffer_date) <<
		"Expires" << string(_buffer_expires);

}

extern "C" int zapata_http() {
	return 1;
}