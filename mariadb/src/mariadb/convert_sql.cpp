/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <n@zgul.me>

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
#include <zapata/mariadb/convert_sql.h>

namespace zpt {
	namespace mariadb {

		std::map<std::string, std::string> OPS = { { "gt", ">" }, { "gte", ">=" }, { "lt", "<" }, { "lte", "<=" }, { "ne", "<>" }, { "exists", "EXISTS" }, { "in", "IN" } };
		
	}
}

auto zpt::mariadb::fromsql(std::shared_ptr<sql::ResultSet> _in, zpt::json _out) -> void {
	sql::ResultSetMetaData* _metadata = _in->getMetaData();
	for (size_t _i = 0; _i != _metadata->getColumnCount(); _i++) {
		switch(_metadata->getColumnType(_i + 1)) {
			case sql::DataType::UNKNOWN : {
				break;
			}
			case sql::DataType::TINYINT: {
				_out << (std::string) _metadata->getColumnName(_i + 1) << _in->getBoolean(_i + 1);
				break;					
			}
			case sql::DataType::SMALLINT:
			case sql::DataType::MEDIUMINT:
			case sql::DataType::INTEGER: {
				_out << (std::string) _metadata->getColumnName(_i + 1) << _in->getInt(_i + 1);
				break;
			}
			case sql::DataType::BIGINT: {
				_out << (std::string) _metadata->getColumnName(_i + 1) << (long long int) _in->getInt64(_i + 1);
				break;
			}
			case sql::DataType::REAL:
			case sql::DataType::DOUBLE:
			case sql::DataType::DECIMAL:
			case sql::DataType::NUMERIC: {
				_out << (std::string) _metadata->getColumnName(_i + 1) << (double) _in->getDouble(_i + 1);
				break;
			}
			case sql::DataType::CHAR:
			case sql::DataType::VARCHAR:
			case sql::DataType::LONGVARCHAR: {
				_out << (std::string) _metadata->getColumnName(_i + 1) << (std::string) _in->getString(_i + 1);
				break;
			}
			case sql::DataType::TIMESTAMP: {
				std::string _ts(_in->getString(_i + 1) + std::string(".000Z"));
				zpt::replace(_ts, " ", "T");
				_out << (std::string) _metadata->getColumnName(_i + 1) << (zpt::timestamp_t) zpt::mkptr(_ts);
				break;
			}
			case sql::DataType::SQLNULL: {
				_out << (std::string) _metadata->getColumnName(_i + 1) << zpt::undefined;
				break;
			}
		}
	}
}

auto zpt::mariadb::fromsql_r(std::shared_ptr<sql::ResultSet> _in) -> zpt::json {
	zpt::json _return = zpt::json::object();
	zpt::mariadb::fromsql(_in, _return);
	return _return;
}

auto zpt::mariadb::get_query(zpt::json _in, std::string&  _queryr) -> void {
	if (!_in->is_object()) {
		return;
	}
	for (auto _i : _in->obj()) {
		std::string _key = _i.first;
		zpt::json _v = _i.second;

		if (_key == "page_size" || _key == "page_start_index" || _key == "order_by" || _key == "fields" || _key == "embed") {
			continue;
		}

		if (_queryr.length() != 0) {
			_queryr += std::string(" AND ");
		}

		std::string _value = (std::string) _v;
		if (_value.length() > 3 && _value.find('/') != std::string::npos) {
			int _bar_count = 0;
			std::istringstream _lss(_value);
			std::string _part;

			std::string _command;
			std::string _expression;
			std::string _options;
			while (std::getline(_lss, _part, '/')) {
				if (_bar_count == 0) {
					_command = _part;
					++_bar_count;
				}
				else if (_bar_count == 1) {
					_expression.append(_part);

					if (_expression.length() == 0 || _expression[_expression.length() - 1] != '\\') {
						++_bar_count;
					}
					else {
						if (_expression.length() > 0) {
							_expression[_expression.length() - 1] = '/';
						}
					}
				}
				else if (_bar_count == 2) {
					_options = _part;
					++_bar_count;
				}
				else {
					++_bar_count;
				}
			}

			if (_command == "m") {
				_queryr += zpt::mariadb::escape(_key) + std::string("=") + zpt::mariadb::escape(_expression);
				continue;
			}
			else if (_command == "n") {
				if (_bar_count == 2) {
					std::istringstream iss(_expression);
					int i = 0;
					iss >> i;
					if (!iss.eof()) {
						iss.clear();
						double d = 0;
						iss >> d;
						if (!iss.eof()) {
							std::string _bexpr(_expression.data());
							std::transform(_bexpr.begin(), _bexpr.end(), _bexpr.begin(), ::tolower);
							if (_bexpr != "true" && _bexpr != "false") {
								_queryr += zpt::mariadb::escape(_key) + std::string("=") + zpt::mariadb::escape(_expression);
							}
							else {
								_queryr += zpt::mariadb::escape(_key) + std::string("=") + _bexpr;
							}
						}
						else {
							_queryr += zpt::mariadb::escape(_key) + std::string("=") + std::to_string(d);
						}
					}
					else {
						_queryr += zpt::mariadb::escape(_key) + std::string("=") + std::to_string(i);
					}
					continue;
				}
			}
			else {
				std::map<std::string, std::string>::iterator _found = zpt::mariadb::OPS.find(_command);
				if (_found != zpt::mariadb::OPS.end()) {
					if (_bar_count == 2) {
						_queryr += zpt::mariadb::escape(_key) + _found->second + zpt::mariadb::escape(_expression);
					}
					else if (_options == "n") {
						std::istringstream iss(_expression);
						int i = 0;
						iss >> i;
						if (!iss.eof()) {
							iss.clear();
							double d = 0;
							iss >> d;
							if (!iss.eof()) {
								std::string _bexpr(_expression.data());
								std::transform(_bexpr.begin(), _bexpr.end(), _bexpr.begin(), ::tolower);
								if (_bexpr != "true" && _bexpr != "false") {
									_queryr += zpt::mariadb::escape(_key) + _found->second + zpt::mariadb::escape(_expression);
								}
								else {
									_queryr += zpt::mariadb::escape(_key) + _found->second + _bexpr;
								}
							}
							else {
								_queryr += zpt::mariadb::escape(_key) + _found->second + std::to_string(d);
							}
						}
						else {
							_queryr += zpt::mariadb::escape(_key) + _found->second + std::to_string(i);
						}
					}
					else if (_options == "j") {
					}
					else if (_options == "d") {
						_queryr += zpt::mariadb::escape(_key) + _found->second + std::string("TIMESTAMP('") + zpt::mariadb::escape(_expression) + std::string("')");
					}
					continue;
				}

			}
		}

		_queryr += _key + std::string("=") + zpt::mariadb::escape(_value);
	}
}

auto zpt::mariadb::get_opts(zpt::json _in, std::string&  _queryr) -> void {
	if (!_in->is_object()) {
		return;
	}

	if (_in["page_size"]) {
		_queryr += std::string(" LIMIT ") + std::to_string(int(_in["page_size"]));
	}
	if (_in["page_start_index"]) {
		_queryr += std::string(" OFFSET ") + std::to_string(int(_in["page_start_index"]));
	}
	if (_in["order_by"]) {
		_queryr += std::string(" ORDER BY ");
		std::istringstream lss(((std::string) _in["order_by"]).data());
		std::string _part;
		bool _first = true;
		while (std::getline(lss, _part, ',')) {
			if (_part.length() > 0) {
				std::string _dir = "ASC";
				
				if (_part[0] == '-') {
					_dir = "DESC";
				}
				_part.erase(0, 1);
				_queryr += (!_first ? ", " : "") + zpt::mariadb::escape(_part) + std::string(" ") + _dir;
				_first = false;
			}
		}
	}
	if (_in["fields"]) {
	}
	if (_in["embed"]) {
	}
}

auto zpt::mariadb::get_column_names(zpt::json _document, zpt::json _opts) -> std::string {
	std::string _columns;
	if (_opts["fields"]->ok()) {
		if (!_document->ok()) {
			for (auto _c : _opts["fields"]->arr()){
				if (_columns.length() != 0) {
					_columns += std::string(",");
				}
				_columns += zpt::mariadb::escape_name(std::string(_c));
			}			
		}
		else {
			for (auto _c : _opts["fields"]->arr()){
				if (!_document[std::string(_c)]->ok()) {
					continue;
				}
				if (_columns.length() != 0) {
					_columns += std::string(",");
				}
				_columns += zpt::mariadb::escape_name(std::string(_c));
			}			
		}
	}
	else {
		if (!_document->ok()) {
			return "*";
		}
		for (auto _c : _document->obj()){
			if (_columns.length() != 0) {
				_columns += std::string(",");
			}
			_columns += zpt::mariadb::escape_name(_c.first);
		}
	}
	return _columns;
}

auto zpt::mariadb::get_column_values(zpt::json _document, zpt::json _opts) -> std::string {
	std::string _values;
	if (_opts["fields"]->ok()) {
		if (!_document->ok()) {
			return "";
		}
		else {
			for (auto _c : _opts["fields"]->arr()){
				if (!_document[std::string(_c)]->ok()) {
					continue;
				}
				if (_values.length() != 0) {
					_values += std::string(",");
				}
				_values += zpt::mariadb::escape(_document[std::string(_c)]);
			}			
		}
	}
	else {
		if (!_document->ok()) {
			return "";
		}
		for (auto _c : _document->obj()){
			if (_values.length() != 0) {
				_values += std::string(",");
			}
			std::string _val = zpt::mariadb::escape(_c.second);
			_values += _val;
		}
	}
	return _values;
}

auto zpt::mariadb::escape_name(std::string _in) -> std::string {
	return _in;
}

auto zpt::mariadb::escape(zpt::json _in) -> std::string {
	return std::string(_in);
}
