/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

#include <zapata/postgresql/Client.h>

zpt::pgsql::ClientPtr::ClientPtr(zpt::pgsql::Client * _target) : std::shared_ptr<zpt::pgsql::Client>(_target) {
}

zpt::pgsql::ClientPtr::ClientPtr(zpt::json _options, std::string _conf_path) : std::shared_ptr<zpt::pgsql::Client>(new zpt::pgsql::Client(_options, _conf_path)) {
}

zpt::pgsql::ClientPtr::~ClientPtr() {
}

zpt::pgsql::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __conn(nullptr) {
	this->connection(_options->getPath(_conf_path));
}

zpt::pgsql::Client::~Client() {
}

auto zpt::pgsql::Client::conn() -> pqxx::connection& {
	return (*this->__conn.get());
}

auto zpt::pgsql::Client::name() -> std::string {
	return std::string("pgsql://") + ((std::string) this->connection()["bind"]) + std::string("/") + ((std::string) this->connection()["db"]);
}

auto zpt::pgsql::Client::options() -> zpt::json {
	return this->__options;
}

auto zpt::pgsql::Client::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::pgsql::Client::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::pgsql::Client::connect() -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	std::string _s_conn = this->connection()["bind"]->str() + std::string(" dbname=") + this->connection()["db"]->str() + (this->connection()["user"]->ok() ? std::string(" user=") + this->connection()["user"]->str() + std::string(" password=") + this->connection()["passwd"]->str() : "") + std::string("");
	this->__conn.reset(new pqxx::connection(_s_conn));
	zpt::Connector::connect();
}

auto zpt::pgsql::Client::reconnect() -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0);
	this->__conn.release();
	this->__conn.reset(new pqxx::connection(this->connection()["bind"]->str() + std::string(" dbname=") + this->connection()["db"]->str() + (this->connection()["user"]->ok() ? std::string(" user=") + this->connection()["user"]->str() + std::string(" password=") + this->connection()["passwd"]->str() : "") + std::string("")));
	zpt::Connector::reconnect();
}

auto zpt::pgsql::Client::insert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {	
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	if (!_document["id"]->ok()) {
		_document << "id" << zpt::generate::r_uuid();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}
	
	std::string _expression("INSERT INTO ");
	_expression += zpt::pgsql::escape_name(_collection);
	_expression += std::string(" (");
	std::string _columns = zpt::pgsql::get_column_names(_document, _opts);
	std::string _values = zpt::pgsql::get_column_values(_document, _opts);
	_expression += _columns + std::string(") VALUES (") + _values + (")");
	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_size = _stmt.exec(_expression).affected_rows();
			_stmt.commit(); }
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in insert: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}
	if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::pgsql::Client::upsert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {	
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	{
		if (_document["href"]->ok() || _document["id"]->ok()) {
			if (!_document["href"]->ok()) {
				_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
			}
			if (!_document["id"]->ok()) {
				zpt::json _split = zpt::split(_document["href"]->str(), "/");
				_document << "id" << _split->arr()->back();
			}
			std::string _href = std::string(_document["href"]);
			std::string _expression("UPDATE ");
			_expression += zpt::pgsql::escape_name(_collection);
			_expression += std::string(" SET ");
			std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
			_expression += _sets;

			zpt::json _splited = zpt::split(_href, "/");
			_expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->arr()->back());

			int _size = 0;
			try {
				{ std::lock_guard< std::mutex > _lock(this->__mtx);
					pqxx::work _stmt(this->conn());
					_size = _stmt.exec(_expression).affected_rows();
					_stmt.commit(); }
			}
			catch(std::exception& _e) {
				zlog(std::string("pgsql: error in set: ") + _e.what(), zpt::error);
				assertz(false, _e.what(), 412, 0);
			}

			if (_size != 0) {
				if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
				return _document["id"]->str();
			}
		}
	}
	{
		if (!_document["id"]->ok()) {
			_document << "id" << zpt::generate::r_uuid();
		}
		if (!_document["href"]->ok() && _href_prefix.length() != 0) {
			_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
		}
	
		std::string _expression("INSERT INTO ");
		_expression += zpt::pgsql::escape_name(_collection);
		_expression += std::string(" (");
		std::string _columns = zpt::pgsql::get_column_names(_document, _opts);
		std::string _values = zpt::pgsql::get_column_values(_document, _opts);
		_expression += _columns + std::string(") VALUES (") + _values + (")");
		int _size = 0;
		try {
			{ std::lock_guard< std::mutex > _lock(this->__mtx);
				pqxx::work _stmt(this->conn());
				_size = _stmt.exec(_expression).affected_rows();
				_stmt.commit(); }
		}
		catch(std::exception& _e) {
			zlog(std::string("pgsql: error in insert: ") + _e.what(), zpt::error);
			assertz(false, _e.what(), 412, 0);
		}
		if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	}
	return _document["id"]->str();
}

auto zpt::pgsql::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _expression("UPDATE ");
	_expression += zpt::pgsql::escape_name(_collection);
	_expression += std::string(" SET ");
	std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
	_expression += _sets;

	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->arr()->back());

	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_size = _stmt.exec(_expression).affected_rows();
			_stmt.commit(); }
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in save: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}

	if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::pgsql::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _expression("UPDATE ");
	_expression += zpt::pgsql::escape_name(_collection);
	_expression += std::string(" SET ");
	std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
	_expression += _sets;

	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->arr()->back());

	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_size = _stmt.exec(_expression).affected_rows();
			_stmt.commit(); }
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in set: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}

	if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::pgsql::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _expression("UPDATE ");
	_expression += zpt::pgsql::escape_name(_collection);
	_expression += std::string(" SET ");
	std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
	_expression += _sets;

	if (_pattern->ok() && _pattern->type() == zpt::JSObject) {
		std::string _where;
		zpt::pgsql::get_query(_pattern, _where);
		_expression += std::string(" WHERE ") + _where;
	}
	zpt::pgsql::get_opts(_opts, _expression);
	
	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_size = _stmt.exec(_expression).affected_rows();
			_stmt.commit(); }
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in set: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}

	if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::pgsql::Client::unset(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _expression("UPDATE ");
	_expression += zpt::pgsql::escape_name(_collection);
	_expression += std::string(" SET ");
	std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
	_expression += _sets;

	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->arr()->back());

	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_size = _stmt.exec(_expression).affected_rows();
			_stmt.commit(); }
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in unset: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}

	if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
	return 1;
}

auto zpt::pgsql::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);

	std::string _expression("UPDATE ");
	_expression += zpt::pgsql::escape_name(_collection);
	_expression += std::string(" SET ");
	std::string _sets = zpt::pgsql::get_column_sets(_document, _opts);
	_expression += _sets;

	if (_pattern->ok() && _pattern->type() == zpt::JSObject) {
		std::string _where;
		zpt::pgsql::get_query(_pattern, _where);
		_expression += std::string(" WHERE ") + _where;
	}
	zpt::pgsql::get_opts(_opts, _expression);
	
	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_size = _stmt.exec(_expression).affected_rows();
			_stmt.commit(); }
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in unset: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}

	if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::pgsql::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }

	std::string _expression("DELETE FROM ");
	_expression += zpt::pgsql::escape_name(_collection);

	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->arr()->back());

	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_size = _stmt.exec(_expression).affected_rows();
			_stmt.commit(); }
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in remove: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}

	if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _href, _opts);
	return 1;
}

auto zpt::pgsql::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }

	zpt::json _selected = this->query(_collection, _pattern, _opts);
	for (auto _record : _selected["elements"]->arr()) {
		std::string _expression = std::string("DELETE FROM ") + zpt::pgsql::escape_name(_collection) + std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_record["id"]);
		int _size = 0;
		try {
			{ std::lock_guard< std::mutex > _lock(this->__mtx);
				pqxx::work _stmt(this->conn());
				_size = _stmt.exec(_expression).affected_rows();
				_stmt.commit(); }
		}
		catch(std::exception& _e) {
			zlog(std::string("pgsql: error in query: ") + _e.what(), zpt::error);
			assertz(false, _e.what(), 412, 0);
		}

		if (_size != 0 && !bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _record["href"]->str(), _opts);
	}
	
	return int(_selected["size"]);
}

auto zpt::pgsql::Client::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
	std::string _expression("SELECT ");
	_expression += zpt::pgsql::get_column_names(zpt::undefined, _opts);
	_expression += std::string(" FROM ");
	_expression += zpt::pgsql::escape_name(_collection);
	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE \"id\"=") + zpt::pgsql::escape(_splited->arr()->back());
	return this->query(_collection, _expression, _opts)[0];
}

auto zpt::pgsql::Client::query(std::string _collection, std::string _pattern, zpt::json _opts) -> zpt::json {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to PostgreSQL at ") + this->name() + std::string(" has not been established."), 500, 0); }
	zpt::json _elements = zpt::json::array();

	try {
		pqxx::result _result;
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			pqxx::work _stmt(this->conn());
			_result = _stmt.exec(_pattern); }
		for (auto _r : _result) {
			_elements << zpt::pgsql::fromsql_r(_r);
		}
	}
	catch(std::exception& _e) {
		zlog(std::string("pgsql: error in query: ") + _e.what(), zpt::error);
		assertz(false, _e.what(), 412, 0);
	}
	return _elements;
}

auto zpt::pgsql::Client::query(std::string _collection, zpt::json _pattern, zpt::json _opts) -> zpt::json {
	std::string _expression("SELECT ");
	_expression += zpt::pgsql::get_column_names(zpt::undefined, _opts);
	_expression += std::string(" FROM ");
	std::string _count_expression("SELECT COUNT(1) FROM ");
	_expression += zpt::pgsql::escape_name(_collection);
	_count_expression += zpt::pgsql::escape_name(_collection);
	if (_pattern->is_object()) {
		std::string _where;
		zpt::pgsql::get_query(_pattern, _where);
		if (_where.length() != 0) {
			_expression += std::string(" WHERE ") + _where;
			_count_expression += std::string(" WHERE ") + _where;
		}
	}
	zpt::pgsql::get_opts(_opts, _expression);
	size_t _size = size_t(this->query(_collection, _count_expression, _opts)[0]["count"]);
	zpt::json _return = {
		"size", _size, 
		"elements", this->query(_collection, _expression, _opts)
	};
	return _return;
}

auto zpt::pgsql::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
	std::string _expression("SELECT ");
	_expression += zpt::pgsql::get_column_names(zpt::undefined, _opts);
	_expression += std::string(" FROM ");
	std::string _count_expression("SELECT COUNT(1) FROM ");
	_expression += zpt::pgsql::escape_name(_collection);
	_count_expression += zpt::pgsql::escape_name(_collection);
	zpt::pgsql::get_opts(_opts, _expression);
	size_t _size = size_t(this->query(_collection, _count_expression, _opts)[0]["count"]);
	zpt::json _return = {
		"size", _size, 
		"elements", this->query(_collection, _expression, _opts)
	};
	return _return;
}

extern "C" auto zpt_postgresql() -> int {
	return 1;
}
