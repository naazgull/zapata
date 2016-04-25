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
#include <zapata/http/config.h>

#include <zapata/http/HTTPTokenizerLexer.h>

zapata::HTTPTokenizerLexer::HTTPTokenizerLexer(std::istream &_in, std::ostream &_out) :
	zapata::HTTPLexer(_in, _out) {
}

zapata::HTTPTokenizerLexer::~HTTPTokenizerLexer() {
}

void zapata::HTTPTokenizerLexer::switchRoots(HTTPReq& _root) {
	this->__root_req = _root.get();
}

void zapata::HTTPTokenizerLexer::switchRoots(HTTPRep& _root) {
	this->__root_rep = _root.get();
}

void zapata::HTTPTokenizerLexer::justLeave() {
	this->leave(-1);
}

void zapata::HTTPTokenizerLexer::init(zapata::HTTPType _in_type) {
	this->d_chunked_body = false;
	this->d_chunked.clear();
	this->__root_type = _in_type;
	switch (_in_type) {
		case zapata::HTTPRequest : {
			zapata::ev::Performative _m;
			string _ms(this->matched());
			zapata::fromstr(_ms, &_m);
			this->__root_req->method(_m);
			break;
		}
		case zapata::HTTPReply : {
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::body() {
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			this->__root_req->body(this->matched());
			break;
		}
		case zapata::HTTPReply : {
			this->__root_rep->body(this->matched());
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::url() {
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			this->__root_req->url(this->matched());
			break;
		}
		case zapata::HTTPReply : {
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::status() {
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			break;
		}
		case zapata::HTTPReply : {
			int _status = 0;
			string _statusstr(this->matched());
			zapata::fromstr(_statusstr, &_status);
			this->__root_rep->status((zapata::HTTPStatus) _status);
			break;
		}
	}
}

void zapata::HTTPTokenizerLexer::add() {
	string _s(this->matched());
	zapata::trim(_s);
	if (this->__header_name.length() == 0) {
		this->__header_name.assign(_s);
		return;
	}
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			this->__root_req->header(this->__header_name, _s);
			break;
		}
		case zapata::HTTPReply : {
			this->__root_rep->header(this->__header_name, _s);
			break;
		}
	}
	this->__header_name.clear();
}

void zapata::HTTPTokenizerLexer::name() {
	string _s(this->matched());
	zapata::trim(_s);
	this->__param_name.assign(_s);			
}

void zapata::HTTPTokenizerLexer::value() {
	string _s(this->matched());
	zapata::trim(_s);
	switch (this->__root_type) {
		case zapata::HTTPRequest : {
			this->__root_req->param(this->__param_name, _s);
			break;
		}
		case zapata::HTTPReply : {
			break;
		}
	}
	this->__param_name.clear();
}

