#pragma once

#include <zapata/core.h>

namespace zapata {

	class Partitioner : public Job {
		public:
			Partitioner(string _key_file_path);
			virtual ~Partitioner();

			virtual void run();
			virtual void divide() = 0;
			virtual void collect() = 0;
	};
}

