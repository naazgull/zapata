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
#include <zapata/http/HTTPParser.h>
#include <zapata/exceptions/CastException.h>
#include <zapata/exceptions/NoHeaderNameException.h>

zpt::HTTPReqT::HTTPReqT() : __method(zpt::ev::Get) {}

zpt::HTTPReqT::~HTTPReqT() {}

zpt::performative zpt::HTTPReqT::method() { return this->__method; }

void zpt::HTTPReqT::method(zpt::performative _method) { this->__method = _method; }

string& zpt::HTTPReqT::url() { return this->__url; }

void zpt::HTTPReqT::url(string _url) { this->__url.assign(_url.data()); }

string& zpt::HTTPReqT::query() { return this->__query; }

void zpt::HTTPReqT::query(string _query) { this->__query.assign(_query.data()); }

zpt::ParameterMap& zpt::HTTPReqT::params() { return this->__params; }

string zpt::HTTPReqT::param(const char* _idx) {
	auto _found = this->__params.find(_idx);
	if (_found != this->__params.end()) {
		return _found->second;
	}
	return "";
}

void zpt::HTTPReqT::param(const char* _name, const char* _value) {
	this->__params.insert(pair<string, string>(_name, _value));
}

void zpt::HTTPReqT::param(const char* _name, string _value) {
	this->__params.insert(pair<string, string>(_name, _value));
}

void zpt::HTTPReqT::param(string _name, string _value) { this->__params.insert(pair<string, string>(_name, _value)); }

void zpt::HTTPReqT::stringify(ostream& _out) {
	string _ret;
	this->stringify(_ret);
	_out << _ret << flush;
}

void zpt::HTTPReqT::stringify(string& _out) {
	_out.insert(_out.length(), zpt::method_names[this->__method]);
	_out.insert(_out.length(), " ");
	_out.insert(_out.length(), this->__url);
	if (this->__params.size() != 0) {
		_out.insert(_out.length(), "?");
		bool _first = true;
		for (auto i : this->__params) {
			if (!_first) {
				_out.insert(_out.length(), "&");
			}
			_first = false;
			string _n(i.first);
			zpt::url::encode(_n);
			string _v(i.second);
			zpt::url::encode(_v);
			_out.insert(_out.length(), _n);
			_out.insert(_out.length(), "=");
			_out.insert(_out.length(), _v);
		}
	}
	_out.insert(_out.length(), " HTTP/1.1");
	_out.insert(_out.length(), CRLF);

	for (auto h : this->__headers) {
		_out.insert(_out.length(), h.first);
		_out.insert(_out.length(), ": ");
		_out.insert(_out.length(), h.second);
		_out.insert(_out.length(), CRLF);
	}
	_out.insert(_out.length(), CRLF);
	_out.insert(_out.length(), this->__body);
}

zpt::HTTPReq::HTTPReq() : shared_ptr<HTTPReqT>(new HTTPReqT()) {}

zpt::HTTPReq::HTTPReq(HTTPReqT* _target) : shared_ptr<HTTPReqT>(_target) {}

zpt::HTTPReq::~HTTPReq() {}

zpt::HTTPReq::operator std::string() { return (*this)->to_string(); }

void zpt::HTTPReq::parse(istream& _in) {
	zpt::HTTPParser _p;
	_p.switchRoots(*this);
	_p.switchStreams(_in);
	_p.parse();
}
