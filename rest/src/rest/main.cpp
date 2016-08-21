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

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

#include <zapata/rest.h>
#include <zapata/mem/usage.h>
#include <semaphore.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

int main(int argc, char* argv[]) {
	try {
		char _c;
		short _log_level = -1;
		char* _conf_file = nullptr;

		while ((_c = getopt(argc, argv, "l:c:")) != -1) {
			switch (_c) {
				case 'c': {
					_conf_file = optarg;
					break;
				}
				case 'l': {
					_log_level = std::stoi(optarg);
					break;
				}
			}
		}

		zpt::log_fd = & cout;
		zpt::log_pid = ::getpid();
		zpt::log_pname = new string(argv[0]);
		zpt::log_lvl = _log_level;

		if (_conf_file == nullptr) {
			zlog("a configuration file must be provided", zpt::error);
			return -1;
		}

		zpt::JSONPtr _ptr;
		{
			ifstream _in;
			_in.open(_conf_file);
			if (!_in.is_open()) {
				zlog("a configuration file must be provided", zpt::error);
				return -1;
			}
			try {
				_in >> _ptr;
			}
			catch(zpt::SyntaxErrorException& _e) {
				std::cout << "syntax error when parsing configuration file: " << _e.what() << endl << flush;
				exit(-10);
			}
		}

		zpt::JSONPtr _to_spawn = zpt::mkobj();
		for (auto _spawn : _ptr->obj()) {
			if (_spawn.second["enabled"]->ok() && !((bool) _spawn.second["enabled"])) {
				continue;
			}
			_to_spawn << _spawn.first << _spawn.second;
		}		
		
		size_t _spawned = 0;
		std::string _name;
		zpt::JSONPtr _options;
		for (auto _spawn : _to_spawn->obj()) {
			if (_spawn.second["enabled"]->ok() && !((bool) _spawn.second["enabled"])) {
				continue;
			}
			if (_spawned == _to_spawn->obj()->size() - 1) {
				_name.assign(_spawn.first);
				_options = _spawn.second;
			}
			else {
				pid_t _pid = fork();
				if (_pid == 0) {
					_name.assign(_spawn.first);
					_options = _spawn.second;
					break;
				}
				else {
					_spawned++;
				}
			}
		}

		size_t _n_workers = _options["workers"]->ok() ? (size_t) _options["workers"] : 1;
		size_t _i = 0;
		for (; _i != _n_workers - 1; _i++) {
			pid_t _pid = fork();
			if (_pid == 0) {
				break;
			}
		}
		_name += std::string("-") + std::to_string(_i + 1);
		zpt::conf::init(_name, _options);
		zpt::RESTServerPtr _server(_name, _options);
		_server->start();
	}
	catch (zpt::AssertionException& _e) {
		zlog(_e.what() + string(" | ") + _e.description(), zpt::emergency);
		exit(-10);
	}
	catch (std::exception& _e) {
		zlog(_e.what(), zpt::emergency);
		exit(-10);
	}
	
	return 0;
}
