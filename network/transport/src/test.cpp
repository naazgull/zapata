#include <zapata/transport.h>
#include <zapata/net/socket.h>

class some_protocol : public zpt::basic_transport {
  public:
    auto make_request() const -> zpt::message override {
        return nullptr;
    }

    auto make_reply() const -> zpt::message override {
        return nullptr;
    }

    auto make_reply(zpt::message _request) const -> zpt::message override {
        return nullptr;
    }

    auto receive(zpt::basic_stream& _stream) const -> zpt::message override {
        auto _message = zpt::make_message<zpt::json_message>();
        _stream >> _message;
        std::cout << _message << std::endl << std::flush;
        return _message;
    }

    auto send(zpt::basic_stream& _stream, zpt::message _to_send) const
      -> void override {
        _stream << _to_send << std::flush;
    }
};

auto
main(int argc, char* argv[]) -> int {
    if (argc > 1) {
        std::istringstream _iss;
        _iss.str(std::string{ argv[1] });
        std::uint16_t _port{ 0 };
        _iss >> _port;

        zpt::transport _transport{ new some_protocol{} };
        zpt::serversocketstream _ssock{ _port };
        do {
            auto _stream = _ssock->accept();
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
    return 0;
}
