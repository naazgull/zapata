#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {
	bool mkdir_recursive(string _name, mode_t _mode);

	bool copy_path(string _from, string _to);
	bool move_path(string _from, string _to);

	void get_mime(string _in, string& _out);
	bool path_exists(string _in);
}
