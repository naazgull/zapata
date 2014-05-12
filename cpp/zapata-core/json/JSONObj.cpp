#include <json/JSONObj.h>

#include <iostream>
#include <exceptions/CastException.h>
#include <exceptions/NoAttributeNameException.h>

zapata::JSONObjRef::JSONObjRef() {
	this->__name = NULL;
}

zapata::JSONObjRef::~JSONObjRef() {
	if (this->__name != NULL) {
		delete this->__name;
		this->__name = NULL;
	}
}

zapata::JSONType zapata::JSONObjRef::type() {
	return zapata::JSObject;
}

void zapata::JSONObjRef::unset(string _in) {
	JSONObjRef::iterator _i;
	if ((_i = this->find(_in)) != this->end()) {
		return this->erase(_i);
	}
}

void zapata::JSONObjRef::unset(long long _in) {
}

void zapata::JSONObjRef::put(int _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(long _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(long long _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(unsigned int _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		JSONInt* _sp = new JSONInt(new JSONIntRef(_in));
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(double _in) {
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
		JSONDbl* _sp = new JSONDbl(new JSONDblRef(_in));
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(bool _in) {
	if (this->__name == NULL) {
		this->__name = new string();
		zapata::tostr(*this->__name, _in);
		this->__name->insert(0, "_");
	}
	else {
		JSONBool* _sp = new JSONBool(new JSONBoolRef(_in));
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(string _in) {
	if (this->__name == NULL) {
		this->__name = new string(_in);
	}
	else {
		JSONStr* _sp = new JSONStr(new JSONStrRef(_in));
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(JSONObj& _in) {
	if (this->__name == NULL) {
		throw zapata::NoAttributeNameException("can't create attribute name from JS Object");
	}
	else {
		JSONObj* _sp = new JSONObj(_in.get());
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(JSONArr& _in) {
	if (this->__name == NULL) {
		throw zapata::NoAttributeNameException("can't create attribute name from JS Array");
	}
	else {
		JSONArr* _sp = new JSONArr(_in.get());
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

void zapata::JSONObjRef::put(JSONBool& _in) {
	this->put((bool) *_in.get());
}

void zapata::JSONObjRef::put(JSONInt& _in) {
	this->put((long long) *_in.get());
}

void zapata::JSONObjRef::put(JSONDbl& _in) {
	this->put((double) *_in.get());
}

void zapata::JSONObjRef::put(JSONStr& _in) {
	this->put((string) *_in.get());
}

void zapata::JSONObjRef::put(JSONNil& _in) {
	if (this->__name == NULL) {
		this->__name = new string("null");
	}
	else {
		JSONNil* _sp = new JSONNil(new JSONNilRef());
		this->insert(string(this->__name->data()), (smart_ptr<JSONElement>*) _sp);
		delete this->__name;
		this->__name = NULL;
	}
}

bool zapata::JSONObjRef::compare(JSONElement& _in) {
	return this->type() == _in.type() && this == &_in;
}

zapata::JSONElement& zapata::JSONObjRef::get(size_t _idx) {
	if(_idx < this->size()) {
		return *this->at(_idx)->get();
	}
	return JSON_NIL;
}

zapata::JSONElement& zapata::JSONObjRef::get(const char* _idx) {
	JSONObjRef::iterator i;
	if ((i = this->find(_idx)) != this->end()) {
		return *((*i)->second->get());
	}
	return JSON_NIL;
}

int zapata::JSONObjRef::getInt() {
	return 0;
}

long zapata::JSONObjRef::getLong() {
	return 0;
}

long zapata::JSONObjRef::getLongLong() {
	return 0;
}

unsigned int zapata::JSONObjRef::getUnsignedInt() {
	return 0;
}

double zapata::JSONObjRef::getDouble() {
	return 0;
}

bool zapata::JSONObjRef::getBool() {
	return false;
}

string zapata::JSONObjRef::getString() {
	string _ret;
	this->stringify(_ret, this->__flags);
	return _ret;
}

void zapata::JSONObjRef::stringify(ostream& _out, short _flags, string _tabs) {
#ifdef DEBUG_JSON
	_out << "(" << this << ")";
#endif
	_out << "{";
	if (_flags & zapata::pretty) {
		_tabs.insert(_tabs.length(), "\t");
	}
	bool first = true;
	for (JSONObjRef::iterator i = this->begin(); i != this->end(); i++) {
		if (!first) {
			_out << ",";
		}
		first = false;
		if (_flags & zapata::pretty) {
			_out << "\n";
		}
		_out << _tabs << "\"" << (*i)->first << (_flags & zapata::pretty ? "\" : " : "\":");
		(*i)->second->get()->stringify(_out, _flags, _tabs);
	}
	if (_flags & zapata::pretty) {
		_out << "\n";
		_tabs .erase(_tabs.length() - 1, 1);
	}
	_out << _tabs << "}" << flush;
}

void zapata::JSONObjRef::stringify(string& _out, short _flags, string _tabs) {
	ostringstream _ret;
	this->stringify(_ret, _flags, _tabs);
	_ret << flush;
	_out.insert(_out.length(), _ret.str());
	_ret.clear();
}

zapata::JSONElement& zapata::JSONObjRef::operator[](size_t _idx) {
	return this->get(_idx);
}

zapata::JSONElement& zapata::JSONObjRef::operator[](const char* _idx) {
	return this->get(_idx);
}
