#pragma once

#include <zapata/core.h>
#include <algorithm/DirectoryService.h>

namespace zapata {

	class JobServer {
		public:
			JobServer(DirectoryService* _ds);
			virtual ~JobServer();

			virtual void serve() = 0;

		private:
			DirectoryService* __ds;

	};
}
