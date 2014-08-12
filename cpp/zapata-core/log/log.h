/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

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
		sys = 0,
		error = 1,
		warning = 2,
		info = 3,
		debug = 4
	};

	void log(string __text, zapata::LogLevel _level);
}

