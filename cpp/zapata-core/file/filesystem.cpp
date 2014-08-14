/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

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

#include <file/manip.h>

#include <fcntl.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/sendfile.h>

bool zapata::mkdir_recursive(string _name, mode_t _mode) {
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

bool zapata::copy_path(string _from, string _to) {
	int _read_fd;
	int _write_fd;
	struct stat _stat_buf;

	_read_fd = open(_from.c_str(), O_RDONLY);
	if (_read_fd < 0) {
		return _read_fd;
	}
	fstat(_read_fd, &_stat_buf);
	_write_fd = open(_to.c_str(), O_WRONLY | O_CREAT, _stat_buf.st_mode);
	int _error = sendfile(_write_fd, _read_fd, NULL, _stat_buf.st_size);
	close(_read_fd);
	close(_write_fd);

	return _error != -1;
}

bool zapata::move_path(string _from, string _to) {
	if (zapata::copy_path(_from, _to)) {
		return std::remove(_from.c_str()) != 0;
	}
	return false;
}

bool zapata::load_path(string _in, string& _out) {
	ifstream _ifs;
	_ifs.open(_in.data());

	if (_ifs.is_open()) {
		while(_ifs >> _out);
		_ifs.close();
		return true;
	}
	return false;
}

bool zapata::load_path(string _in, wstring& _out) {
	wifstream _ifs;
	_ifs.open(_in.data());

	if (_ifs.is_open()) {
		while(_ifs >> _out);
		_ifs.close();
		return true;
	}
	return false;
}


bool zapata::dump_path(string _in, string& _content) {
	ofstream _ofs;
	_ofs.open(_in.data());
	_ofs << _content << flush;
	_ofs.flush();
	_ofs.close();
	return true;
}

bool zapata::dump_path(string _in, wstring& _content) {
	wofstream _ofs;
	_ofs.open(_in.data());
	_ofs << _content << flush;
	_ofs.flush();
	_ofs.close();
	return true;
}
