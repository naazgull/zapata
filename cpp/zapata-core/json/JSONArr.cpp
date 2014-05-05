#include <json/JSONObj.h>

#include <iostream>
#include <exceptions/CastException.h>
#include <exceptions/NoAttributeNameException.h>

zapata::JSONArrRef::JSONArrRef() {
}

zapata::JSONArrRef::~JSONArrRef() {
	for (JSONArrRef::iterator i = this->begin(); i != this->end(); i++) {
		delete *i;
	}
}

zapata::JSONType zapata::JSONArrRef::type() {
	return zapata::JSArray;
}

void zapata::JSONArrRef::unset(string _in) {
}

void zapata::JSONArrRef::unset(long long _in) {
	if (_in < (long long) this->size()) {
		this->erase(this->begin() + _in);
	}
}

void zapata::JSONArrRef::put(int _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(long _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(long long _in) {
	JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(double _in) {
	JSONDbl* _sp = new JSONDbl(new JSONDblRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(bool _in) {
	JSONBool* _sp = new JSONBool(new JSONBoolRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(string _in) {
	JSONStr* _sp = new JSONStr(new JSONStrRef(_in));
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(JSONObj& _in) {
	JSONObj* _sp = new JSONObj(_in.get());
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(JSONArr& _in) {
	JSONArr* _sp = new JSONArr(_in.get());
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

void zapata::JSONArrRef::put(JSONBool& _in) {
	this->put((bool) *_in.get());
}

void zapata::JSONArrRef::put(JSONInt& _in) {
	this->put((long long) *_in.get());
}

void zapata::JSONArrRef::put(JSONDbl& _in) {
	this->put((double) *_in.get());
}

void zapata::JSONArrRef::put(JSONStr& _in) {
	this->put((string) *_in.get());
}

void zapata::JSONArrRef::put(JSONNil& _in) {
	JSONNil* _sp = new JSONNil(new JSONNilRef());
	this->push_back((smart_ptr<JSONElement>*) _sp);
}

bool zapata::JSONArrRef::compare(JSONElement& _in) {
	return this->type() == _in.type() && this == &_in;
}

zapata::JSONElement& zapata::JSONArrRef::get(size_t _idx) {
	if(_idx < this->size()) {
		return *this->at(_idx)->get();
	}
	return JSON_NIL;
}

zapata::JSONElement& zapata::JSONArrRef::get(const char* _idx) {
	return *this;
}

int zapata::JSONArrRef::getInt() {
	return 0;
}

long zapata::JSONArrRef::getLong() {
	return 0;
}

long zapata::JSONArrRef::getLongLong() {
	return 0;
}

double zapata::JSONArrRef::getDouble() {
	return 0;
}

bool zapata::JSONArrRef::getBool() {
	return false;
}

string zapata::JSONArrRef::getString() {
	string _ret;
	this->stringify(_ret, this->__flags);
	return _ret;
}

void zapata::JSONArrRef::stringify(ostream& _out, short _flags, string _tabs) {
#ifdef DEBUG_JSON
	_out << "(" << this << ")";
#endif
	_out <<  "[";
	if (_flags & zapata::pretty) {
		_tabs.insert(_tabs.length(), "\t");
	}
	bool first = true;
	for (JSONArrRef::iterator i = this->begin(); i != this->end(); i++) {
		if (!first) {
			_out << ",";
		}
		first = false;
		if (_flags & zapata::pretty) {
			_out << "\n" << _tabs;
		}
		(*i)->get()->stringify(_out, _flags, _tabs);
	}
	if (_flags & zapata::pretty) {
		_out << "\n";
		_tabs .erase(_tabs.length() - 1, 1);
	}
	_out << _tabs << "]" << flush;
}

void zapata::JSONArrRef::stringify(string& _out, short _flags, string _tabs) {
	ostringstream _ret;
	this->stringify(_ret, _flags, _tabs);
	_ret << flush;
	_out.insert(_out.length(), _ret.str());
	_ret.clear();
}

zapata::JSONElement& zapata::JSONArrRef::operator[](size_t _idx) {
	return this->get(_idx);
}
