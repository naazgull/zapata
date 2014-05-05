#pragma once

namespace zapata {

	class JobChannel {
		public:
			JobChannel();
			virtual ~JobChannel();

			virtual void send(void* _in) = 0;
			virtual void* receive() = 0;

	};
}
