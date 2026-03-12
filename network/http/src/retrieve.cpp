#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <zapata/http.h>
#include <zapata/http/retrieve.h>
#include <zapata/net/socket/socket_stream.h>

auto zpt::http::retrieve(zpt::message _to_send) -> zpt::message {
    _to_send->headers()["Cache-Control"] = "no-store";

    auto _scheme = _to_send->uri()("scheme")->string();
    auto _use_ssl = (_scheme == "https");
    auto _domain = _to_send->uri()("domain")->string();

    auto _stream = zpt::make_stream<zpt::socketstream>(
      _domain, _use_ssl ? 443 : 80, _use_ssl, IPPROTO_TCP);

    _stream //
      ->write<zpt::message>(_to_send);

    auto _reply = zpt::allocate_message<zpt::http::basic_reply>();
    (*_stream) >> std::noskipws >> _reply;

    return _reply;
}

auto zpt::http::resolve(std::string const& _domain) -> zpt::json {
    addrinfo _hints{};
    _hints.ai_family = AF_UNSPEC; // IPv4 and IPv6
    _hints.ai_socktype = SOCK_STREAM;

    addrinfo* _results = nullptr;
    int _err = getaddrinfo(_domain.c_str(), nullptr, &_hints, &_results);
    expect(_err == 0, "getaddrinfo error: " << gai_strerror(_err));

    zpt::json _return{ "ipv4", zpt::json::array(), "ipv6", zpt::json::array() };
    for (addrinfo* _r = _results; _r != nullptr; _r = _r->ai_next) {
        if (_r->ai_family == AF_INET) {
            char _ip[INET_ADDRSTRLEN] = { 0 };
            auto* _addr = reinterpret_cast<sockaddr_in*>(_r->ai_addr);
            inet_ntop(AF_INET, &_addr->sin_addr, _ip, sizeof(_ip));
            _return["ipv4"] << std::string{ _ip, sizeof(_ip) };
        }
        else if (_r->ai_family == AF_INET6) {
            char _ip[INET6_ADDRSTRLEN] = { 0 };
            auto* _addr = reinterpret_cast<sockaddr_in6*>(_r->ai_addr);
            inet_ntop(AF_INET6, &_addr->sin6_addr, _ip, sizeof(_ip));
            _return["ipv6"] << std::string{ _ip, sizeof(_ip) };
        }
    }

    freeaddrinfo(_results);
    return _return;
}
