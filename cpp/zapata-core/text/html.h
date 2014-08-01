#pragma once

#include <string.h>
#include <stdio.h>
#include <string>
#include <wchar.h>
#include <wctype.h>
#include <stdint.h>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <memory.h>
#include <algorithm>
#include <fstream>
#include <text/manip.h>
#include <json/JSONObj.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void html_entities_encode(wstring s, ostream& out, bool quote = true, bool tags = false);
	void html_entities_encode(string& out, bool quote = true, bool tags = false);
	void html_entities_decode(string& out);

	void content_boundary(string& _in, string& _out);

	void fromformurlencoded(string& _in, JSONObj& _out);
	void fromformdata(string& _in, string _boundary, string _tmp_path, JSONObj& _out);


}
