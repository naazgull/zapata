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

zpt::json zpt::split(std::string _to_split, std::string _separator) {
	std::istringstream _iss(_to_split);
	std::string _part;
	zpt::json _ret = zpt::json::array();
	while(_iss.good()) {
		std::getline(_iss, _part, _separator[0]);
		if (_part.length() != 0) {
			_ret << _part;
		}
	}
	return _ret;
}

std::string zpt::join(zpt::json _to_join, std::string _separator) {
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

zpt::json zpt::path::split(std::string _to_split) {
	return zpt::split(_to_split, "/");
}

std::string zpt::path::join(zpt::json _to_join) {
	return std::string("/") + zpt::join(_to_join, "/");
}

zpt::json zpt::conf::init(int argc, char* argv[]) {
	char _c;
	short _log_level = -1;
	char* _conf_file = nullptr;
	std::string _bind;
	zpt::ev::performative _method = zpt::ev::Get;
	std::string _url;
	std::string _token;
	short _type = 0;
	zpt::json _body;
	bool _verbose = false;
	zpt::log_format = true;

	while ((_c = getopt(argc, argv, "vc:l:b:m:u:a:t:j:r")) != -1) {
		switch (_c) {
			case 'c': {
				_conf_file = optarg;
				break;
			}
			case 'l': {
				_log_level = std::stoi(optarg);
				break;
			}
			case 'v': {
				_verbose = true;
				break;
			}
			case 'b': {
				_bind.assign(string(optarg));
				if (_bind[0] != '@' && _bind[0] != '>') {
					_bind = std::string(">") + _bind;
				}
				break;
			}
			case 'm': {
				_method = zpt::ev::from_str(string(optarg));
				break;
			}
			case 'u': {
				_url.assign(string(optarg));
				break;
			}
			case 'a': {
				_token.assign(string(optarg));
				break;
			}
			case 't': {
				std::string _str_type(optarg);
				std::transform(_str_type.begin(), _str_type.end(), _str_type.begin(), ::toupper);
				if (_str_type == "ROUTER/DEALER"){
					_type = -3;
				}
				else if (_str_type == "REQ"){
					_type = 3;
				}
				else if (_str_type == "REP"){
					_type = 4;
				}
				else if (_str_type == "PUB/SUB"){
					_type = -1;
				}
				else if (_str_type == "XPUB/XSUB"){
					_type = -2;
				}
				else if (_str_type == "PUB"){
					_type = 1;
				}
				else if (_str_type == "SUB"){
					_type = 2;
				}
				else if (_str_type == "PUSH"){
					_type = 8;
				}
				else if (_str_type == "PULL"){
					_type = 7;
				}
				break;
			}
			case 'j': {
				_body = zpt::json(string(optarg));
				break;
			}
			case 'r': {
				zpt::log_format = false;
				break;
			}
		}
	}

	zpt::log_fd = & cout;
	zpt::log_pid = ::getpid();
	zpt::log_pname = new string(argv[0]);
	zpt::log_lvl = (_verbose ? 7 : _log_level);

	zpt::json _ptr;
	if (_conf_file == nullptr) {
		_ptr = zpt::json::object();
		std::string _name(argv[0], strlen(argv[0]));
		_ptr << _name << zpt::json(
			{
				"rest", {
					"method", _method
				},
				"zmq", {
					"bind", _bind,
					"type", _type					
				}
			}
		);
		if (_url.length() != 0) {
			_ptr[_name]["rest"] << "target" << _url;
		}
		if (_token.length() != 0) {
			_ptr[_name]["rest"] << "token" << _token;
		}
		if (_body->ok()) {
			_ptr[_name]["rest"] << "body" << _body;
		}
	}
	else {
		std::ifstream _in;
		_in.open(_conf_file);
		if (!_in.is_open()) {
			zlog("unable to start client: a valid configuration file must be provided", zpt::error);
			exit(-10);
		}
		try {
			_in >> _ptr;
		}
		catch(zpt::SyntaxErrorException& _e) {
			std::cout << "unable to start client: syntax error when parsing configuration file: " << _e.what() << endl << flush;
			exit(-10);
		}
	}

	if (zpt::log_lvl == -1 && _ptr["log"]["level"]->ok()) {
		zpt::log_lvl = (int) _ptr["log"]["level"];
	}
	if (zpt::log_lvl == -1) {
		zpt::log_lvl = 4;
	}
	if (!!_ptr["log"]["file"]) {
		zpt::log_fd = new std::ofstream();
		((std::ofstream *) zpt::log_fd)->open(((std::string) _ptr["log"]["file"]).data());
	}

	return _ptr;
}

void zpt::conf::setup(zpt::json _options) {
	zpt::conf::dirs(_options);
	zpt::conf::env(_options);
	if (_options["log"]->ok()) {
		if (_options["log"]["file"]->ok()) {
			zpt::log_fd = new ofstream();
			std::string _log_file((std::string) _options["log"]["file"]);
			((std::ofstream *) zpt::log_fd)->open(_log_file.data(), (std::ios_base::out | std::ios_base::app) & ~std::ios_base::ate);
		}
		if (zpt::log_lvl == -1 && _options["log"]["level"]->ok()) {
			zpt::log_lvl = (int) _options["log"]["level"];
		}
	}
	if (zpt::log_lvl == -1) {
		zpt::log_lvl = 4;
	}
}

void zpt::conf::dirs(std::string _dir, zpt::json _options) {
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

void zpt::conf::dirs(zpt::json _options) {
	zpt::json _traversable = _options->clone();
	_traversable->inspect({ "$regexp", "(.*)" },
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
			}
		}
	);
}

void zpt::conf::env(zpt::json _options) {
	zpt::json _traversable = _options->clone();
	_traversable->inspect({ "$regexp", "\\$\\{([^}]+)\\}" }, [ & ] (std::string _object_path, std::string _key, zpt::JSONElementT& _parent) -> void {
		std::string _var = _options->getPath(_object_path);
		_var = _var.substr(2, _var.length() - 3);
		const char * _valuec = std::getenv(_var.data());
		if (_valuec != nullptr) {
			std::string _value(_valuec);
			_options->setPath(_object_path, zpt::mkptr(_value));
		}
	});
}

