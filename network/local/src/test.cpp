#include <zapata/transport.h>
#include <zapata/net/socket.h>
#include <zapata/net/local.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 2) {
        std::string _scheme{ argv[1] };
        std::string _path{ argv[2] };

        if (_scheme == "unix") {
            zpt::transport _transport{ new zpt::net::transport::unix_socket{} };
            zpt::serversocketstream _ssock{ _path };
            do {
                auto _stream = _ssock->accept();
                _stream->transport("unix");
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
        if (_scheme == "file") {
            zpt::transport _transport{ new zpt::net::transport::file{} };
            auto _in = zpt::make_stream<std::fstream>(_path, std::ios_base::in);
            auto _out = zpt::make_stream<std::fstream>(
              _path, std::ios_base::out | std::ios_base::ate | std::ios_base::app);
            _in->transport("file");
            _out->transport("file");
            auto _t1 = std::chrono::high_resolution_clock::now();
            auto _received = _transport->receive(*_in);
            auto _t2 = std::chrono::high_resolution_clock::now();
            auto _duration1 =
              std::chrono::duration_cast<std::chrono::microseconds>(_t2 - _t1).count();
            auto _t3 = std::chrono::high_resolution_clock::now();
            _transport->send(*_out, _received);
            auto _t4 = std::chrono::high_resolution_clock::now();
            auto _duration2 =
              std::chrono::duration_cast<std::chrono::microseconds>(_t4 - _t3).count();
            std::cout << "# processing time" << std::endl
                      << "\trequest: " << _duration1 << "µs" << std::endl
                      << "\tresponse: " << _duration2 << "µs" << std::endl
                      << std::flush;
        }
    }
    return 0;
}
