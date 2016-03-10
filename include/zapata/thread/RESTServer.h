/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <zapata/stream/SocketStreams.h>
#include <zapata/thread/RESTJob.h>
#include <zapata/api/RESTPool.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zapata {

	class RESTServer {
		public:
			RESTServer(zapata::JSONObj& _options);
			virtual ~RESTServer();
	
			virtual void start();
			virtual void wait();
			virtual void notify();

			zapata::JSONObj& options();
			size_t max();
			size_t next();
			void max(size_t _max);

			zapata::RESTPoolPtr pool();

		private:
			zapata::serversocketstream __ss;
			std::vector< zapata::RESTJob * > __jobs;
			zapata::RESTPoolPtr __pool;
			bool __initialized;
			
		protected:
			zapata::JSONObj __options;
			size_t __next;
			size_t __max_idx;
	};

}
