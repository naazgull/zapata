#include <text/manip.h>

#include <unistd.h>
#include <iomanip>

using namespace std;
using namespace __gnu_cxx;

void zapata::ltrim(std::string &_in_out) {
        _in_out.erase(_in_out.begin(), std::find_if(_in_out.begin(), _in_out.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void zapata::rtrim(std::string &_in_out) {
        _in_out.erase(std::find_if(_in_out.rbegin(), _in_out.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _in_out.end());
}

void zapata::trim(std::string &_in_out) {
        zapata::ltrim(_in_out);
        zapata::rtrim(_in_out);
}

void zapata::replace(string& str, string find, string replace) {
	if (str.length() == 0) {
		return;
	}

	size_t start = 0;

	while ((start = str.find(find, start)) != string::npos) {
		str.replace(start, find.size(), replace);
		start += replace.length();
	}
}

void zapata::normalize_path(string& _in_out, bool _with_trailing) {
	if (_with_trailing) {
		if (_in_out[_in_out.length() - 1] != '/') {
			_in_out.insert(_in_out.length(), "/");
		}
	}
	else {
		if (_in_out[_in_out.length() - 1] == '/') {
			_in_out.erase(_in_out.length() - 1, 1);
		}
	}
}
