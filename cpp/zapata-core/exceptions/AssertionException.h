#pragma once

#include <exception>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	class AssertionException: public std::exception {
		private:
			string __what;
			int __http_code;
			int __code;
			string __description;
			int __line;
			string __file;

		public:
			AssertionException(string _what,  int _http_code, int _code, string _desc, int _line, string _file);
			virtual ~AssertionException() throw();

			const char* what();
			const char* description();
			int code();
			int status();
	};

}
