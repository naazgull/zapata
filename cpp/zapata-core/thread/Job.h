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

#pragma once

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string>
#include <json/JSONObj.h>

using namespace std;
using namespace __gnu_cxx;

typedef int semid_t;

namespace zapata {

	extern pthread_key_t __configuration_key;

	class Job {
		public:
			Job(string _key_file_path);
			virtual ~Job();

			static void* start(void* thread);

			size_t pending();
			virtual void start();
			virtual void run() = 0;
			virtual void assign();
			virtual void wait(int seconds);
			virtual void wait();

			size_t idx();
			size_t max();
			semid_t semid();
			zapata::JSONObj& configuration();

			void idx(size_t _idx);
			void max(size_t _max);

		private:
			size_t __idx;
			size_t __max_idx;
			semid_t __sem;

		protected:
			string __skey;
			pthread_mutex_t* __mtx;
			pthread_t* __thr;
			pthread_mutexattr_t __attr;
			JSONObj __configuration;
	};

}
