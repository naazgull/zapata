#include <zapata/io/pipe.h>

auto main(int, char**) -> int {
    zpt::pipestream _pss;
    _pss.open("pipe");

    if (fork()) { // child
        std::string _out;
        _pss >> _out;
        std::cout << _out << std::endl << std::flush;
    }
    else { // parent
        _pss << "abc fgh" << std::flush;
        std::cout << "wrote to pipe" << std::endl << std::flush;
    }
    return 0;
}
