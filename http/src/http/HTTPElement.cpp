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

#include <zapata/http/HTTPObj.h>

#include <iostream>
#include <zapata/exceptions/CastException.h>

namespace zpt {
	string nil_header = "";

	const char* method_names[] = {
		"GET",
		"PUT",
		"POST",
		"DELETE",
		"HEAD",
		"OPTIONS",
		"PATCH",
		"REPLY",
		"M-SEARCH",
		"NOTIFY",
		"TRACE",
		"CONNECT"
	};

}

void zpt::init(HTTPReq& _req) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	string _url(_req->url());
	if (_url != "") {
		size_t _b = _url.find("://") + 3;
		size_t _e = _url.find("/", _b);
		string _domain(_url.substr(_b, _e - _b));
		string _path(_url.substr(_e));
		_req->header("Host", _domain);
		_req->url(_path);
	}

	_req->method(zpt::ev::Get);
	_req->header("User-Agent", "zapata rest-ful server");
	_req->header("Cache-Control", "max-age=3600");
	_req->header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
	_req->header("Date", string(_buffer_date));

}

void zpt::init(HTTPRep& _rep) {
	time_t _rawtime = time(nullptr);
	struct tm _ptm;
	char _buffer_date[80];
	localtime_r(&_rawtime, &_ptm);
	strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

	char _buffer_expires[80];
	_ptm.tm_hour += 1;
	strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

	_rep->status(zpt::HTTP404);
	_rep->header("User-Agent", "zapata rest-ful server");
	_rep->header("Cache-Control", "max-age=3600");
	_rep->header("Vary", "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag");
	_rep->header("Date", string(_buffer_date));
	_rep->header("Expires", string(_buffer_expires));

}

zpt::HTTPPtr::HTTPPtr(HTTPObj* _target) : shared_ptr<HTTPObj>(_target) {
}

zpt::HTTPPtr::~HTTPPtr(){
}

zpt::HTTPObj::HTTPObj() {
}

zpt::HTTPObj::~HTTPObj() {
}

string& zpt::HTTPObj::body() {
	return this->__body;
}

void zpt::HTTPObj::body(string _body) {
	this->__body.assign(_body.data());
}

zpt::HeaderMap& zpt::HTTPObj::headers() {
	return this->__headers;
}

string zpt::HTTPObj::header(const char* _idx) {
	string _name(_idx);
	zpt::prettify_header_name(_name);
	auto _found = this->__headers.find(_name);
	if (_found != this->__headers.end()) {
		return _found->second;
	}
	return "";
}

void zpt::HTTPObj::header(const char* _name, const char* _value) {
	string _n(_name);
	zpt::prettify_header_name(_n);
	auto _found = this->__headers.find(_n);
	if (_found != this->__headers.end()) {
		this->__headers.erase(_found);
	}
	this->__headers.insert(pair< string, string> (_n, _value));
}

void zpt::HTTPObj::header(const char* _name, string _value) {
	string _n(_name);
	zpt::prettify_header_name(_n);
	auto _found = this->__headers.find(_n);
	if (_found != this->__headers.end()) {
		this->__headers.erase(_found);
	}
	this->__headers.insert(pair< string, string> (_n, _value));
}

void zpt::HTTPObj::header(string _name, string _value) {
	string _n(_name);
	zpt::prettify_header_name(_n);
	auto _found = this->__headers.find(_n);
	if (_found != this->__headers.end()) {
		this->__headers.erase(_found);
	}
	this->__headers.insert(pair< string, string> (_n, _value));
}

zpt::HTTPObj::operator std::string() {
	return this->to_string();
}

std::string zpt::HTTPObj::to_string() {
	std::string _return;
	this->stringify(_return);
	return _return;
}
