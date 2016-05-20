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
	short _log_level = 7;
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

	zapata::log_fd = & cout;
	zapata::log_pid = ::getpid();
	zapata::log_pname = new string(argv[0]);
	zapata::log_lvl = _log_level;

	if (_conf_file == nullptr) {
		zlog("a configuration file must be provided", zapata::error);
		return -1;
	}

	zapata::JSONPtr _ptr;
	{
		ifstream _in;
		_in.open(_conf_file);
		if (!_in.is_open()) {
			zlog("a configuration file must be provided", zapata::error);
			return -1;
		}

		_in >> _ptr;
		zapata::dirs("/etc/zapata/conf.d/", _ptr);
		zapata::env(_ptr);
	}

	try {
		zapata::RESTClientPtr _api(_ptr);
		size_t _max = 50100;
		size_t * _n = new size_t();
		_api->emitter()->on(zapata::ev::Reply, "/api/0.9/users", [ & ] (zapata::ev::Performative _performative, std::string _resource, zapata::JSONPtr _envelope, zapata::EventEmitterPtr _events) -> zapata::JSONPtr {
			(* _n)++;
			zlog(std::to_string(* _n), zapata::debug);
			if ((* _n) == (_max - 100)) {
				cout << "PROCESSED " << (* _n) << " MESSAGES" << endl << flush;
				exit(0);
			}
			return zapata::undefined;
		});
		zapata::ZMQPtr _client = _api->bind("zmq");
		zapata::JSONPtr _message = zapata::mkptr(JSON(
			"name" << "m/@gmail.com/i"
		));
		zapata::Job _sender([ & ] (zapata::Job& _job) -> void {
			for (size_t _k = 0; _k != _max; _k++) {
				_client->send(zapata::ev::Get, "/api/0.9/users", _message);
			}
			if (_ptr["zmq"]["type"]->str() == "req/rep") {
				cout << "PROCESSED " << (_max) << " MESSAGES" << endl << flush;
				exit(0);				
			}
		});
		_sender->start();
		_api->start();
	}
	catch (zapata::AssertionException& _e) {
		zlog(_e.what() + string("\n") + _e.description(), zapata::error);
	}
	
	return 0;
}
