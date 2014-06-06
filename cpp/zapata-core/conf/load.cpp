#include <conf/load.h>

bool zapata::to_configuration(zapata::JSONObj& _out, string& _key_file_path) {
	ifstream _global_file;
	_global_file.open("/etc/zapata/zapata.conf");
	if (_global_file.is_open()) {
		zapata::fromfile(_global_file, _out);
	}



	ifstream _key_file;
	_key_file.open(_key_file_path.data());
	if (!_key_file.is_open()) {
		return false;
	}
	zapata::fromfile(_key_file, _out);
	return false;
}
