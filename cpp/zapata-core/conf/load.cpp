#include <conf/load.h>

bool zapata::to_configuration(zapata::JSONObj& _out, string& _key_file_path) {
	ifstream _global_file;
	_global_file.open("/etc/zapata/zapata.conf");
	if (_global_file.is_open()) {
		zapata::fromfile(_global_file, _out);
	}

	DIR *_dp = NULL;
	struct dirent *_dirp = NULL;

	if ((_dp = opendir("/etc/zapata/conf.d/")) != NULL) {
		while ((_dirp = readdir(_dp)) != NULL) {
			string _cname = string(_dirp->d_name);
			if (_cname.find('.') != 0 && _dirp->d_type == DT_REG) {
				_cname.insert(0, "/etc/zapata/conf.d/");
				ifstream _confd_file;
				_confd_file.open(_cname.data());
				if (_confd_file.is_open()) {
					zapata::fromfile(_confd_file, _out);
				}
			}
		}
		closedir(_dp);
	}

	ifstream _key_file;
	_key_file.open(_key_file_path.data());
	if (!_key_file.is_open()) {
		return false;
	}
	zapata::fromfile(_key_file, _out);
	return false;
}
