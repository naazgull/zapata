#pragma once

#include <thread/Job.h>

namespace zapata {

	class JobServer : public Job {
		public:
			JobServer(string _key_file_path);
			virtual ~JobServer();

			virtual void run();
			virtual void notify() = 0;

			size_t next();

		private:
			size_t __next;

	};
}
