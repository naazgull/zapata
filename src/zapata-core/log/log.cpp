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

#include <zapata/log/log.h>

#include <zapata/text/convert.h>

namespace zapata {
	short int log_lvl = 0;
	ostream* log_fd = nullptr;

	const char* log_lvl_names[] = {
		"\033[1;34m\033[4;35msys\033[0m     | ",
		"\033[1;31m\033[4;31merror\033[0m   | ",
		"\033[1;33m\033[4;33mwarning\033[0m | ",
		"\033[1;32m\033[4;34minfo\033[0m    | ",
		"\033[1;35m\033[4;36mdebug\033[0m   | "
	};
}

void zapata::log(string _prefix, string _text, zapata::LogLevel _level) {
	if (_level <= zapata::log_lvl) {
		string _time;
		zapata::tostr(_time, time(NULL), "%F %T");
		zapata::replace(_text, "\n", "");
		(*zapata::log_fd) << zapata::log_lvl_names[_level] << _time << " | \033[4;32m" << _prefix << "\033[0m | " << _text << endl << flush;
	}
}

void zapata::log(string _text, zapata::LogLevel _level) {
	if (_level <= zapata::log_lvl) {
		string _time;
		zapata::tostr(_time, time(nullptr), "%F %T");
		zapata::replace(_text, "\n", "");
		(*zapata::log_fd) << zapata::log_lvl_names[_level] << _time << " | " << _text << endl << flush;
	}
}
