#include <http/HTTPObj.h>

#include <iostream>
#include <exceptions/CastException.h>
#include <exceptions/NoHeaderNameException.h>

zapata::HTTPRepRef::HTTPRepRef() : __name(NULL) {
}

zapata::HTTPRepRef::~HTTPRepRef() {
}

string& zapata::HTTPRepRef::body() {
	return this->__body;
}

void zapata::HTTPRepRef::body(string _body) {
	this->__body.assign(_body.data());
}

zapata::HTTPStatus zapata::HTTPRepRef::status() {
	return this->__status;
}

void zapata::HTTPRepRef::status(zapata::HTTPStatus _in) {
	this->__status = _in;
}

void zapata::HTTPRepRef::put(int _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		this->insert(string(this->__name->data()), _s);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPRepRef::put(long _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		this->insert(string(this->__name->data()), _s);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPRepRef::put(long long _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		this->insert(string(this->__name->data()), _s);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPRepRef::put(double _in) {
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
		this->insert(string(this->__name->data()), _s);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPRepRef::put(bool _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		string* _s = new string();
		zapata::tostr(*_s, _in);
		this->insert(string(this->__name->data()), _s);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::HTTPRepRef::put(string _in) {
	if (this->__name == NULL) {
		this->__name = new string(_in);
	}
	else {
		string* _s = new string(_in);
		this->insert(string(this->__name->data()), _s);
		delete this->__name;
		this->__name = NULL;
	}
}

string& zapata::HTTPRepRef::get(size_t _idx) {
	if(_idx < this->size()) {
		return *this->at(_idx);
	}
	return zapata::nil_header;
}

string& zapata::HTTPRepRef::get(const char* _idx) {
	HTTPRepRef::iterator i;
	if ((i = this->find(_idx)) != this->end()) {
		return *((*i)->second);
	}
	return zapata::nil_header;
}

string& zapata::HTTPRepRef::header(const char* _idx) {
	return this->get(_idx);
}

void zapata::HTTPRepRef::stringify(ostream& _out, short _flags, string _tabs) {
	if (_flags & zapata::pretty) {
	}
	_out << "HTTP/1.1 " << zapata::status_names[this->__status] << CRLF;
	for (HTTPRepRef::iterator i = this->begin(); i != this->end(); i++) {
		_out << (*i)->first << ": " << *(*i)->second << CRLF;
	}
	_out << CRLF;
	_out << this->__body;
	if (_flags & zapata::pretty) {
	}
}

void zapata::HTTPRepRef::stringify(string& _out, short _flags, string _tabs) {
	ostringstream _ret;
	this->stringify(_ret, _flags, _tabs);
	_ret << flush;
	_out.insert(_out.length(), _ret.str());
	_ret.clear();
}
