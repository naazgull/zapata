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

zapata::HTTPRep::HTTPRep() : __status(zapata::HTTP100) {
	JSONObj _headers;
	(* this) << "headers" << _headers;
}

zapata::HTTPRep::~HTTPRep() {
}

string& zapata::HTTPRep::body() {
	return this->__body;
}

void zapata::HTTPRep::body(string _body) {
	this->__body.assign(_body);
}

zapata::HTTPStatus zapata::HTTPRep::status() {
	return this->__status;
}

void zapata::HTTPRep::status(zapata::HTTPStatus _in) {
	this->__status = _in;
}

string zapata::HTTPRep::header(const char* _idx) {
	return (string) (* this)["headers"][_idx];
}

void zapata::HTTPRep::header(const char* _name, const char* _value) {
	(* this)["headers"] << _name << _value;
}

void zapata::HTTPRep::header(const char* _name, string _value) {
	(* this)["headers"] << _name << _value;
}

void zapata::HTTPRep::stringify(ostream& _out) {
	string _ret;
	this->stringify(_ret);
	_out << _ret << flush;
}

void zapata::HTTPRep::stringify(string& _out) {
	_out.insert(_out.length(), "HTTP/1.1 "),
	_out.insert(_out.length(),  zapata::status_names[this->__status]);
	_out.insert(_out.length(), CRLF);
	JSONObj _headers = (JSONObj&) (* this)["headers"];
	for (auto i : *_headers) {
		_out.insert(_out.length(), i.first);
		_out.insert(_out.length(), ": ");
		_out.insert(_out.length(), i.second);
		_out.insert(_out.length(), CRLF);
	}
	_out.insert(_out.length(), CRLF);
	_out.insert(_out.length(), this->__body);
}
