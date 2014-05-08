#pragma once

#include <algorithm/DirectoryService.h>

namespace zapata {

	class ClusterDirectoryService: public DirectoryService {
		public:
			ClusterDirectoryService(string _key_file_path);
			virtual ~ClusterDirectoryService();

			virtual void run();
	};

}
