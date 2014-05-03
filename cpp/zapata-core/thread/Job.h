#pragma once

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class Job {
		public:
			Job(string _key_file_path);
			virtual ~Job();

			static void* start(void* thread);

			size_t pending();
			void start();
			virtual void run() = 0;
			void assign();
			void wait(int seconds);
			void wait();

			size_t idx();
			size_t max();
			int semid();

			void idx(size_t _idx);
			void max(size_t _max);

		private:
			size_t __idx;
			size_t __max_idx;
			int __sem;
			string __skey;
			pthread_mutex_t* __mtx;
			pthread_t* __thr;
			pthread_mutexattr_t __attr;
	};

}
