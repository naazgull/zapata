#pragma once

#include <thread/Job.h>

namespace zapata {

	class RESTJob: public Job {
		public:
			RESTJob(string _key_file_path);
			virtual ~RESTJob();

			virtual void run();
			virtual void assign(int _cs_fd);

		private:
			int __cur_fd;
	};

}
