#pragma once

namespace zapata {

	template <typename I, typename O>
	class Reducer {
		public:
			Reducer();
			virtual ~Reducer();

			virtual void reduce(I _in, O _out) = 0;
	};

}
