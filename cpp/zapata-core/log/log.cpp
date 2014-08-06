/*
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

#include <log/log.h>

#include <text/convert.h>

namespace zapata {
	short int log_lvl = 0;
	ostream* log_fd = NULL;

	const char* log_lvl_names[] = {
		"\033[1;34m\033[4;34mlog\033[0m    | ",
		"\033[1;31m\033[4;31merror\033[0m  | ",
		"\033[1;33m\033[4;33mwarning\033[0m| ",
		"\033[1;32m\033[4;32minfo\033[0m   | ",
		"\033[1;35m\033[4;35mdebug\033[0m  | "
	};
}

void zapata::log(string _text, zapata::LogLevel _level) {
	if (_level <= zapata::log_lvl) {
		string _time;
		zapata::tostr(_time, time(NULL), "%F %T");
		(*zapata::log_fd) << zapata::log_lvl_names[_level] << "\033[1;37m" << _time << "\033[0m | " << _text << endl << flush;
	}
}
