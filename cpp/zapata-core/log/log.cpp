#include <log/log.h>

namespace zapata {
	short int log_lvl = 0;
	ostream* log_fd = NULL;

	const char* log_lvl_names[] = {
		"\033[4;37minfo\033[0m   | ",
		"\033[1;31merror\033[0m  | ",
		"\033[1;33mwarning\033[0m| ",
		"\033[1;32minfo\033[0m   | ",
		"\033[1;35mdebug\033[0m  | "
	};
}

void zapata::log(string _text, zapata::LogLevel _level) {
	if (_level <= zapata::log_lvl) {
		(*zapata::log_fd) << zapata::log_lvl_names[_level] << _text << endl << flush;
	}
}
