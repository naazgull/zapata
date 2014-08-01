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

	void normalize_path(string& _in_out, bool _with_trailing);

	void cipher(string _in, string _key, string& _out);
	void decipher(string _in, string _key, string& _out);
	void encrypt(string& _out, string _in, string _key);
	void decrypt(string& _out, string _in, string _key);
}
