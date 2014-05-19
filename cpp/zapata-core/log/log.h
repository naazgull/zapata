#pragma once

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {
	extern short int log_lvl;
	extern ostream* log_fd;
	extern const char* log_lvl_names[];

	enum LogLevel {
		system = 0,
		error = 1,
		warning = 2,
		info = 3,
		debug = 4
	};

	void log(string __text, zapata::LogLevel _level);
}

