/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

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
