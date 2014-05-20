#include <db/convert_mongo.h>

void zapata::frommongo(bson::bo& _in, zapata::JSONObj& _out) {
	for (mongo::BSONObjIterator _i = _in.begin(); _i.more();) {
		bson::be _it = _i.next();
		string _key(_it.fieldName());

		switch (_it.type()) {
			case mongo::jstNULL: {
				_out << _key << "null";
				break;
			}
			case mongo::Bool: {
				_out << _key << _it.Bool();

				break;
			}
			case mongo::NumberDouble: {
				_out << _key << _it.Double();
				break;
			}
			case mongo::NumberLong:
				case mongo::NumberInt: {
				_out << _key << _it.Int();
				break;
			}
			case mongo::String: {
				_out << _key << _it.String();
				break;
			}
			case mongo::Object: {
				bson::bo _mobj = _it.Obj();
				zapata::JSONObj _obj;
				zapata::frommongo(_mobj, _obj);
				_out << _key << _obj;
				break;
			}
			case mongo::Array: {
				zapata::JSONArr _arr;
				zapata::frommongo(_it, _arr);
				_out << _key << _arr;
				break;
			}
			default: {
				continue;
			}
		}
	}
}

void zapata::frommongo(bson::be& _in, zapata::JSONArr& _out) {
	vector<bson::be> _obj = _in.Array();
	for (vector<bson::be>::iterator _i = _obj.begin(); _i != _obj.end(); _i++) {
		bson::be _it = *_i;
		switch (_it.type()) {
			case mongo::jstNULL: {
				_out << "null";
				break;
			}
			case mongo::Bool: {
				_out << _it.Bool();

				break;
			}
			case mongo::NumberDouble: {
				_out << _it.Double();
				break;
			}
			case mongo::NumberLong:
				case mongo::NumberInt: {
				_out << _it.Int();
				break;
			}
			case mongo::String: {
				_out << _it.String();
				break;
			}
			case mongo::Object: {
				bson::bo _mobj = _it.Obj();
				zapata::JSONObj _obj;
				zapata::frommongo(_mobj, _obj);
				_out <<  _obj;
				break;
			}
			case mongo::Array: {
				zapata::JSONArr _arr;
				zapata::frommongo(_it, _arr);
				_out << _arr;
				break;
			}
			default: {
				continue;
			}
		}
	}
}

void zapata::tomongo(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _out) {
	for (JSONObjIterator _i = _in->begin(); _i != _in->end(); _i++) {
		string _key = (*_i)->first;
		smart_ptr<JSONElement>* _value = (*_i)->second;

		switch ((*_value)->type()) {
			case zapata::JSObject: {
				mongo::BSONObjBuilder _mobj;
				JSONObj _obj = *((JSONObj*) _value->get());
				zapata::tomongo(_obj, _mobj);
				_out << _key << _mobj.obj();
				break;
			}
			case zapata::JSArray: {
				mongo::BSONArrayBuilder _mobj;
				JSONArr _arr = *((JSONArr*) _value->get());
				zapata::tomongo(_arr, _mobj);
				_out << _key << _mobj.arr();
				break;
			}
			case zapata::JSString: {
				_out << _key << (string) (*(*_value));
				break;
			}
			case zapata::JSBoolean: {
				_out << _key << (bool) (*(*_value));
				break;
			}
			case zapata::JSDouble: {
				_out << _key << (double) (*(*_value));
				break;
			}
			case zapata::JSInteger: {
				_out << _key << (int) (*(*_value));
				break;
			}
			case zapata::JSNil: {
				_out.appendNull(_key);
				break;
			}
			default: {
				continue;
			}
		}
	}
}

void zapata::tomongo(zapata::JSONArr& _in, mongo::BSONArrayBuilder&  _out) {
	for (JSONArrIterator _i = _in->begin(); _i != _in->end(); _i++) {
		smart_ptr<JSONElement>* _value = (*_i);

		switch ((*_value)->type()) {
			case zapata::JSObject: {
				mongo::BSONObjBuilder _mobj;
				JSONObj _obj = *((JSONObj*) _value->get());
				zapata::tomongo(_obj, _mobj);
				_out << _mobj.obj();
				break;
			}
			case zapata::JSArray: {
				mongo::BSONArrayBuilder _mobj;
				JSONArr _arr = *((JSONArr*) _value->get());
				zapata::tomongo(_arr, _mobj);
				_out << _mobj.arr();
				break;
			}
			case zapata::JSString: {
				_out << (string) (*(*_value));
				break;
			}
			case zapata::JSBoolean: {
				_out << (bool) (*(*_value));
				break;
			}
			case zapata::JSDouble: {
				_out << (double) (*(*_value));
				break;
			}
			case zapata::JSInteger: {
				_out << (int) (*(*_value));
				break;
			}
			case zapata::JSNil: {
				_out.appendNull();
				break;
			}
			default: {
				continue;
			}
		}
	}
}
