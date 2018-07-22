/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#include <zapata/base/assertz.h>
#include <zapata/file/manip.h>

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

int zpt::ls(std::string dir, std::vector<string>& result, bool recursive) {
	DIR* dp;
	struct dirent* dirp;
	if ((dp = opendir(dir.c_str())) == NULL) {
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		std::string cname = std::string(dirp->d_name);
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

bool zpt::mkdir_recursive(std::string _name) {
	istringstream _iss(_name);
	std::string _line;
	int _count = 0;
	ostringstream _dname;

	while (_iss.good()) {
		std::getline(_iss, _line, '/');
		std::string cname(_line.data());
		_dname << cname << flush;

		if (mkdir(_dname.str().data(), 0755) == 0) {
			_count++;
		}
		_dname << "/" << flush;
	}
	return _count != 0;
}

bool zpt::copy_path(std::string _from, std::string _to) {
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

bool zpt::move_path(std::string _from, std::string _to) {
	if (zpt::copy_path(_from, _to)) {
		return std::remove(_from.c_str()) != 0;
	}
	return false;
}

bool zpt::load_path(std::string _in, std::string& _out) {
	std::ifstream _ifs;
	_ifs.open(_in.data());

	if (_ifs.is_open()) {
		_ifs.seekg(0, std::ios::end);
		_out.reserve(_ifs.tellg());
		_ifs.seekg(0, std::ios::beg);
		_out.assign((std::istreambuf_iterator<char>(_ifs)), std::istreambuf_iterator<char>());
		_ifs.close();
		return true;
	}
	return false;
}

bool zpt::load_path(std::string _in, std::wstring& _out) {
	std::wifstream _ifs;
	_ifs.open(_in.data());

	if (_ifs.is_open()) {
		_ifs.seekg(0, std::ios::end);
		_out.reserve(_ifs.tellg());
		_ifs.seekg(0, std::ios::beg);
		_out.assign((std::istreambuf_iterator<wchar_t>(_ifs)), std::istreambuf_iterator<wchar_t>());
		_ifs.close();
		return true;
	}
	return false;
}

bool zpt::dump_path(std::string _in, std::string& _content) {
	ofstream _ofs;
	_ofs.open(_in.data());
	_ofs << _content << flush;
	_ofs.flush();
	_ofs.close();
	return true;
}

bool zpt::dump_path(std::string _in, wstring& _content) {
	wofstream _ofs;
	_ofs.open(_in.data());
	_ofs << _content << flush;
	_ofs.flush();
	_ofs.close();
	return true;
}

int zpt::globRegexp(std::string& dir, vector<string>& result, std::regex& pattern, short recursion) {
	DIR* dp;
	struct dirent* dirp;
	vector<string> torecurse;

	if ((dp = opendir(dir.c_str())) == NULL) {
		return errno;
	}
	while ((dirp = readdir(dp)) != NULL) {
		std::string cname = std::string(dirp->d_name);
		if (cname.find('.') != 0) {
			if (dir[dir.length() - 1] != '/') {
				cname.insert(0, "/");
			}
			cname.insert(0, dir.data());
			if (recursion != 0 && dirp->d_type == 4 && cname != dir) {
				torecurse.push_back(cname);
			}
			if (std::regex_match(std::string(dirp->d_name), pattern)) {
				result.insert(result.begin(), cname);
			}
		}
	}
	closedir(dp);

	for (auto i : torecurse) {
		zpt::globRegexp(i, result, pattern, recursion - 1);
	}

	return 0;
}

int zpt::glob(std::string dir, vector<string>& result, std::string pattern, short recursion) {
	std::regex regexp(pattern);
	int _return = zpt::globRegexp(dir, result, regexp, recursion);
	return _return;
}

bool zpt::is_dir(std::string _path) {
	struct stat _s;
	if (stat(_path.data(), &_s) == 0) {
		return _s.st_mode & S_IFDIR;
	}
	return false;
}
