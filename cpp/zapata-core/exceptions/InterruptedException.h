#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class InterruptedException: public std::exception {
		private:
			string __what;

		public:
			InterruptedException(string _what);
			virtual ~InterruptedException() throw();

			const char* what();
	};

}
