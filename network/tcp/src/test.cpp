#include <zapata/transport.h>
#include <zapata/net/socket.h>
#include <zapata/net/tcp.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 1) {
        std::istringstream _iss;
        _iss.str(std::string{ argv[1] });
        std::uint16_t _port{ 0 };
        _iss >> _port;

        zpt::transport _transport{ new zpt::net::transport::tcp{} };
        zpt::serversocketstream _ssock{ _port };
        do {
            auto _stream = _ssock->accept();
            _stream->transport("tcp");
            auto _t1 = std::chrono::high_resolution_clock::now();
            auto _received = _transport->receive(*_stream);
            auto _t2 = std::chrono::high_resolution_clock::now();
            auto _duration1 =
              std::chrono::duration_cast<std::chrono::microseconds>(_t2 - _t1).count();
            auto _t3 = std::chrono::high_resolution_clock::now();
            _transport->send(*_stream, _received);
            auto _t4 = std::chrono::high_resolution_clock::now();
            auto _duration2 =
              std::chrono::duration_cast<std::chrono::microseconds>(_t4 - _t3).count();
            std::cout << "# processing time" << std::endl
                      << "\trequest: " << _duration1 << "µs" << std::endl
                      << "\tresponse: " << _duration2 << "µs" << std::endl
                      << std::flush;
        } while (true);
    }
}
