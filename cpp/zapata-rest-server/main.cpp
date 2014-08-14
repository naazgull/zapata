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
	char* _conf_file = NULL;

	while ((_c = getopt(argc, argv, "c:")) != -1) {
		switch (_c) {
			case 'c': {
				_conf_file = optarg;
				break;
			}
		}
	}

	zapata::log_fd = &cout;

	if (_conf_file == NULL) {
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
	pthread_key_create(&zapata::__memory_key, NULL);
	pthread_key_create(&zapata::__configuration_key, NULL);

	zapata::RESTServer _server(_conf_file);
	__semid = _server.semid();
	_server.start();

	return 0;
}
