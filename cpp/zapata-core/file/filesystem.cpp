#include <file/manip.h>

#include <fcntl.h>
#include <stdio.h>
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

	return _error == -1;
}

bool zapata::move_path(string from, string to) {
	if (zapata::copy_path(from, to) == 0) {
		return remove(from.c_str()) != 0;
	}
	return false;
}
