#include <zapata/io/pipe.h>

auto main(int argc, char* argv[]) -> int {
    zpt::pipestream _pss;
    _pss.open("pipe");
    _pss << "abc fgh" << std::flush;
    std::cout << "wrote to pipe" << std::endl << std::flush;
    std::string _out;
    _pss >> _out;
    std::cout << _out << std::endl << std::flush;
    _pss >> _out;
    std::cout << _out << std::endl << std::flush;
    return 0;
}
