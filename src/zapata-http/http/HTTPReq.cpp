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
#include <zapata/exceptions/NoHeaderNameException.h>

zapata::HTTPReq::HTTPReq() : __method(zapata::HTTPGet) {
	JSONObj _headers;
	JSONObj _params;
	(* this) << "headers" << _headers;
	(* this) << "params" << _params;
}

zapata::HTTPReq::~HTTPReq() {
}

zapata::HTTPMethod zapata::HTTPReq::method() {
	return this->__method;
}

void zapata::HTTPReq::method(zapata::HTTPMethod _method) {
	this->__method = _method;
}

string& zapata::HTTPReq::url() {
	return this->__url;
}

void zapata::HTTPReq::url(string _url) {
	this->__url.assign(_url.data());
}

string& zapata::HTTPReq::body() {
	return this->__body;
}

void zapata::HTTPReq::body(string _body) {
	this->__body.assign(_body.data());
}

string& zapata::HTTPReq::query() {
	return this->__query;
}

void zapata::HTTPReq::query(string _query) {
	this->__query.assign(_query.data());
}

string zapata::HTTPReq::header(const char* _idx) {
	return (string) (* this)["headers"][_idx];
}

void zapata::HTTPReq::header(const char* _name, const char* _value) {
	(* this)["headers"] << _name << _value;
}

void zapata::HTTPReq::header(const char* _name, string _value) {
	(* this)["headers"] << _name << _value;
}

string zapata::HTTPReq::param(const char* _idx) {
	return (string) (* this)["params"][_idx];
}

void zapata::HTTPReq::param(const char* _name, const char* _value) {
	(* this)["params"] << _name << _value;
}

void zapata::HTTPReq::param(const char* _name, string _value) {
	(* this)["params"] << _name << _value;
}

void zapata::HTTPReq::stringify(ostream& _out) {
	string _ret;
	this->stringify(_ret);
	_out << _ret << flush;
}

void zapata::HTTPReq::stringify(string& _out) {
	_out.insert(_out.length(), zapata::method_names[this->__method]);
	_out.insert(_out.length(),  " ");
	_out.insert(_out.length(), this->__url);
	if ((* this)["params"]->obj()->size() != 0) {
		_out.insert(_out.length(), "?");
		JSONObj _params = (JSONObj&) (* this)["params"];
		bool _first = true;
		for (auto i : *_params) {
			if (!_first) {
				_out.insert(_out.length(), "&");
			}
			_first = false;
			_out.insert(_out.length(), i.first);
			_out.insert(_out.length(), "=");
			_out.insert(_out.length(), i.second);
		}
	}
	_out.insert(_out.length(), " HTTP/1.1");
	_out.insert(_out.length(),  CRLF);
	JSONObj _headers = (JSONObj&) (* this)["headers"];
	bool _first = true;
	for (auto h : *_headers) {
		_out.insert(_out.length(), h.first);
		_out.insert(_out.length(), ": ");
		_out.insert(_out.length(), h.second);
		_out.insert(_out.length(), CRLF);
	}
	_out.insert(_out.length(), CRLF);
	_out.insert(_out.length(), this->__body);
}
