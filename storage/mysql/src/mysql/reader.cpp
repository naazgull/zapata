#include <zapata/mysql.h>

#include <fstream>

auto
main(int _arg_c, char* _argv[]) -> int {
    zpt::log_fd = &std::cout;
    zpt::log_pid = ::getpid();
    zpt::log_pname = new string(_argv[0]);
    zpt::log_lvl = 7;
    zpt::log_format = 1;

    std::string _bl(_argv[1]);
    std::ifstream _ifs;
    _ifs.open(_argv[1]);

    zpt::mysql::magic_number _mn;
    _ifs >> _mn;

    while (_ifs.good()) {
        zpt::mysql::event _event;
        _ifs >> _event;
        if (!_event.get()) {
            break;
        }
        std::cout << zpt::json::pretty(_event->to_json()) << std::endl << std::flush;
    }
    return 0;
}
