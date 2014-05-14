#pragma once

#include <thread/JobServer.h>
#include <stream/SocketStreams.h>
#include <thread/RESTJob.h>
#include <api/RESTPool.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class RESTServer: public JobServer {
		public:
			RESTServer(string _key_file_path);
			virtual ~RESTServer();

			virtual void wait();
			virtual void notify();

			RESTPool& pool();

		private:
			size_t __n_jobs;
			serversocketstream __ss;
			vector<RESTJob*> __jobs;
			RESTPool __pool;
	};

}
