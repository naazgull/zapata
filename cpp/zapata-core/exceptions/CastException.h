#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class CastException: public std::exception {
		private:
			string __what;

		public:
			CastException(string _what);
			virtual ~CastException() throw();

			const char* what();
	};

}
