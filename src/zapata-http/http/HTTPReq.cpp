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

zapata::HTTPReqRef::HTTPReqRef() : __name(NULL), __method(zapata::HTTPGet) {
}

zapata::HTTPReqRef::~HTTPReqRef() {
}

zapata::HTTPMethod zapata::HTTPReqRef::method() {
	return this->__method;
}

void zapata::HTTPReqRef::method(zapata::HTTPMethod _method) {
	this->__method = _method;
}

string& zapata::HTTPReqRef::url() {
	return this->__url;
}

void zapata::HTTPReqRef::url(string _url) {
	this->__url.assign(_url.data());
}

string& zapata::HTTPReqRef::body() {
	return this->__body;
}

void zapata::HTTPReqRef::body(string _body) {
	this->__body.assign(_body.data());
}

string& zapata::HTTPReqRef::query() {
	return this->__query;
}

void zapata::HTTPReqRef::query(string _query) {
	this->__query.assign(_query.data());
}

void zapata::HTTPReqRef::unset(string _in) {
	HTTPReqRef::iterator _i;
	if ((this->__flags & zapata::params) == zapata::params) {
		if ((_i = this->__params.find(_in)) != this->__params.end()) {
			return this->__params.erase(_i);
		}
	}
	else {
		if ((_i = this->find(_in)) != this->end()) {
			return this->erase(_i);
		}
	}
}

void zapata::HTTPReqRef::unset(long long _in) {
}

void zapata::HTTPReqRef::put(int _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		if ((this->__flags & zapata::params) == zapata::params) {
			this->__params.insert(string(this->__name->data()), _s);
		}
		else {
			this->insert(string(this->__name->data()), _s);
		}
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPReqRef::put(long _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		if ((this->__flags & zapata::params) == zapata::params) {
			this->__params.insert(string(this->__name->data()), _s);
		}
		else {
			this->insert(string(this->__name->data()), _s);
		}
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPReqRef::put(long long _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		if ((this->__flags & zapata::params) == zapata::params) {
			this->__params.insert(string(this->__name->data()), _s);
		}
		else {
			this->insert(string(this->__name->data()), _s);
		}
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPReqRef::put(double _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		size_t dot = this->__name->find(".");
		if (dot != string::npos) {
			this->__name->erase(dot, 1);
			this->__name->insert(dot, "_");
		}
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		if ((this->__flags & zapata::params) == zapata::params) {
			this->__params.insert(string(this->__name->data()), _s);
		}
		else {
			this->insert(string(this->__name->data()), _s);
		}
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPReqRef::put(bool _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		if ((this->__flags & zapata::params) == zapata::params) {
			this->__params.insert(string(this->__name->data()), _s);
		}
		else {
			this->insert(string(this->__name->data()), _s);
		}
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPReqRef::put(string _in) {
	if (this->__name == NULL) {
		this->__name = new string(_in);
	}
	else {
		string* _s = new string(_in);
		if ((this->__flags & zapata::params) == zapata::params) {
			this->__params.insert(string(this->__name->data()), _s);
		}
		else {
			this->insert(string(this->__name->data()), _s);
		}
		delete this->__name;
		this->__name = NULL;
	}
}

string& zapata::HTTPReqRef::get(size_t _idx) {
	if ((this->__flags & zapata::params) == zapata::params) {
		if(_idx < this->__params.size()) {
			return *this->__params.at(_idx);
		}
	}
	else {
		if(_idx < this->size()) {
			return *this->at(_idx);
		}
	}
	return zapata::nil_header;
}

string& zapata::HTTPReqRef::get(const char* _idx) {
	HTTPReqRef::iterator i;
	if ((this->__flags & zapata::params) == zapata::params) {
		if ((i = this->__params.find(_idx)) != this->__params.end()) {
			return *((*i)->second);
		}
	}
	else {
		if ((i = this->find(_idx)) != this->end()) {
			return *((*i)->second);
		}
	}
	return zapata::nil_header;
}

string& zapata::HTTPReqRef::header(const char* _idx) {
	return this->get(_idx);
}

string& zapata::HTTPReqRef::param(const char* _idx) {
	HTTPReqRef::iterator i;
	if ((i = this->__params.find(_idx)) != this->__params.end()) {
		return *((*i)->second);
	}
	return zapata::nil_header;
}

zapata::str_map<string*>& zapata::HTTPReqRef::params() {
	return this->__params;
}

void zapata::HTTPReqRef::stringify(ostream& _out, short _flags, string _tabs) {
	string _ret;
	this->stringify(_ret, _flags, _tabs);
	_out << _ret << flush;
}

void zapata::HTTPReqRef::stringify(string& _out, short _flags, string _tabs) {
	_out.insert(_out.length(), zapata::method_names[this->__method]);
	_out.insert(_out.length(),  " ");
	_out.insert(_out.length(), this->__url);
	if (this->__params.size() != 0) {
		_out.insert(_out.length(), "?");
		for (HTTPReqRef::iterator i = this->__params.begin(); i != this->__params.end(); i++) {
			if (i != this->__params.begin()) {
				_out.insert(_out.length(), "&");
			}
			_out.insert(_out.length(), (*i)->first);
			_out.insert(_out.length(), "=");
			_out.insert(_out.length(), *(*i)->second);
		}
	}
	_out.insert(_out.length(), " HTTP/1.1");
	_out.insert(_out.length(),  CRLF);
	for (HTTPReqRef::iterator i = this->begin(); i != this->end(); i++) {
		_out.insert(_out.length(), (*i)->first);
		_out.insert(_out.length(), ": ");
		_out.insert(_out.length(), *(*i)->second);
		_out.insert(_out.length(), CRLF);
	}
	_out.insert(_out.length(), CRLF);
	_out.insert(_out.length(), this->__body);
}
