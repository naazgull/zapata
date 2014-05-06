#pragma once

#include <zapata/core.h>

namespace zapata {

	class Mapper : public Job {
		public:
			Mapper(string _key_file_path);
			virtual ~Mapper();

			virtual void run();
			virtual void map() = 0;
			virtual void collect() = 0;

	};
}
