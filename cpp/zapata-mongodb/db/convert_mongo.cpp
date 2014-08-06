/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <db/convert_mongo.h>

#define _VALID_OPS string("$gt^$gte^$lt^$lte^$ne^$type^$exists^")

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

void zapata::tomongoquery(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _queryr, mongo::BSONObjBuilder& _order, size_t& _page_size, size_t& _page_start_index) {
	for (JSONObjIterator _i = _in->begin(); _i != _in->end(); _i++) {
		string _key = (*_i)->first;
		smart_ptr<JSONElement>* _value = (*_i)->second;

		if (_key == "fields" || _key == "embed") {
			continue;
		}

		if (_key == "pageSize") {
			istringstream iss((string) (*_value->get()));
			int i = 0;
			iss >> i;
			if (!iss.eof()) {
				_page_size = 0;
			}
			else {
				_page_size = i;
				if (_page_size < 0) {
					_page_size *= -1;
				}
			}
			continue;
		}

		if (_key == "pageStartIndex") {
			istringstream iss((string) (*_value->get()));
			int i = 0;
			iss >> i;
			if (!iss.eof()) {
				_page_start_index = 0;
			}
			else {
				_page_start_index = i;
				if (_page_start_index < 0) {
					_page_start_index *= -1;
				}
			}
			continue;
		}

		if (_key == "orderBy") {
			istringstream lss(((string) (*_value->get())).data());
			string part;
			while (std::getline(lss, part, ',')) {
				if (part.length() > 0) {
					int dir = 1;

					if (part[0] == '-') {
						dir = -1;
						part.erase(0, 1);
					}
					else if (part[0] == '+') {
						part.erase(0, 1);
					}

					if (part.length() > 0) {
						ostringstream oss;
						oss << part << flush;

						_order.append(oss.str(), dir);
					}
				}
			}

			continue;
		}

		ostringstream oss;
		oss << _key << flush;

		string key = oss.str();
		string value = ((string) (*_value->get()));
		if (value.length() > 3 && value.find('/') != string::npos) {
			int bar_count = 0;

			istringstream lss(value);
			string part;

			string command;
			string expression;
			string options;
			while (std::getline(lss, part, '/')) {
				if (bar_count == 0) {
					command = part;
					++bar_count;
				}
				else if (bar_count == 1) {
					expression.append(part);

					if (expression.length() == 0
					                || expression[expression.length() - 1] != '\\') {
						++bar_count;
					}
					else {
						if (expression.length() > 0) {
							expression[expression.length() - 1] = '/';
						}
					}
				}
				else if (bar_count == 2) {
					options = part;
					++bar_count;
				}
				else {
					++bar_count;
				}
			}

			if (command == "m") {
				if (bar_count == 3) {
					_queryr.appendRegex(key, expression, options);
					continue;
				}
			}
			else if (command == "n") {
				if (bar_count == 2) {
					istringstream iss(expression);
					int i = 0;
					iss >> i;
					if (!iss.eof()) {
						iss.clear();
						double d = 0;
						iss >> d;
						if (!iss.eof()) {
							string bexpr(expression.data());
							std::transform(bexpr.begin(), bexpr.end(), bexpr.begin(), ::tolower);
							if (bexpr != "true" && bexpr != "false") {
								_queryr.append(key, expression);
							}
							else {
								_queryr.append(key, bexpr == "true");
							}
						}
						else {
							_queryr.append(key, d);
						}
					}
					else {
						_queryr.append(key, i);
					}
					continue;
				}
			}
			else {
				string comp("$");
				comp.insert(comp.length(), command);

				if (_VALID_OPS.find(comp + string("^")) != string::npos) {
					if (bar_count == 2) {
						_queryr.append(key, BSON(comp << expression));
					}
					else if (options == "n") {
						istringstream iss(expression);
						int i = 0;
						iss >> i;
						if (!iss.eof()) {
							iss.str(expression);
							double d = 0;
							iss >> d;
							if (!iss.eof()) {
								_queryr.append(key, BSON(comp << expression));
							}
							else {
								_queryr.append(key, BSON(comp << d));
							}
						}
						else {
							_queryr.append(key, BSON(comp << i));
						}
					}
					continue;
				}

			}
		}

		_queryr.append(key, value);
	}
}

void zapata::torestcollection(mongo::ScopedDbConnection* _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out) {
	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::tomongoquery(_params, _query_b, _order_b, _page_size, _page_start_index);

	size_t _count = -1;
	mongo::Query _query(_query_b.done());
	mongo::BSONObj _order = _order_b.done();

	_count = (*_conn)->count(_mongo_collection, _query.obj);
	if (!_order.isEmpty()) {
		_query.sort(_order);
	}

	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_mongo_collection, _query, (_page_start_index > 0 ? _page_size : _page_start_index + _page_size), (_page_start_index > 0 ? _page_start_index : 0));

	zapata::JSONArr _elements;
	while (_ptr->more()) {
		mongo::BSONObj _obj = _ptr->next();
		zapata::JSONObj _record;
		zapata::frommongo(_obj, _record);
		_elements << _record;
	}
	 _ptr->decouple();
	 (*_conn)->killCursor(_ptr->getCursorId());
	 delete _ptr.release();

	_out << "size" << _count;
	_out << "elements" << _elements;
}

void zapata::toreststore(mongo::ScopedDbConnection* _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out) {
	zapata::torestcollection(_conn,  _mongo_collection, _params, _out);
}

void zapata::torestdocument(mongo::ScopedDbConnection* _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out) {
	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::tomongoquery(_params, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	unique_ptr<mongo::DBClientCursor> _ptr = (*_conn)->query(_mongo_collection, _query);

	if (_ptr->more()) {
		mongo::BSONObj _obj = _ptr->next();
		zapata::frommongo(_obj, _out);
	}
	 _ptr->decouple();
	 (*_conn)->killCursor(_ptr->getCursorId());
	 delete _ptr.release();
}
