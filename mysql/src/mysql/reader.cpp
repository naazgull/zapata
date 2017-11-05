#include <zapata/mysql.h>

#include <fstream>

auto main(int _arg_c, char* _argv[]) -> int {
	std::string _bl(_argv[1]);
	std::ifstream _ifs;
	_ifs.open(_argv[1]);
	
	zpt::mysql::magic_number _mn;
	_ifs >> _mn;

	{
		zpt::mysql::event _event;
		_ifs >> _event;
		std::cout << zpt::json::pretty(_event->to_json()) << std::endl << std::flush;
	}	
	{
		//zpt::mysql::event _event = zpt::mysql::event::instance(zpt::mysql::WRITE_ROWS_EVENT);
		zpt::mysql::event _event;
		_ifs >> _event;
		std::cout << zpt::json::pretty(_event->to_json()) << std::endl << std::flush;
	}	
	return 0;
}
