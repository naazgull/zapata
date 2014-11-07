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

#define DEBUG 1

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

#include <zapata/core.h>
#include <zapata/http.h>
#include <zapata/net.h>
#include <zapata/rest.h>
#include <zapata/mem/usage.h>

using namespace std;
using namespace __gnu_cxx;

pthread_t __thread_id = 0;
semid_t __semid = 0;

void sigsev(int sig) {
	if (__semid != 0) {
		semctl(__semid, 0, IPC_RMID);
		__semid = 0;
	}
	exit(-1);
}

void stop(int sig) {
	(*zapata::log_fd ) << endl << flush;
	string _text("finishing thread ");
	zapata::tostr(_text, __thread_id);
	_text.insert(_text.length(),  " with semid ");
	zapata::tostr(_text, __semid);
	zapata::log(_text, zapata::sys);
	if (pthread_self() == __thread_id && __semid != 0) {
		semctl(__semid, 0, IPC_RMID);
		__semid = 0;
	}
	exit(SIGTERM);
}

int main(int argc, char* argv[]) {
//	locale loc("");
//	cout.imbue(loc);

	char _c;
	char* _conf_file = nullptr;

	while ((_c = getopt(argc, argv, "c:")) != -1) {
		switch (_c) {
			case 'c': {
				_conf_file = optarg;
				break;
			}
		}
	}

	zapata::log_fd = &cout;
	zapata::log_lvl = 5;

	if (_conf_file == nullptr) {
		zapata::log("a configuration file must be provided", zapata::error);
		return -1;
	}

	// Adds listeners for SIGSEV and SIGABRT.
	{
		signal(SIGPIPE, SIG_IGN );

		struct sigaction action;
		action.sa_handler = sigsev;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		sigaction(SIGSEGV, &action, 0);
		sigaction(SIGABRT, &action, 0);
		sigaction(SIGKILL, &action, 0);
	}

	// Adds listeners for SIGKILL, for gracefull stop
	{
		struct sigaction action;
		action.sa_handler = stop;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		sigaction(SIGTERM, &action, 0);
		sigaction(SIGQUIT, &action, 0);
		sigaction(SIGINT, &action, 0);
	}
	
	__thread_id = pthread_self();
	//pthread_key_create(&zapata::__memory_key, nullptr);
	//pthread_key_create(&zapata::__configuration_key, nullptr);

	/*zapata::log_mem_usage();
	{
		zapata::log_mem_usage();
		{
			for (int _k = 0;  _k != 1000; _k++){
				ifstream _cs;
				_cs.open(_conf_file);
				zapata::JSONPtr _config = zapata::fromfile(_cs);
				cout << *_config << endl << flush;
				_cs.close();
			}
		}
		zapata::log_mem_usage();

		zapata::JSONObj _obj3;
		zapata::JSONObj _obj2;
		zapata::log_mem_usage();
		{
			_obj2 << "a" << JSON( "a" << 1 << "b" << time(nullptr) << "c" << "something really special");
			zapata::JSONObj a = (zapata::JSONObj&) _obj2["a"];
			cout << a << endl << (string) a["b"] << endl << flush;
			_obj3 << "mya" << (zapata::JSONObj&) _obj2["a"];
			zapata::log_mem_usage();
		}
		zapata::log_mem_usage();
		zapata::JSONArr _arr;
		_arr << _obj2 << _obj2 << _obj3;

		cout << "done adding" << endl << flush;
		cout << _arr << endl << flush;

		_obj2 >> "a";
		_arr >> 0;

		cout << _arr << endl << flush;

		for (int _k = 0;  _k != 1000; _k++){
			zapata::JSONObj _options = JSON( 
				"constants" << JSON( 
					"TiU" << 10 <<
					"StMR" << JSON_ARRAY( 10 << 2 << 4 ) << 
					"FdMR" << JSON_ARRAY( 1 << 5 ) <<
					"NoCT" << 0.7 << 
					"AsRT" << 1 << 
					"AcERR" << 1 << 
					"WmF" << JSON_ARRAY( 3 <<  100 ) <<
					"ExD" << 2
					) << 
				"variables" << JSON(
					"MATCHING" << JSON(
						"identifiers" << JSON_ARRAY( "terms" << "roles" << "context" ) <<
						"exclusive" << JSON( 
							"roles" << JSON_ARRAY( "roles.sender.type" << "roles.receiver.type" ) <<
							"terms" << JSON_ARRAY( "a" << "d.p.io" << "d.p.component" << "d.p.property" ) 
							) 
						) <<
					"CREATING" << JSON(
						"identifier" << "terms"
						) <<
					"FETCHING" << JSON(
						"identifier" << "roles.sender.type" << 
						"sequence" << JSON_ARRAY( "sensor" << "actuator" ) 
						)
					) <<
				"mongo" << JSON( 
					"host" << "localhost" <<
					"db" << "muzzley_profiler"
					)
				);
			cout << _options << endl << flush;
		}
	}
	zapata::log_mem_usage();
	return 0;*/

	zapata::RESTServer _server(_conf_file);
	__semid = _server.semid();
	_server.start();

	return 0;
}
