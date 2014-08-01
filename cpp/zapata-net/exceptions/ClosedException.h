#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class ClosedException: public std::exception {
		private:
			string __what;

		public:
			ClosedException(string _what);
			virtual ~ClosedException() throw();

			const char* what();
	};

}
