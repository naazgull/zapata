#include <zapata/net/socket.h>
#include <zapata/json/JSONClass.h>
#include <zapata/http.h>

auto
main(int argc, char* argv[]) -> int {
    if (argc > 1) {
        try {
            std::istringstream _iss;
            _iss.str(std::string{ argv[1] });
            std::uint16_t _port{ 0 };
            _iss >> _port;

            zpt::serversocketstream _ssock{ _port };
            do {
                auto _csock = _ssock->accept();
                auto _t1 = std::chrono::high_resolution_clock::now();
                zpt::http::req _request;
                (*_csock) >> std::noskipws >> _request;
                std::cout << _request << std::flush;

                zpt::http::rep _reply;
                _reply->status(zpt::http::status::HTTP200);
                if (argc > 2) {
                    std::ifstream _ifs;
                    _ifs.open(argv[2]);
                    zpt::json _data;
                    _ifs >> _data;
                    std::string _body{ static_cast<std::string>(_data) };
                    _reply->header("Content-Type", "application/json");
                    _reply->header("Content-Length", std::to_string(_body.length()));
                    _reply->body(_body);
                }
                else {
                    std::string _body{ "<h1>HELLO WORLD!</h1>" };
                    _reply->header("Content-Type", "text/html");
                    _reply->header("Content-Length", std::to_string(_body.length()));
                    _reply->body(_body);
                }
                (*_csock) << _reply << std::flush;
                auto _t2 = std::chrono::high_resolution_clock::now();
                auto _duration =
                  std::chrono::duration_cast<std::chrono::milliseconds>(_t2 - _t1).count();
                _reply->body("");
                std::cout << _reply << std::flush;
                std::cout << "# processing time: " << _duration << "ms" << std::endl << std::flush;

            } while (true);
        }
        catch (std::exception const& _e) {
            std::cout << _e.what() << std::endl << std::flush;
        }
    }
    return 0;
}
