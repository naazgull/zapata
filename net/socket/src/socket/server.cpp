#include <zapata/net/socket.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 1) {
        std::istringstream _iss;
        _iss.str(std::string{ argv[1] });
        uint16_t _port{ 0 };
        _iss >> _port;

        zpt::serversocketstream _ssock{ _port };
        do {
            auto _csock = _ssock.accept();
            do {
                char _content{ '\0' };
                _csock >> std::noskipws >> _content;
                std::cout << _content << std::flush;
            } while (_csock->good());
        } while (true);
    }
    return 0;
}
