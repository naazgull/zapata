#include <zapata/net/socket.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 2) {
        std::unique_ptr<zpt::serversocketstream> _ssock{ nullptr };
        std::string _type{ argv[1] };
        if (_type == "-t") {
            std::istringstream _iss;
            _iss.str(std::string{ argv[2] });
            uint16_t _port{ 0 };
            _iss >> _port;
            _ssock = std::make_unique<zpt::serversocketstream>(_port);
        }
        else if (_type == "-u") {
            std::string _path{ argv[2] };
            _ssock = std::make_unique<zpt::serversocketstream>(_path);
        }

        do {
            auto _csock = (*_ssock)->accept();
            std::cout << ">>> connection accepted" << std::endl << std::flush;
            do {
                char _content{ '\0' };
                (*_csock) >> std::noskipws >> _content;
                std::cout << _content << std::flush;
            } while ((*_csock)->good());
        } while (true);
    }
    return 0;
}
