#include <zapata/http/retrieve.h>

auto main(int _argc, char* _argv[]) -> int {
    std::string _uri{ "https://en.wikipedia.org/wiki/Main_Page" };
    if (_argc > 1) { _uri.assign(_argv[1]); }

    auto _request = zpt::allocate_message<zpt::http::basic_request>();
    _request //
      ->performative(zpt::Get)
      .uri(_uri);

    std::cout << _request << std::endl;

    auto _reply = zpt::http::retrieve(_request);
    if (_reply != nullptr) { std::cout << _reply << std::endl; }

    return 0;
}
