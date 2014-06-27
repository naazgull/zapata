#pragma once

#include <thread/Job.h>
#include <api/RESTPool.h>
#include <queue>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class RESTJob: public Job {
		public:
			RESTJob(string _key_file_path);
			virtual ~RESTJob();

			virtual void run();
			virtual void assign(int _cs_fd);

			RESTPool& pool();
			void pool(RESTPool* _pool);

		private:
			queue<int> __cur_fd;
			RESTPool* __pool;
	};

}
