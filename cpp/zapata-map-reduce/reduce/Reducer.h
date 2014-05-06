#pragma once

#include <zapata/core.h>

namespace zapata {

	class Reducer : public Job {
		public:
			Reducer(string _key_file_path);
			virtual ~Reducer();

			virtual void run();
			virtual void reduce() = 0;
			virtual void collect() = 0;

	};
}
