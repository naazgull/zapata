#pragma once

#include <zapata/core.h>

namespace zapata {

	class MapReduce : public Job {
		public:
			MapReduce();
			virtual ~MapReduce();

			virtual void run();

	};
}
