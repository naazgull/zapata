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

		_in >> _ptr;
		zpt::conf::dirs(_ptr);
		zpt::conf::env(_ptr);
	}


	if (_ptr["log"]->ok()) {
		if (_ptr["log"]["file"]->ok()) {
			zpt::log_fd = new ofstream();
			string _log_file((string) _ptr["log"]["file"]);
			((std::ofstream *) zpt::log_fd)->open(_log_file.data(), (std::ios_base::out | std::ios_base::app) & ~std::ios_base::ate);
		}
		if (zpt::log_lvl == -1 && _ptr["log"]["level"]->ok()) {
			zpt::log_lvl = (int) _ptr["log"]["level"];
		}
	}
	if (zpt::log_lvl == -1) {
		zpt::log_lvl = 4;
	}

	try {
		zlog(std::string("starting 0mq RESTful server for ") + zpt::pretty(_ptr["zmq"]), zpt::alert);
		zpt::RESTServerPtr _server(_ptr);
		_server->start();
		zlog("exiting", zpt::info);
	}
	catch (zpt::AssertionException& _e) {
		zlog(_e.what() + string("\n") + _e.description(), zpt::error);
	}
	
	return 0;
}
