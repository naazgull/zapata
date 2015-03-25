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

#include <zapata/exceptions/AssertionException.h>

#include <sstream>
#include <zapata/text/convert.h>

zapata::AssertionException::AssertionException(string _in, int _http_code, int _code, string _desc, int _line, string _file) : __what(_in), __http_code(_http_code), __code(_code), __description(_desc), __line(_line), __file(_file){
	zapata::replace(this->__what, "\"", "");
	zapata::replace(this->__description, "\"", "");
	this->__description.insert(0, "'");
	this->__description.insert(this->__description.length(), "' failed on file ");
	this->__description.insert(this->__description.length(), this->__file);
	this->__description.insert(this->__description.length(),  ", line ");
	zapata::tostr(this->__description, this->__line);
}

zapata::AssertionException::~AssertionException() throw() {
}

const char* zapata::AssertionException::what() {
	return this->__what.data();
}

const char* zapata::AssertionException::description() {
	return this->__description.data();
}

int zapata::AssertionException::code() {
	return this->__code;
}

int zapata::AssertionException::status() {
	return this->__http_code;
}
