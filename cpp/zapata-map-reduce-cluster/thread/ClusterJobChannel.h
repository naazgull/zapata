#pragma once

#include <zapata/core.h>
#include <zapata/mapreduce.h>
#include <zapata/net.h>
#include <zapata/http.h>
#include <thread/JobChannel.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class ClusterJobChannel: public JobChannel {
		public:
			ClusterJobChannel(string _address, uint16_t _port);
			virtual ~ClusterJobChannel();

			virtual void notify(void* _in, void* _out);

		private:
			socketstream __stream;
	};

}
