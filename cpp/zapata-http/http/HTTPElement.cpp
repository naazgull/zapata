#include <http/HTTPObj.h>

#include <iostream>
#include <exceptions/CastException.h>

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

	const char* status_names[] = { NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	                    "100 Continue ",
	                    "101 Switching Protocols ",
	                    "102 Processing ",
	                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	                    "200 OK ",
	                    "201 Created ",
	                    "202 Accepted ",
	                    "203 Non-Authoritative Information ",
	                    "204 No Content ",
	                    "205 Reset Content ",
	                    "206 Partial Content ",
	                    "207 Multi-Status ",
	                    "208 Already Reported ",
	                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	                    "226 IM Used ",
	                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	                    "300 Multiple Choices ",
	                    "301 Moved Permanently ",
	                    "302 Found ",
	                    "303 See Other ",
	                    "304 Not Modified ",
	                    "305 Use Proxy ",
	                    "306 (Unused) ",
	                    "307 Temporary Redirect ",
	                    "308 Permanent Redirect ",
	                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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
	                    NULL, NULL, NULL, NULL,
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
	                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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
		*_out = zapata::HTTPGet;
	}
	if (_in == string("POST")) {
		*_out = zapata::HTTPGet;
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

zapata::HTTPElement::HTTPElement() {
	this->__flags = 0;
}

zapata::HTTPElement::~HTTPElement() {
}

void zapata::HTTPElement::put(ObjectOp _in) {
	this->__flags |= _in;
}

void zapata::HTTPElement::stringify(ostream& _out, short _flags, string _tabs) {
	_out << _tabs << "null" << flush;
}

void zapata::HTTPElement::stringify(string& _out, short _flags, string _tabs) {
	ostringstream _ret;
	this->stringify(_ret, _flags, _tabs);
	_ret << flush;
	_out.insert(_out.length(), _ret.str());
	_ret.clear();
}

zapata::HTTPElement::operator string() {
	string _out;
	this->stringify(_out);
	return _out;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(const char* _in) {
	this->put(string(_in));
	return *this;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(string _in) {
	this->put(_in);
	return *this;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(bool _in) {
	this->put(_in);
	return *this;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(int _in) {
	this->put(_in);
	return *this;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(long _in) {
	this->put(_in);
	return *this;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(long long _in) {
	this->put(_in);
	return *this;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(double _in) {
	this->put(_in);
	return *this;
}

zapata::HTTPElement& zapata::HTTPElement::operator<<(zapata::ObjectOp _in) {
	this->put(_in);
	return *this;
}
