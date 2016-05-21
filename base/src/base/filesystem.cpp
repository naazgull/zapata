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

#include <zapata/base/assert.h>
#include <zapata/file/manip.h>

#include <fcntl.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/sendfile.h>

int zpt::ls(string dir, std::vector<string>& result, bool recursive) {
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dir.c_str())) == NULL) {
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		string cname = string(dirp->d_name);
		if (cname.find('.') != 0) {
			cname.insert(0, "/");
			cname.insert(0, dir);
			result.push_back(cname);
			if (recursive) {
				zpt::ls(cname, result, true);
			}

		}
	}

	closedir(dp);
	return 0;
}

bool zpt::mkdir_recursive(string _name, mode_t _mode) {
	istringstream _iss(_name);
	string _line;
	int _count = 0;
	ostringstream _dname;

	while (_iss.good()) {
		std::getline(_iss, _line, '/');
		string cname(_line.data());
		_dname << cname << flush;

		if (mkdir(_dname.str().data(), 0777) == 0) {
			_count++;
		}
		_dname << "/" << flush;
	}
	return _count != 0;
}

bool zpt::copy_path(string _from, string _to) {
	int _read_fd;
	int _write_fd;
	struct stat _stat_buf;

	_read_fd = open(_from.c_str(), O_RDONLY);
	if (_read_fd < 0) {
		return _read_fd;
	}
	fstat(_read_fd, &_stat_buf);
	_write_fd = open(_to.c_str(), O_WRONLY | O_CREAT, _stat_buf.st_mode);
	int _error = sendfile(_write_fd, _read_fd, nullptr, _stat_buf.st_size);
	close(_read_fd);
	close(_write_fd);

	return _error != -1;
}

bool zpt::move_path(string _from, string _to) {
	if (zpt::copy_path(_from, _to)) {
		return std::remove(_from.c_str()) != 0;
	}
	return false;
}

bool zpt::load_path(string _in, string& _out) {
	ifstream _ifs;
	_ifs.open(_in.data());

	if (_ifs.is_open()) {
		while(_ifs >> _out);
		_ifs.close();
		return true;
	}
	return false;
}

bool zpt::load_path(string _in, wstring& _out) {
	wifstream _ifs;
	_ifs.open(_in.data());

	if (_ifs.is_open()) {
		while(_ifs >> _out);
		_ifs.close();
		return true;
	}
	return false;
}


bool zpt::dump_path(string _in, string& _content) {
	ofstream _ofs;
	_ofs.open(_in.data());
	_ofs << _content << flush;
	_ofs.flush();
	_ofs.close();
	return true;
}

bool zpt::dump_path(string _in, wstring& _content) {
	wofstream _ofs;
	_ofs.open(_in.data());
	_ofs << _content << flush;
	_ofs.flush();
	_ofs.close();
	return true;
}

int zpt::globRegexp(string& dir, vector<string>& result, regex_t& pattern, bool recursive) {
	DIR *dp;
	struct dirent *dirp;
	vector<string> torecurse;

	if ((dp = opendir(dir.c_str())) == NULL) {
		return errno;
	}
	while ((dirp = readdir(dp)) != NULL) {
		string cname = string(dirp->d_name);
		if (cname.find('.') != 0) {
			cname.insert(0, "/");
			cname.insert(0, dir.data());
			if (recursive && dirp->d_type == 4 && cname != dir) {
				torecurse.push_back(cname);
			}
			if (regexec(&pattern, dirp->d_name, (size_t) (0), NULL, 0) == 0) {
				result.insert(result.begin(), cname);
			}

		}
	}
	closedir(dp);

	for (auto i : torecurse) {
		zpt::globRegexp(i, result, pattern, true);
	}

	return 0;
}

int zpt::glob(string& dir, vector<string>& result, string pattern, bool recursive) {
	regex_t regexp;
	assertz(regcomp(& regexp, pattern.data(), REG_EXTENDED | REG_NOSUB) == 0, "the regular expression is not well defined.", 500, 0);
	int _return = zpt::globRegexp(dir, result, regexp, recursive);
	regfree(& regexp);	
	return _return;
}