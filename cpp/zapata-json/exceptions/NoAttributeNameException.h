#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class NoAttributeNameException: public std::exception {
		private:
			string __what;

		public:
			NoAttributeNameException(string _what);
			virtual ~NoAttributeNameException() throw();

			const char* what();
	};

}
