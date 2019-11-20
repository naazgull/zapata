#include <zapata/net/socket.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 2) {
        std::istringstream _iss;
        _iss.str(std::string{ argv[1] });
        uint16_t _port{ 0 };
        _iss >> _port;

        zpt::socketstream _csock;
        _csock->open("localhost", _port);
        _csock << std::string{ argv[2] } << std::endl << std::flush;
    }
    return 0;
}
