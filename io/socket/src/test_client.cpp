#include <zapata/net/socket.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 3) {
        std::string _type{ argv[1] };
        if (_type == "-t") {
            std::istringstream _iss;
            _iss.str(std::string{ argv[2] });
            std::uint16_t _port{ 0 };
            _iss >> _port;

            auto _csock = zpt::make_stream<zpt::socketstream>("localhost", _port);
            (*_csock) << std::string{ argv[3] } << std::endl << std::flush;
        }
        else if (_type == "-u") {
            std::string _path{ argv[2] };
            auto _csock = zpt::make_stream<zpt::socketstream>(_path);
            (*_csock) << std::string{ argv[3] } << std::endl << std::flush;
        }
    }
    return 0;
}
