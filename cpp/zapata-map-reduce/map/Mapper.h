#pragma once

namespace zapata {

	template <typename I, typename O>
	class Mapper {
		public:
			Mapper();
			virtual ~Mapper();

			virtual void map(I _in, O _out) = 0;
	};
}
