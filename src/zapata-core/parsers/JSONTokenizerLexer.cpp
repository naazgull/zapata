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

#include <zapata/parsers/JSONTokenizerLexer.h>

zapata::JSONTokenizerLexer::JSONTokenizerLexer(std::istream &_in, std::ostream &_out) :
	zapata::JSONLexer(_in, _out) {
}

zapata::JSONTokenizerLexer::~JSONTokenizerLexer() {
}

void zapata::JSONTokenizerLexer::switchRoots(JSONPtr& _root) {
	this->__root = this->__parent = _root.get();
}

void zapata::JSONTokenizerLexer::result(zapata::JSONType _in) {
	try {			
		this->__root_type = _in;
	}
	catch (zapata::AssertionException& _e) {
		cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << endl << flush;
		throw _e;
	}
}

void zapata::JSONTokenizerLexer::finish(zapata::JSONType _in) {
	try {			
		//cout << "- finishing object: " << (* this->__parent) <<  " > " << this->__parent->type() << endl << flush;
		this->__parent = this->__parent->parent();
	}
	catch (zapata::AssertionException& _e) {
		cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << endl << flush;
		throw _e;
	}
}

void zapata::JSONTokenizerLexer::init(zapata::JSONType _in_type, const string _in_str) {
	try {			
		//cout << "- starting field: " << _in_str << endl << flush;
		(* this->__parent) << _in_str;
	}
	catch (zapata::AssertionException& _e) {
		cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << endl << flush;
		throw _e;
	}
}

void zapata::JSONTokenizerLexer::init(zapata::JSONType _in_type) {
	try {		
		//cout << "- putting this object on hold: " << (* this->__parent) <<  " > " << this->__parent->type() << endl << flush;
		switch (_in_type) {
			case zapata::JSObject : {
				JSONObj _obj;
				JSONElementT* _ptr = new JSONElementT(_obj);
				_ptr->parent(this->__parent);
				(* this->__parent) << _ptr;
				this->__parent = _ptr;
				//cout << "- starting object: " << (* this->__parent) <<  " > " << this->__parent->type() << " with parent " << this->__parent->parent() << endl << flush;
				break;
			}
			case zapata::JSArray : {
				JSONArr _arr;
				JSONElementT* _ptr = new JSONElementT(_arr);
				_ptr->parent(this->__parent);
				(* this->__parent) << _ptr;
				this->__parent = _ptr;
				//cout << "- starting object: " << (* this->__parent) <<  " > " << this->__parent->type() << " with parent " << this->__parent->parent() << endl << flush;
				break;
			}
			default : {
			}
		}
	}
	catch (zapata::AssertionException& _e) {
		this->__parent->type(_in_type);
	}
}

void zapata::JSONTokenizerLexer::init(bool _in) {
	try {			
		//cout << "- adding value: " << _in << endl << flush;
		JSONElementT* _ptr = new JSONElementT(_in);
		_ptr->parent(this->__parent);
		(* this->__parent) << _ptr;
	}
	catch (zapata::AssertionException& _e) {
		cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << endl << flush;
		throw _e;
	}
}

void zapata::JSONTokenizerLexer::init(long long _in) {
	try {			
		//cout << "- adding value: " << _in << endl << flush;
		JSONElementT* _ptr = new JSONElementT(_in);
		_ptr->parent(this->__parent);
		(* this->__parent) <<  _ptr;
	}
	catch (zapata::AssertionException& _e) {
		cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << endl << flush;
		throw _e;
	}
}

void zapata::JSONTokenizerLexer::init(double _in) {
	try {			
		//cout << "- adding value: " << _in << endl << flush;
		JSONElementT* _ptr = new JSONElementT(_in);
		_ptr->parent(this->__parent);
		(* this->__parent) <<  _ptr;
	}
	catch (zapata::AssertionException& _e) {
		cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << endl << flush;
		throw _e;
	}
}

void zapata::JSONTokenizerLexer::init(string _in) {
	try {			
		//cout << "- adding value: " << _in << endl << flush;
		JSONElementT* _ptr = new JSONElementT(_in);
		_ptr->parent(this->__parent);
		(* this->__parent) <<  _ptr;
	}
	catch (zapata::AssertionException& _e) {
		cout << __FILE__ << ":" << __LINE__ << " " << _e.description() << endl << flush;
		throw _e;
	}
}

void zapata::JSONTokenizerLexer::add() {
}

