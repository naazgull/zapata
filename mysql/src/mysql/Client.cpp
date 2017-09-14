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

#include <zapata/mysql/Client.h>

zpt::mysql::ClientPtr::ClientPtr(zpt::mysql::Client * _target) : std::shared_ptr<zpt::mysql::Client>(_target) {
}

zpt::mysql::ClientPtr::ClientPtr(zpt::json _options, std::string _conf_path) : std::shared_ptr<zpt::mysql::Client>(new zpt::mysql::Client(_options, _conf_path)) {
}

zpt::mysql::ClientPtr::~ClientPtr() {
}

zpt::mysql::Client::Client(zpt::json _options, std::string _conf_path) : __options( _options), __conn(nullptr) {
	this->connection(_options->getPath(_conf_path));
}

zpt::mysql::Client::~Client() {
	if (this->__conn.get() != nullptr) {
		this->__conn->close();
		this->__conn.release();
	}
}

auto zpt::mysql::Client::name() -> std::string {
	return std::string("mysql://") + ((std::string) this->connection()["bind"]) + std::string("/") + ((std::string) this->connection()["db"]);
}

auto zpt::mysql::Client::options() -> zpt::json {
	return this->__options;
}

auto zpt::mysql::Client::events(zpt::ev::emitter _emitter) -> void {
	this->__events = _emitter;
}

auto zpt::mysql::Client::events() -> zpt::ev::emitter {
	return this->__events;
}

auto zpt::mysql::Client::connect() -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	this->__conn.reset(sql::mysql::get_mysql_driver_instance()->connect(string("tcp://") + this->connection()["bind"]->str(), std::string(this->connection()["user"]), std::string(this->connection()["passwd"])));
	zpt::Connector::connect();
}

auto zpt::mysql::Client::reconnect() -> void {
	std::lock_guard< std::mutex > _lock(this->__mtx);
	assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
	this->__conn->close();
	this->__conn.release();
	this->__conn.reset(sql::mysql::get_mysql_driver_instance()->connect(string("tcp://") + this->connection()["bind"]->str(), std::string(this->connection()["user"]), std::string(this->connection()["passwd"])));
	zpt::Connector::reconnect();
}

auto zpt::mysql::Client::insert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	if (!_document["id"]->ok()) {
		_document << "id" << zpt::generate::r_uuid();
	}
	if (!_document["href"]->ok() && _href_prefix.length() != 0) {
		_document << "href" << (_href_prefix + (_href_prefix.back() != '/' ? std::string("/") : std::string("")) + _document["id"]->str());
	}
	
	std::string _expression("INSERT INTO ");
	_expression += _collection;
	_expression += std::string("(");
	std::string _columns;
	std::string _values;
	for (auto _c : _document->obj()){
		if (_columns.length() != 0) {
			_columns += std::string(",");
		}
		_columns += _c.first;
		if (_values.length() != 0) {
			_values += std::string(",");
		}
		std::string _val;
		_c.second->stringify(_val);
		_values += _val;
	}

	_expression += _columns + std::string(") VALUES (") + _values + (")");
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
			_stmt->execute(_expression); }
	}
	catch(std::exception& _e) {
		assertz(false, _e.what(), 412, 0);
	}

	if (!bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	return _document["id"]->str();
}

auto zpt::mysql::Client::upsert(std::string _collection, std::string _href_prefix, zpt::json _document, zpt::json _opts) -> std::string {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

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
			_expression += _collection;
			_expression += std::string(" SET ");
			std::string _sets;
			for (auto _c : _document->obj()){
				if (_sets.length() != 0) {
					_sets += std::string(",");
				}
				std::string _val;
				_c.second->stringify(_val);
				_sets += _c.first + std::string("=") + _val;
			}
			_expression += _sets;

			zpt::json _splited = zpt::split(_href, "/");
			_expression += std::string(" WHERE id=") + zpt::mysql::escape(_splited->arr()->back());

			try {
				{ std::lock_guard< std::mutex > _lock(this->__mtx);
					std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
					_stmt->execute(_expression); }
			}
			catch(std::exception& _e) {}

			if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
			return _document["id"]->str();
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
		_expression += _collection;
		_expression += std::string("(");
		std::string _columns;
		std::string _values;
		for (auto _c : _document->obj()){
			if (_columns.length() != 0) {
				_columns += std::string(",");
			}
			_columns += _c.first;
			if (_values.length() != 0) {
				_values += std::string(",");
			}
			std::string _val;
			_c.second->stringify(_val);
			_values += _val;
		}

		_expression += _columns + std::string(") VALUES (") + _values + (")");
		try {
			{ std::lock_guard< std::mutex > _lock(this->__mtx);
				std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
				_stmt->execute(_expression); }
		}
		catch(std::exception& _e) {
			assertz(false, _e.what(), 412, 0);
		}

		if (!bool(_opts["mutated-event"])) zpt::Connector::insert(_collection, _href_prefix, _document, _opts);
	}
	return _document["id"]->str();
}

auto zpt::mysql::Client::save(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {	
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	std::string _expression("UPDATE ");
	_expression += _collection;
	_expression += std::string(" SET ");
	std::string _sets;
	for (auto _c : _document->obj()){
		if (_sets.length() != 0) {
			_sets += std::string(",");
		}
		std::string _val;
		_c.second->stringify(_val);
		_sets += _c.first + std::string("=") + _val;
	}
	_expression += _sets;

	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE id=") + zpt::mysql::escape(_splited->arr()->back());

	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
			_size = _stmt->execute(_expression); }
	}
	catch(std::exception& _e) {}

	if (!bool(_opts["mutated-event"])) zpt::Connector::save(_collection, _href, _document, _opts);
	return _size;
}

auto zpt::mysql::Client::set(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	std::string _expression("UPDATE ");
	_expression += _collection;
	_expression += std::string(" SET ");
	std::string _sets;
	for (auto _c : _document->obj()){
		if (_sets.length() != 0) {
			_sets += std::string(",");
		}
		std::string _val;
		_c.second->stringify(_val);
		_sets += _c.first + std::string("=") + _val;
	}
	_expression += _sets;

	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE id=") + zpt::mysql::escape(_splited->arr()->back());

	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
			_size = _stmt->execute(_expression); }
	}
	catch(std::exception& _e) {}

	if (!bool(_opts["mutated-event"])) zpt::Connector::set(_collection, _href, _document, _opts);
	return _size;
}

auto zpt::mysql::Client::set(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	std::string _expression("UPDATE ");
	_expression += _collection;
	_expression += std::string(" SET ");
	std::string _sets;
	for (auto _c : _document->obj()){
		if (_sets.length() != 0) {
			_sets += std::string(",");
		}
		std::string _val;
		_c.second->stringify(_val);
		_sets += _c.first + std::string("=") + _val;
	}
	_expression += _sets;

	if (_pattern->ok() && _pattern->type() == zpt::JSObject) {
		std::string _where;
		zpt::mysql::get_query(_pattern, _where);
		_expression += std::string(" WHERE ") + _where;
	}
	zpt::mysql::get_opts(_opts, _expression);
	
	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
			_size = _stmt->execute(_expression); }
	}
	catch(std::exception& _e) {}

	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::set(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::mysql::Client::unset(std::string _collection, std::string _href, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	std::string _expression("UPDATE ");
	_expression += _collection;
	_expression += std::string(" SET ");
	std::string _sets;
	for (auto _c : _document->obj()){
		if (_sets.length() != 0) {
			_sets += std::string(",");
		}
		_sets += _c.first + std::string("=NULL");
	}
	_expression += _sets;

	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE id=") + zpt::mysql::escape(_splited->arr()->back());

	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
			_size = _stmt->execute(_expression); }
	}
	catch(std::exception& _e) {}

	if (!bool(_opts["mutated-event"])) zpt::Connector::unset(_collection, _href, _document, _opts);
	return _size;
}

auto zpt::mysql::Client::unset(std::string _collection, zpt::json _pattern, zpt::json _document, zpt::json _opts) -> int {
	assertz(_document->ok() && _document->type() == zpt::JSObject, "'_document' must be of type JSObject", 412, 0);
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	std::string _expression("UPDATE ");
	_expression += _collection;
	_expression += std::string(" SET ");
	std::string _sets;
	for (auto _c : _document->obj()){
		if (_sets.length() != 0) {
			_sets += std::string(",");
		}
		_sets += _c.first + std::string("=NULL");
	}
	_expression += _sets;

	if (_pattern->ok() && _pattern->type() == zpt::JSObject) {
		std::string _where;
		zpt::mysql::get_query(_pattern, _where);
		_expression += std::string(" WHERE ") + _where;
	}
	zpt::mysql::get_opts(_opts, _expression);
	
	int _size = 0;
	try {
		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
			_size = _stmt->execute(_expression); }
	}
	catch(std::exception& _e) {}

	if (!bool(_opts["mutated-event"]) && _size != 0) zpt::Connector::unset(_collection, _pattern, _document, _opts);
	return _size;
}

auto zpt::mysql::Client::remove(std::string _collection, std::string _href, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	zpt::json _splited = zpt::split(_href, "/");
	std::string _expression = std::string("DELETE FROM ") +  _collection + std::string(" WHERE id=") + zpt::mysql::escape(_splited->arr()->back());

	int _size = 0;
	zpt::json _removed;
	try {
		if (!bool(_opts["mutated-event"])) _removed = this->get(_collection, _href);

		{ std::lock_guard< std::mutex > _lock(this->__mtx);
			std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
			_size = _stmt->execute(_expression); }
	}
	catch(std::exception& _e) {}

	if (!bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _href, _opts + zpt::json{ "removed", _removed });
	return _size;
}

auto zpt::mysql::Client::remove(std::string _collection, zpt::json _pattern, zpt::json _opts) -> int {
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	zpt::json _selected = this->query(_collection, _pattern, _opts);
	if (!_selected->ok()) {
		return 0;
	}
	for (auto _record : _selected["elements"]->arr()) {
		std::string _expression = std::string("DELETE FROM ") + _collection + std::string(" WHERE id=") + zpt::mysql::escape(_record["id"]);	
		try {
			{ std::lock_guard< std::mutex > _lock(this->__mtx);
				std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
				_stmt->execute(_expression); }
		}
		catch(std::exception& _e) {}

		if (!bool(_opts["mutated-event"])) zpt::Connector::remove(_collection, _record["href"]->str(), _opts + zpt::json{ "removed", _record });
	}
	
	return int(_selected["size"]);
}

auto zpt::mysql::Client::get(std::string _collection, std::string _href, zpt::json _opts) -> zpt::json {
	std::string _expression("SELECT * FROM ");
	zpt::json _splited = zpt::split(_href, "/");
	_expression += std::string(" WHERE id=") + zpt::mysql::escape(_splited->arr()->back());
	return this->query(_collection, _expression, _opts)["elements"][0];
}

auto zpt::mysql::Client::query(std::string _collection, std::string _pattern, zpt::json _opts) -> zpt::json {
	zpt::json _elements = zpt::json::array();
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		assertz(this->__conn.get() != nullptr, std::string("connection to MySQL at ") + this->name() + std::string(" has not been established."), 500, 0);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_stmt->execute(string("USE ") + this->connection()["db"]->str()); }

	std::shared_ptr<sql::ResultSet> _result;
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_result.reset(_stmt->executeQuery(_pattern)); }
	for (; _result->next(); ) {
		_elements << zpt::mysql::fromsql_r(_result);
	}
	if (_elements->arr()->size() == 0) {
		return zpt::undefined;
	}
	return { "size", _elements->arr()->size(), "elements", _elements };
}

auto zpt::mysql::Client::query(std::string _collection, zpt::json _pattern, zpt::json _opts) -> zpt::json {
	std::string _expression("SELECT * FROM ");
	std::string _count_expression("SELECT COUNT(1) FROM ");
	_expression += _collection;
	_count_expression += _collection;
	if (_pattern->ok() && _pattern->type() == zpt::JSObject) {
		std::string _where;
		zpt::mysql::get_query(_pattern, _where);
		_expression += std::string(" WHERE ") + _where;
		_count_expression += std::string(" WHERE ") + _where;
	}
	zpt::mysql::get_opts(_opts, _expression);

	std::shared_ptr<sql::ResultSet> _result;
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_result.reset(_stmt->executeQuery(_count_expression)); }
	size_t _size = 0;
	if (_result->next()) {
		_size = size_t(zpt::mysql::fromsql_r(_result)["count"]);
	}
	if (_size == 0) {
		return zpt::undefined;
	}
	
	zpt::json _return = this->query(_collection, _expression, _opts);
	if (_return->ok()) {
		_return << "size" << _size;
	}
	return _return;
	
}

auto zpt::mysql::Client::all(std::string _collection, zpt::json _opts) -> zpt::json {
	std::string _expression("SELECT * FROM ");
	std::string _count_expression("SELECT COUNT(1) FROM ");
	_expression += _collection;
	_count_expression += _collection;
	zpt::mysql::get_opts(_opts, _expression);

	std::shared_ptr<sql::ResultSet> _result;
	{ std::lock_guard< std::mutex > _lock(this->__mtx);
		std::unique_ptr<sql::Statement> _stmt(this->__conn->createStatement());
		_result.reset(_stmt->executeQuery(_count_expression)); }
	size_t _size = 0;
	if (_result->next()) {
		_size = size_t(zpt::mysql::fromsql_r(_result)["count"]);
	}
	if (_size == 0) {
		return zpt::undefined;
	}
	
	zpt::json _return = this->query(_collection, _expression, _opts);
	if (_return->ok()) {
		_return << "size" << _size;
	}
	return _return;
}

extern "C" auto zpt_mysql() -> int {
	return 1;
}
