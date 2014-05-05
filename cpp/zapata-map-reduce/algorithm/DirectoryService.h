#pragma once

#include <partition/Partitioner.h>
#include <map/Mapper.h>
#include <reduce/Reducer.h>

namespace zapata {

	class DirectoryService {
		public:
			DirectoryService();
			virtual ~DirectoryService();

			virtual void signalPartioners() = 0;
			virtual void signalMappers() = 0;
			virtual void signalReducers() = 0;

	};

}
