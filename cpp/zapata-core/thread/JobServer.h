#pragma once

#include <thread/Job.h>
#include <parsers/json.h>

namespace zapata {

	class JobServer {
		public:
			JobServer(string _key_file_path);
			virtual ~JobServer();

			virtual void start();
			virtual void notify() = 0;
			virtual void wait() = 0;

			size_t max();
			size_t next();
			semid_t semid();
			void max(size_t _max);
			zapata::JSONObj& configuration();

		private:
			size_t __next;
			size_t __max_idx;
			semid_t __sem;
			JSONObj __configuration;

		protected:
			string __skey;
	};
}
