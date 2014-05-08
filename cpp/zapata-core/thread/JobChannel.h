#pragma once

namespace zapata {

	class JobChannel {
		public:
			JobChannel();
			virtual ~JobChannel();

			virtual void notify(void* _in, void* _out) = 0;

	};
}
