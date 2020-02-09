#include <zapata/transport.h>
#include <zapata/http.h>
#include <zapata/net/socket.h>

class some_protocol : public zpt::transport::transport_t {
  public:
    auto receive(zpt::message& _message) -> void override {
        zpt::http::req _request;
        (*_message->stream()) >> std::noskipws >> _request;
        if (_request->body().length() != 0 &&
            _request->header("Content-Type") == "application/json") {
            std::istringstream _iss;
            _iss.str(_request->body());
            zpt::json _content;
            try {
                _iss >> _content;
                _message->received() << "version" << _request->version() << "body" << _content;
            }
            catch (...) {
            }
        }
    }

    auto send(zpt::message& _message) -> void override {
        zpt::http::rep _response;
        zpt::init(_response);
        if (_message->received()->size() != 0) {
            _response->status(zpt::http::HTTP200);
            _response->version(_message->received()["version"]);
            _response->body(_message->received()["body"]);
            _response->header("Content-Type", "application/json");
        }
        else {
            _response->status(zpt::http::HTTP415);
        }
        (*_message->stream()) << _response << std::flush;
    }
};

auto
main(int argc, char* argv[]) -> int {
    if (argc > 1) {
        try {
            std::istringstream _iss;
            _iss.str(std::string{ argv[1] });
            uint16_t _port{ 0 };
            _iss >> _port;

            zpt::transport _transport = zpt::transport::alloc<some_protocol>();
            zpt::serversocketstream _ssock{ _port };
            do {
                auto _stream = _ssock->accept();
                auto _t1 = std::chrono::high_resolution_clock::now();
                zpt::message _received{ _stream.get() };
                _transport->receive(_received);
                auto _t2 = std::chrono::high_resolution_clock::now();
                auto _duration1 =
                  std::chrono::duration_cast<std::chrono::microseconds>(_t2 - _t1).count();
                auto _t3 = std::chrono::high_resolution_clock::now();
                _transport->send(_received);
                auto _t4 = std::chrono::high_resolution_clock::now();
                auto _duration2 =
                  std::chrono::duration_cast<std::chrono::microseconds>(_t4 - _t3).count();
                std::cout << "# processing time" << std::endl
                          << "\trequest: " << _duration1 << "µs" << std::endl
                          << "\tresponse: " << _duration2 << "µs" << std::endl
                          << std::flush;
            } while (true);
        }
        catch (zpt::failed_expectation& _e) {
            std::cout << _e.what() << std::endl << _e.description() << std::endl << std::flush;
        }
        catch (zpt::SyntaxErrorException& _e) {
            std::cout << _e.what() << std::endl << std::flush;
        }
    }
    return 0;
}
