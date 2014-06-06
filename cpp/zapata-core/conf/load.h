#pragma once

#include <file/manip.h>
#include <parsers/json.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	bool to_configuration(zapata::JSONObj& _out, string& _key_file_path);
}
