#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class SyntaxErrorException: public std::exception {
		private:
			string __what;

		public:
			SyntaxErrorException(string _what);
			virtual ~SyntaxErrorException() throw();

			const char* what();
	};

}
