#pragma once

#include <string.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void ltrim(std::string &_in_out);
	void rtrim(std::string &_in_out);
	void trim(std::string &_in_out);
	void replace(string& str, string find, string replace);

}
