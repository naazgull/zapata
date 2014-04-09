#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class NoHeaderNameException: public std::exception {
		private:
			string __what;

		public:
			NoHeaderNameException(string _what);
			virtual ~NoHeaderNameException() throw();

			const char* what();
	};

}
