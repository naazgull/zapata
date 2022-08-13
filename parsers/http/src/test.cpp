#include <zapata/net/socket.h>
#include <zapata/json/JSONClass.h>
#include <zapata/http.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 1) {
        std::istringstream _iss;
        _iss.str(std::string{ argv[1] });
        std::uint16_t _port{ 0 };
        _iss >> _port;

        zpt::serversocketstream _ssock{ _port };
        do {
            auto _csock = _ssock->accept();
            auto _t1 = std::chrono::high_resolution_clock::now();
            auto _request = zpt::make_message<zpt::http::basic_request>();
            (*_csock) >> std::noskipws >> _request;

            auto _reply = zpt::make_message<zpt::http::basic_reply>(
              message_cast<zpt::http::basic_request>(_request), true);
            _reply->status(zpt::http::status::HTTP200);
            if (argc > 2) {
                std::ifstream _ifs;
                _ifs.open(argv[2]);
                zpt::json _data;
                _ifs >> _data;
                _reply->headers()["Content-Type"] = "application/json";
                _reply->body() = _data;
            }
            else {
                std::string _body{ "<h1>HELLO WORLD!</h1>" };
                _reply->headers()["Content-Type"] = "text/html";
                _reply->body() = _body;
            }
            (*_csock) << _reply << std::flush;
            auto _t2 = std::chrono::high_resolution_clock::now();
            auto _duration =
              std::chrono::duration_cast<std::chrono::microseconds>(_t2 - _t1).count();
            std::cout << _request << std::flush;
            std::cout << _reply << std::flush;
            std::cout << std::endl
                      << "# processing time: " << _duration << "us" << std::endl
                      << "-----------------------------------------------------------------"
                      << std::endl
                      << std::flush;

        } while (true);
    }
    return 0;
}
