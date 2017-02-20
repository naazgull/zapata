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

#include <zapata/json/json.h>
#include <zapata/log/log.h>
#include <zapata/file/manip.h>
#include <zapata/exceptions/SyntaxErrorException.h>
#include <regex>

auto zpt::split(std::string _to_split, std::string _separator, bool _trim) -> zpt::json {
	std::istringstream _iss(_to_split);
	std::string _part;
	zpt::json _ret = zpt::json::array();
	while(_iss.good()) {
		std::getline(_iss, _part, _separator[0]);
		if (_part.length() != 0) {
			if (_trim) {
				zpt::trim(_part);
			}
			_ret << _part;
		}
	}
	return _ret;
}

auto zpt::join(zpt::json _to_join, std::string _separator) -> std::string {
	assertz(_to_join->type() == zpt::JSArray || _to_join->type() == zpt::JSObject, "JSON to join must be an array", 412, 0);
	std::string _return;
	if (_to_join->type() == zpt::JSArray) {
		for (auto _a : _to_join->arr()) {
			if (_return.length() != 0) {
				_return += _separator;
			}
			_return += ((std::string) _a);
		}
	}
	else if (_to_join->type() == zpt::JSObject) {
		for (auto _a : _to_join->obj()) {
			if (_return.length() != 0) {
				_return += _separator;
			}
			_return += _a.first + _separator + ((std::string) _a.second);
		}
	}
	return _return;
}

auto zpt::path::split(std::string _to_split) -> zpt::json {
	return zpt::split(_to_split, "/", true);
}

auto zpt::path::join(zpt::json _to_join) -> std::string {
	return std::string("/") + zpt::join(_to_join, "/");
}

auto zpt::conf::getopt(int _argc, char* _argv[]) -> zpt::json {
	zpt::json _files = zpt::json::array();
	zpt::json _return = { "files", _files };
	std::string _last("");
	for (int _i = 1; _i != _argc; _i++) {
		std::string _arg(_argv[_i]);
		if (_arg.find("--") == 0) {
			if (_last.length() != 0) {
				_return << std::string(_last.data()) << true;
			}
			_arg.erase(0, 2);
			_last.assign(_arg);
		}
		else if (_arg.find("-") == 0) {
			if (_last.length() != 0) {
				_return << std::string(_last.data()) << true;
			}
			_arg.erase(0, 1);
			_last.assign(_arg);
		}
		else {
			if (_last.length() != 0) {
				if (!_return[_last]->ok()) {
					_return << std::string(_last.data()) << zpt::json::array();
				}
				_return[_last] << _arg;
				_last.assign("");
			}
			else {
				_files << _arg;
			}
		}
	}
	if (_last.length() != 0) {
		_return << std::string(_last.data()) << true;
	}
	return _return;
}

auto zpt::conf::setup(zpt::json _options) -> void {
	zpt::conf::dirs(_options);
	zpt::conf::env(_options);
}

auto zpt::conf::dirs(std::string _dir, zpt::json _options) -> void {
	std::vector<std::string> _files;
	if (zpt::is_dir(_dir)) {
		zpt::glob(_dir, _files, "(.*)\\.conf");
	}
	else {
		_files.push_back(_dir);
	}
	for (auto _file : _files) {
		zpt::json _conf;
		std::ifstream _ifs;
		_ifs.open(_file.data());
		try {
			_ifs >> _conf;
		}
		catch(zpt::SyntaxErrorException& _e) {
			_conf = zpt::undefined;
		}
		_ifs.close();

		assertz(_conf->ok(), std::string("syntax error parsing file: ") + _file, 500, 0);

		for (auto _new_field : _conf->obj()) {
			_options << _new_field.first << (_options[_new_field.first] + _new_field.second);
		}
	}
}

auto zpt::conf::dirs(zpt::json _options) -> void {
	bool* _redo = new bool(false);
	do {
		*_redo = false;
		zpt::json _traversable = _options->clone();
		_traversable->inspect({ "$any", "type" },
			[ & ] (std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
				if (_key == "$include") {
					zpt::json _object = (_object_path.rfind(".") != std::string::npos ? _options->getPath(_object_path.substr(0, _object_path.rfind("."))) : _options);
					zpt::json _to_include = _options->getPath(_object_path);
					if (_to_include->type() == zpt::JSArray) {
						for (auto _file : _to_include->arr()) {
							zpt::conf::dirs((std::string) _file, _object);
						}
					}
					else {
						zpt::conf::dirs((std::string) _to_include, _object);
					}
					_object >> "$include";
					*_redo = true;
				}
			}
		);
	}
	while(*_redo);
	delete _redo;
}

auto zpt::conf::env(zpt::json _options) -> void {
	zpt::json _traversable = _options->clone();
	_traversable->inspect({ "$regexp", "([\"])(.*)([$])([{])([^}]+)([}])(.*)([\"])" },
		[ & ] (std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
			std::string _value = std::string(_options->getPath(_object_path));
			std::string _found = std::string(_value.data());

			for (size_t _idx = _found.find("$"); _idx != std::string::npos; _idx = _found.find("$", _idx + 1)) {
				std::string _var = _found.substr(_idx + 2, _found.find("}", _idx) - _idx - 2);
				
				const char * _var_val = std::getenv(_var.data());
				if (_var_val != nullptr) {
					zpt::replace(_value, std::string("${") + _var + std::string("}"), zpt::r_trim(_var_val));
				}
			}
			_options->setPath(_object_path, zpt::json::string(_value));
		}
	);
}

auto zpt::uri::parse(std::string _uri) -> zpt::json {
	if (_uri.find(":") >= _uri.find("/")) {
		_uri = std::string("zpt://127.0.0.1") + _uri;
	}
	static const std::regex _uri_rgx(
		"[@>]{0,1}([a-zA-Z][a-zA-Z0-9+.-]*):"  // scheme:
		"([/]{1,2})([^/]+)"           // authority
		"([^?#]*)"                    // path
		"(?:\\?([^#]*))?"             // ?query
		"(?:#(.*))?"		      // #fragment
	);
	
	std::smatch _uri_matches;
	std::regex_match(_uri, _uri_matches, _uri_rgx);

	std::string _q_str = (std::string) _uri_matches[5];
	zpt::json _query = zpt::uri::query::parse(_q_str);

	return {
		"scheme", (std::string) _uri_matches[1],
		"authority", (std::string) _uri_matches[3],
		"path", (std::string) _uri_matches[4],
		"query", (_query->obj()->size() != 0 ? _query : zpt::undefined),
		"fragment", zpt::url::r_decode((std::string) _uri_matches[6])
	};
}

auto zpt::uri::query::parse(std::string _query) -> zpt::json {
	static const std::regex _rgx(
		"(^|&)" 			//start of query or start of parameter "&"
		"([^=&]*)=?" 			//parameter name and "=" if value is expected
		"([^=&]*)" 			//parameter value
		"(?=(&|$))" 			//forward reference, next should be end of query or start of next parameter
	);

	zpt::json _return = zpt::json::object();
	auto _begin = std::sregex_iterator(_query.begin(), _query.end(), _rgx);
	auto _end = std::sregex_iterator();
	for (auto _i = _begin; _i != _end; ++_i) {
		std::smatch _match = *_i;
		_return << (std::string) _match[2] << zpt::url::r_decode((std::string) _match[3]);
	}
	
	return _return;
}

auto zpt::test::location(zpt::json _location) -> bool {
	return _location->type() == zpt::JSObject && _location["longitude"]->type() == zpt::JSDouble && _location["latitude"]->type() == zpt::JSDouble;
}
