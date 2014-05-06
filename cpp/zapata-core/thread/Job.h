#pragma once

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string>
#include <thread/JobChannel.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

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

			virtual JobChannel* channel() = 0;
			size_t idx();
			size_t max();
			int semid();

			void idx(size_t _idx);
			void max(size_t _max);

		private:
			size_t __idx;
			size_t __max_idx;
			int __sem;

		protected:
			string __skey;
			pthread_mutex_t* __mtx;
			pthread_t* __thr;
			pthread_mutexattr_t __attr;
	};

}
