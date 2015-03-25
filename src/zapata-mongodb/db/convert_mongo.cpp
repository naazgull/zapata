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

#include <zapata/db/convert_mongo.h>
#include <zapata/http/HTTPObj.h>
#include <zapata/api/codes_rest.h>
#include <zapata/parsers/json.h>
#include <zapata/log/log.h>

#define _VALID_OPS string("$gt^$gte^$lt^$lte^$ne^$type^$exists^")

void zapata::mongodb::frommongo(mongo::BSONObj& _in, zapata::JSONObj& _out) {
	for (mongo::BSONObjIterator _i = _in.begin(); _i.more();) {
		mongo::BSONElement _it = _i.next();
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
				mongo::BSONObj _mobj = _it.Obj();
				zapata::JSONObj _obj;
				zapata::mongodb::frommongo(_mobj, _obj);
				_out << _key << _obj;
				break;
			}
			case mongo::Array: {
				zapata::JSONArr _arr;
				zapata::mongodb::frommongo(_it, _arr);
				_out << _key << _arr;
				break;
			}
			default: {
				continue;
			}
		}
	}
}

void zapata::mongodb::frommongo(mongo::BSONElement& _in, zapata::JSONArr& _out) {
	vector<mongo::BSONElement> _obj = _in.Array();
	for (vector<mongo::BSONElement>::iterator _i = _obj.begin(); _i != _obj.end(); _i++) {
		mongo::BSONElement _it = *_i;
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
				mongo::BSONObj _mobj = _it.Obj();
				zapata::JSONObj _obj;
				zapata::mongodb::frommongo(_mobj, _obj);
				_out <<  _obj;
				break;
			}
			case mongo::Array: {
				zapata::JSONArr _arr;
				zapata::mongodb::frommongo(_it, _arr);
				_out << _arr;
				break;
			}
			default: {
				continue;
			}
		}
	}
}

void zapata::mongodb::tomongo(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _out) {
	for (auto _i : *_in) {
		string _key = _i.first;
		JSONElement _value = _i.second;

		switch (_value->type()) {
			case zapata::JSObject: {
				mongo::BSONObjBuilder _mobj;
				zapata::mongodb::tomongo(_value->obj(), _mobj);
				_out << _key << _mobj.obj();
				break;
			}
			case zapata::JSArray: {
				mongo::BSONArrayBuilder _mobj;
				zapata::mongodb::tomongo(_value->arr(), _mobj);
				_out << _key << _mobj.arr();
				break;
			}
			case zapata::JSString: {
				_out << _key << (string) _value;
				break;
			}
			case zapata::JSBoolean: {
				_out << _key << (bool) _value;
				break;
			}
			case zapata::JSDouble: {
				_out << _key << (double) _value;
				break;
			}
			case zapata::JSInteger: {
				_out << _key << (int) _value;
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

void zapata::mongodb::tomongo(zapata::JSONArr& _in, mongo::BSONArrayBuilder&  _out) {
	for (auto _i : *_in) {
		JSONPtr _value = _i;

		switch (_value->type()) {
			case zapata::JSObject: {
				mongo::BSONObjBuilder _mobj;
				zapata::mongodb::tomongo(_value->obj(), _mobj);
				_out << _mobj.obj();
				break;
			}
			case zapata::JSArray: {
				mongo::BSONArrayBuilder _mobj;
				zapata::mongodb::tomongo(_value->arr(), _mobj);
				_out << _mobj.arr();
				break;
			}
			case zapata::JSString: {
				_out << (string) _value;
				break;
			}
			case zapata::JSBoolean: {
				_out << (bool) _value;
				break;
			}
			case zapata::JSDouble: {
				_out << (double) _value;
				break;
			}
			case zapata::JSInteger: {
				_out << (int) _value;
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

void zapata::mongodb::tosetcommand(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _out, string _prefix) {
	for (auto _i : *_in) {
		string _key = _i.first;
		JSONElement _value = _i.second;

		switch (_value->type()) {
			case zapata::JSObject: {
				zapata::mongodb::tosetcommand(_value->obj(), _out, (_prefix.length() != 0 ? _prefix + string(".") + _key : _key));
				break;
			}
			case zapata::JSArray: {
				zapata::mongodb::tosetcommand(_value->arr(), _out, (_prefix.length() != 0 ? _prefix + string(".") + _key : _key));
				break;
			}
			case zapata::JSString: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + _key : _key) << (string) _value;
				break;
			}
			case zapata::JSBoolean: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + _key : _key) << (bool) _value;
				break;
			}
			case zapata::JSDouble: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + _key : _key) << (double) _value;
				break;
			}
			case zapata::JSInteger: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + _key : _key) << (int) _value;
				break;
			}
			case zapata::JSNil: {
				_out.appendNull((_prefix.length() != 0 ? _prefix + string(".") + _key : _key));
				break;
			}
			default: {
				continue;
			}
		}
	}
}

void zapata::mongodb::tosetcommand(zapata::JSONArr& _in, mongo::BSONObjBuilder&  _out, string _prefix) {
	size_t _idx = 0;
	for (auto _i : *_in) {
		JSONPtr _value = _i;

		switch (_value->type()) {
			case zapata::JSObject: {
				zapata::mongodb::tosetcommand(_value->obj(), _out, (_prefix.length() != 0 ? _prefix + string(".") + std::to_string(_idx) : std::to_string(_idx)));
				break;
			}
			case zapata::JSArray: {
				zapata::mongodb::tosetcommand(_value->arr(), _out, (_prefix.length() != 0 ? _prefix + string(".") + std::to_string(_idx) : std::to_string(_idx)));
				break;
			}
			case zapata::JSString: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + std::to_string(_idx) : std::to_string(_idx)) << (string) _value;
				break;
			}
			case zapata::JSBoolean: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + std::to_string(_idx) : std::to_string(_idx)) << (bool) _value;
				break;
			}
			case zapata::JSDouble: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + std::to_string(_idx) : std::to_string(_idx)) << (double) _value;
				break;
			}
			case zapata::JSInteger: {
				_out << (_prefix.length() != 0 ? _prefix + string(".") + std::to_string(_idx) : std::to_string(_idx)) << (int) _value;
				break;
			}
			case zapata::JSNil: {
				_out.appendNull((_prefix.length() != 0 ? _prefix + string(".") + std::to_string(_idx) : std::to_string(_idx)));
				break;
			}
			default: {
				continue;
			}
		}

		_idx++;
	}
}

void zapata::mongodb::get_query(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _queryr, mongo::BSONObjBuilder& _order, size_t& _page_size, size_t& _page_start_index) {
	for (auto _i : *_in) {
		string _key = _i.first;
		JSONElement _value = _i.second;

		if (_key == "fields" || _key == "embed") {
			continue;
		}

		if (_key == "pageSize") {
			istringstream iss((string) _value);
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
			istringstream iss((string) _value);
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
			istringstream lss(((string) _value).data());
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
		string value = (string) _value;
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

					if (expression.length() == 0 || expression[expression.length() - 1] != '\\') {
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

zapata::JSONPtr zapata::mongodb::get_collection(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params) {
	zapata::JSONObj _out;

	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_params, _query_b, _order_b, _page_size, _page_start_index);

	size_t _count = -1;
	mongo::Query _query(_query_b.done());
	mongo::BSONObj _order = _order_b.done();

	_count = _conn->count(_collection, _query.obj);
	assertz(_count != 0, "The requested resource is empty", zapata::HTTP204, zapata::ERRResourceIsEmpty);

	if (!_order.isEmpty()) {
		_query.sort(_order);
	}

	std::unique_ptr<mongo::DBClientCursor> _ptr = _conn->query(_collection, _query, (_page_start_index > 0 ? _page_size : _page_start_index + _page_size), (_page_start_index > 0 ? _page_start_index : 0));

	zapata::JSONArr _elements;
	while (_ptr->more()) {
		mongo::BSONObj _obj = _ptr->next();
		zapata::JSONObj _record;
		zapata::mongodb::frommongo(_obj, _record);
		_record->pop("_id");
		_elements << _record;
	}
	_ptr->decouple();
	delete _ptr.release();

	_out << "size" << _count;
	_out << "elements" << _elements;

	_conn.done();
	return zapata::JSONPtr(make_element(_out));
}

zapata::JSONPtr zapata::mongodb::patch_from_collection(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _payload) {
	zapata::JSONObj _out;

	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_params, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	size_t _count = -1;
	_count = _conn->count(_collection, _query.obj);
	assertz(_count != 0, "The requested resource was not found", zapata::HTTP204, zapata::ERRResourceNotFound);

	mongo::BSONObjBuilder _bo;
	zapata::mongodb::tosetcommand(_payload, _bo);
	mongo::BSONObj _patch = _bo.obj();
	_conn->update(_collection, _query, BSON( "$set" << _patch) );
	assertz(_conn->getLastError().length() == 0, string("There has been an error trying to patch the document: ") + _conn->getLastError(), zapata::HTTP412, zapata::ERRGeneric);		

	_conn.done();
	_out << "patched" << _count;
	return zapata::JSONPtr(make_element(_out));
}

zapata::JSONPtr zapata::mongodb::delete_from_collection(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params) {
	zapata::JSONObj _out;

	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_params, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	size_t _count = -1;
	_count = _conn->count(_collection, _query.obj);
	assertz(_count != 0, "The requested resource was not found", zapata::HTTP204, zapata::ERRResourceNotFound);

	_conn->remove(_collection, _query);
	assertz(_conn->getLastError().length() == 0, string("There has been an error trying to patch the document: ") + _conn->getLastError(), zapata::HTTP412, zapata::ERRGeneric);		

	_conn.done();
	_out << "removed" << _count;
	return zapata::JSONPtr(make_element(_out));
}

zapata::JSONPtr zapata::mongodb::get_store(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params) {
	return zapata::mongodb::get_collection(_config,  _mongo_collection, _params);
}

zapata::JSONPtr zapata::mongodb::patch_from_store(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _payload) {
	return zapata::mongodb::patch_from_collection(_config, _mongo_collection, _params, _payload);
}

zapata::JSONPtr zapata::mongodb::delete_from_store(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params) {
	return zapata::mongodb::delete_from_collection(_config, _mongo_collection, _params);
}

zapata::JSONPtr zapata::mongodb::get_document(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params) {
	zapata::JSONObj _out;

	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_params, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	std::unique_ptr<mongo::DBClientCursor> _ptr = _conn->query(_collection, _query);

	assertz(_ptr->more(), "The requested resource was not found", zapata::HTTP404, zapata::ERRResourceNotFound);

	mongo::BSONObj _obj = _ptr->next();
	zapata::mongodb::frommongo(_obj, _out);
	_out->pop("_id");
	_ptr->decouple();
	delete _ptr.release();

	_conn.done();
	return zapata::JSONPtr(make_element(_out));
}

zapata::JSONPtr zapata::mongodb::replace_document(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _payload) {
	zapata::JSONObj _out;

	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_params, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	std::unique_ptr<mongo::DBClientCursor> _ptr = _conn->query(_collection, _query);

	assertz(_ptr->more(), "The requested resource was not found", zapata::HTTP404, zapata::ERRResourceNotFound);
	mongo::BSONObj _obj = _ptr->next();
	zapata::mongodb::frommongo(_obj, _out);
	_ptr->decouple();
	delete _ptr.release();

	mongo::BSONObjBuilder _bo;
	zapata::mongodb::tomongo(_payload, _bo);
	try {
		_conn->update(_collection, BSON("_id" << (string) _out["_id"]), _bo.obj());
	}
	catch (exception& _e) {
		assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
	}
	assertz(_conn->getLastError().length() == 0, string("There has been an error trying to replace the document: ") + _conn->getLastError(), zapata::HTTP412, zapata::ERRGeneric);

	_out->pop("_id");
	_conn.done();
	return zapata::JSONPtr(make_element(_out));
}

zapata::JSONPtr zapata::mongodb::patch_document(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _payload) {
	zapata::JSONObj _out;

	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_params, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	std::unique_ptr<mongo::DBClientCursor> _ptr = _conn->query(_collection, _query);

	assertz(_ptr->more(), "The requested resource was not found", zapata::HTTP404, zapata::ERRResourceNotFound);
	mongo::BSONObj _obj = _ptr->next();
	zapata::mongodb::frommongo(_obj, _out);
	_ptr->decouple();
	delete _ptr.release();

	mongo::BSONObjBuilder _bo;
	zapata::mongodb::tosetcommand(_payload, _bo);
	mongo::BSONObj _set = _bo.obj();
	try {
		_conn->update(_collection, BSON("_id" << (string) _out["_id"]), BSON( "$set" << _set) );
	}
	catch (exception& _e) {
		assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
	}
	assertz(_conn->getLastError().length() == 0, string("There has been an error trying to patch the document: ") + _conn->getLastError(), zapata::HTTP412, zapata::ERRGeneric);

	_out->pop("_id");
	_conn.done();
	return zapata::JSONPtr(make_element(_out));
}

zapata::JSONPtr zapata::mongodb::delete_document(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _params) {
	zapata::JSONObj _out;

	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	size_t _page_size = 0;
	size_t _page_start_index = 0;
	mongo::BSONObjBuilder _query_b;
	mongo::BSONObjBuilder _order_b;
	zapata::mongodb::get_query(_params, _query_b, _order_b, _page_size, _page_start_index);

	mongo::Query _query(_query_b.done());
	std::unique_ptr<mongo::DBClientCursor> _ptr = _conn->query(_collection, _query);

	assertz(_ptr->more(), "The requested resource was not found", zapata::HTTP404, zapata::ERRResourceNotFound);
	mongo::BSONObj _obj = _ptr->next();
	zapata::mongodb::frommongo(_obj, _out);
	_ptr->decouple();
	delete _ptr.release();

	try {
		_conn->remove(_collection, BSON("_id" << (string) _out["_id"]));
	}
	catch (exception& _e) {
		assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
	}
	assertz(_conn->getLastError().length() == 0, string("There has been an error trying to delete the document: ") + _conn->getLastError(), zapata::HTTP412, zapata::ERRGeneric);

	_out->pop("_id");
	_conn.done();
	return zapata::JSONPtr(make_element(_out));
}

zapata::JSONPtr zapata::mongodb::create_document(zapata::JSONObj& _config, string _mongo_collection, zapata::JSONObj& _payload) {
	mongo::ScopedDbConnection _conn((string) _config["zapata"]["mongodb"]["address"]);
	if (_config["zapata"]["mongodb"]["user"]->ok()) {
		_conn->auth(BSON("mechanism" << "MONGODB-CR" << "user" << (string) _config["zapata"]["mongodb"]["user"] << "pwd" << (string) _config["zapata"]["mongodb"]["passwd"] << "db" << (string) _config["zapata"]["mongodb"]["db"]));
	}
	string _collection((string) _config["zapata"]["mongodb"]["db"]);
	_collection.insert(_collection.length(), string(".") + _mongo_collection);

	bool _exists = false;
	if (_payload["id"]->ok()) {
		mongo::BSONObj _found = _conn->findOne(_collection, QUERY("id" << (string) _payload["id"]));
		_exists = _found.nFields() != 0;
	}

	if (!_exists) {
		if (!_payload["_id"]->ok()) {
			string _id(string("/") + _mongo_collection + string("/"));
			_id.insert(_id.length(), mongo::OID::gen().toString());
			_payload << "_id" << _id;
		}
		if (!_payload["href"]->ok()) {
			_payload << "href" << (string) _payload["_id"];
		}
		
		mongo::BSONObjBuilder _bo;
		zapata::mongodb::tomongo(_payload, _bo);
		try {
			_conn->insert(_collection, _bo.obj());
		}
		catch (exception& _e) {
			assertz(false, _e.what(), zapata::HTTP500, zapata::ERRGeneric);
		}
		assertz(_conn->getLastError().length() == 0, string("There has been an error trying to create the document: ") + _conn->getLastError(), zapata::HTTP412, zapata::ERRGeneric);
	}	
	assertz(!_exists, "Already exists a user identified by that ID", zapata::HTTP412, zapata::ERRResourceDuplicate);

	_payload->pop("_id");
	_conn.done();
	return zapata::JSONPtr(make_element(_payload));
}

extern "C" int zapata_mongodb() {
	return 1;
}