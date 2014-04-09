#include <parsers/JSONTokenizerLexer.h>

zapata::JSONTokenizerLexer::JSONTokenizerLexer(std::istream &_in, std::ostream &_out, zapata::JSONObj* _rootobj, zapata::JSONArr* _rootarr) :
	zapata::JSONLexer(_in, _out), __root_obj(_rootobj), __root_arr(_rootarr), __root_type(zapata::JSObject), __value(NULL), __parent(NULL) {
}

zapata::JSONTokenizerLexer::~JSONTokenizerLexer() {
}

void zapata::JSONTokenizerLexer::switchRoots(JSONObj* _rootobj, JSONArr* _rootarr) {
	this->__root_obj = _rootobj;
	this->__root_arr = _rootarr;
}

void zapata::JSONTokenizerLexer::result(zapata::JSONType _in) {
	this->__root_type = _in;
	switch (_in) {
		case zapata::JSObject : {
			this->__root_obj->set((JSONObjRef*) this->__parent);
			break;
		}
		case zapata::JSArray : {
			this->__root_arr->set((JSONArrRef*) this->__parent);
			break;
		}
		default: {
		}
	}
	this->__parent = NULL;
	this->__value = NULL;
}

void zapata::JSONTokenizerLexer::finish(zapata::JSONType _in) {
	this->__value = this->__parent;
	if (this->__context.size() != 0) {
		this->__parent = this->__context.back();
		this->__context.pop_back();
	}
}

void zapata::JSONTokenizerLexer::init(zapata::JSONType _in_type, const string _in_str) {
	(*this->__parent) << _in_str;
}

void zapata::JSONTokenizerLexer::init(zapata::JSONType _in_type) {
	if (this->__parent != NULL) {
		this->__context.push_back(this->__parent);
		this->__parent = NULL;
	}
	switch (_in_type) {
		case zapata::JSObject : {
			this->__parent = new JSONObjRef();
			break;
		}
		case zapata::JSArray : {
			this->__parent = new JSONArrRef();
			break;
		}
		default : {
		}
	}
}

void zapata::JSONTokenizerLexer::init(bool _in) {
	this->__value = new JSONBoolRef(_in);
}

void zapata::JSONTokenizerLexer::init(long long _in) {
	this->__value = new JSONIntRef(_in);
}

void zapata::JSONTokenizerLexer::init(double _in) {
	this->__value = new JSONDblRef(_in);
}

void zapata::JSONTokenizerLexer::init(string _in) {
	this->__value = new JSONStrRef(_in);
}

void zapata::JSONTokenizerLexer::add() {
	switch (this->__value->type()) {
		case zapata::JSObject : {
			zapata::JSONObj _obj((JSONObjRef*) this->__value);
			(*this->__parent) << _obj;
			break;
		}
		case zapata::JSArray : {
			zapata::JSONArr _arr((JSONArrRef*) this->__value);
			(*this->__parent) << _arr;
			break;
		}
		case zapata::JSBoolean : {
			(*this->__parent) << (bool) *this->__value;
			delete this->__value;
			break;
		}
		case zapata::JSInteger : {
			(*this->__parent) << (long long) *this->__value;
			delete this->__value;
			break;
		}
		case zapata::JSDouble : {
			(*this->__parent) << (double) *this->__value;
			delete this->__value;
			break;
		}
		case zapata::JSString : {
			(*this->__parent) << (string) *this->__value;
			delete this->__value;
			break;
		}
		case zapata::JSNil : {
			(*this->__parent) << zapata::undefined;
			delete this->__value;
			break;
		}
		default: {
		}
	}
	this->__value = NULL;
}

