#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class ParserEOF: public std::exception {
		private:
			string __what;

		public:
			ParserEOF(string _what);
			virtual ~ParserEOF() throw();

			const char* what();
	};

}
