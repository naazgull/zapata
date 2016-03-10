/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <zapata/conf/load.h>

bool zapata::to_configuration(zapata::JSONObj& _out, string& _key_file_path) {
	/*ifstream _global_file;
	_global_file.open("/etc/zapata/zapata.conf");
	if (_global_file.is_open()) {
		_out = (zapata::JSONObj&) zapata::fromfile(_global_file);
	}

	DIR *_dp = nullptr;
	struct dirent *_dirp = nullptr;

	if ((_dp = opendir("/etc/zapata/conf.d/")) != nullptr) {
		while ((_dirp = readdir(_dp)) != nullptr) {
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
	}*/

	ifstream _key_file;
	_key_file.open(_key_file_path.data());
	if (!_key_file.is_open()) {
		return false;
	}
	_out = (zapata::JSONObj&) zapata::fromfile(_key_file);
	return false;
}
