/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
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
