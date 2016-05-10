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
	char* _conf_file = nullptr;

	while ((_c = getopt(argc, argv, "kc:")) != -1) {
		switch (_c) {
			case 'c': {
				_conf_file = optarg;
				break;
			}
		}
	}

	zapata::log_fd = & cout;
	zapata::log_pid = ::getpid();
	zapata::log_pname = new string(argv[0]);
	zapata::log_lvl = 8;

	if (_conf_file == nullptr) {
		zlog("a configuration file must be provided", zapata::error);
		return -1;
	}

	zapata::JSONObj _options;
	{
		ifstream _in;
		_in.open(_conf_file);
		if (!_in.is_open()) {
			zlog("a configuration file must be provided", zapata::error);
			return -1;
		}

		zapata::JSONPtr _ptr;
		_in >> _ptr;
		zapata::dirs("/etc/zapata/conf.d/", _ptr);
		zapata::env(_ptr);
		_options = (zapata::JSONObj&) _ptr;
	}

	try {
		zapata::ZMQPollPtr _poll(_options);
		zapata::ZMQPtr _client = _poll->borrow(ZMQ_REQ, "tcp://localhost:9999");
		_client->send(zapata::make_ptr(JSON(
			"performative" << zapata::ev::Get <<
			"resource" << "/api/0.9/users" <<
			"payload" << JSON(
				"name" << "m/@gmail.com/i"
			)
		)));
		_poll->loop();
	}
	catch (zapata::AssertionException& _e) {
		zlog(_e.what() + string("\n") + _e.description(), zapata::error);
	}
	
	return 0;
}
