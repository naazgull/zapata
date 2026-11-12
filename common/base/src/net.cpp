#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <zapata/base/expect.h>
#include <zapata/net/manip.h>
#include <zapata/text/manip.h>

namespace {
auto split(std::string const& _to_split, std::string const& separator) -> std::vector<std::string>;
} // namespace

auto zpt::net::getip(std::string const& _if) -> std::string {
    std::string _out;
    struct ifaddrs *_if_addr = nullptr, *_ifa = nullptr;
    void* _tmp_add_ptr = nullptr;

    getifaddrs(&_if_addr);
    for (_ifa = _if_addr; _ifa != nullptr; _ifa = _ifa->ifa_next) {
        if (_ifa->ifa_addr->sa_family == AF_INET) {
            char _mask[INET_ADDRSTRLEN];
            void* _mask_ptr = &((struct sockaddr_in*)_ifa->ifa_netmask)->sin_addr;
            inet_ntop(AF_INET, _mask_ptr, _mask, INET_ADDRSTRLEN);
            if (strcmp(_mask, "255.0.0.0") != 0 &&
                (_if.length() == 0 || std::string(_ifa->ifa_name) == _if)) {
                _tmp_add_ptr = &((struct sockaddr_in*)_ifa->ifa_addr)->sin_addr;
                char _address_buf[INET_ADDRSTRLEN];
                bzero(_address_buf, INET_ADDRSTRLEN);
                inet_ntop(AF_INET, _tmp_add_ptr, _address_buf, INET_ADDRSTRLEN);
                _out.assign(_address_buf);
                if (_if_addr != nullptr) freeifaddrs(_if_addr);
                if (_out.length() == 0 || _out == "::") {
                    if (_if.length() == 0) { continue; }
                    return "127.0.0.1";
                }
                return _out;
            }
        }
        else if (_ifa->ifa_addr->sa_family == AF_INET6) {
            char _mask[INET6_ADDRSTRLEN];
            void* _mask_ptr = &((struct sockaddr_in*)_ifa->ifa_netmask)->sin_addr;
            inet_ntop(AF_INET6, _mask_ptr, _mask, INET6_ADDRSTRLEN);
            if (strcmp(_mask, "255.0.0.0") != 0 &&
                (_if.length() == 0 || std::string(_ifa->ifa_name) == _if)) {
                _tmp_add_ptr = &((struct sockaddr_in*)_ifa->ifa_addr)->sin_addr;
                char _address_buf[INET6_ADDRSTRLEN];
                bzero(_address_buf, INET6_ADDRSTRLEN);
                inet_ntop(AF_INET6, _tmp_add_ptr, _address_buf, INET6_ADDRSTRLEN);
                _out.assign(_address_buf);
                if (_if_addr != nullptr) freeifaddrs(_if_addr);
                if (_out.length() == 0 || _out == "::") {
                    if (_if.length() == 0) { continue; }
                    return "127.0.0.1";
                }
                return _out;
            }
        }
    }
    if (_if_addr != nullptr) freeifaddrs(_if_addr);
    if (_out.length() == 0 || _out == "::") { return "127.0.0.1"; }
    return _out;
}

auto zpt::net::get_available_port(std::string const& _protocol, std::uint32_t _start_from)
  -> std::uint32_t {
    std::set<std::uint32_t> _assigned;
    std::uint32_t _result{ 0 };

    std::ifstream _ifs;
    _ifs.open(std::format("/proc/net/{}", _protocol));
    expect(_ifs.is_open(), "couldn't open /proc/net/" << _protocol);

    std::string _line;
    std::getline(_ifs, _line);
    while (_ifs.good()) {
        std::getline(_ifs, _line);
        zpt::trim(_line);
        if (_line.length() == 0) { break; }

        auto _address = ::split(_line, " ");
        if (_address.size() < 2) { continue; }
        auto _port = ::split(_address[1], ":");
        if (_port.size() < 2) { continue; }

        std::uint32_t _port_i;
        std::istringstream _ss;
        _ss.str(_port[1]);
        _ss >> std::hex >> _port_i;

        if (_port_i > _start_from) { _assigned.insert(_port_i); }

        _line.clear();
    }

    for (std::uint32_t _try = _start_from + 1; _try != 49150; ++_try) {
        if (!_assigned.contains(_try)) {
            _result = _try;
            break;
        }
    }

    _ifs.close();
    expect(_result != 0, "no available ports to assign");
    return _result;
}

auto zpt::net::resolve_address(std::string const& _domain) -> zpt::addresses_t {
    addrinfo _hints{};
    _hints.ai_family = AF_UNSPEC; // IPv4 and IPv6
    _hints.ai_socktype = SOCK_STREAM;

    addrinfo* _results = nullptr;
    int _err = getaddrinfo(_domain.c_str(), nullptr, &_hints, &_results);
    expect(_err == 0, "getaddrinfo error: " << gai_strerror(_err));

    zpt::addresses_t _return;
    for (addrinfo* _r = _results; _r != nullptr; _r = _r->ai_next) {
        if (_r->ai_family == AF_INET) {
            char _ip[INET_ADDRSTRLEN] = { 0 };
            auto* _addr = reinterpret_cast<sockaddr_in*>(_r->ai_addr);
            inet_ntop(AF_INET, &_addr->sin_addr, _ip, sizeof(_ip));
            _return["ipv4"].push_back(std::string{ const_cast<char const*>(_ip) });
        }
        else if (_r->ai_family == AF_INET6) {
            char _ip[INET6_ADDRSTRLEN] = { 0 };
            auto* _addr = reinterpret_cast<sockaddr_in6*>(_r->ai_addr);
            inet_ntop(AF_INET6, &_addr->sin6_addr, _ip, sizeof(_ip));
            _return["ipv6"].push_back(std::string{ const_cast<char const*>(_ip) });
        }
    }

    freeaddrinfo(_results);
    return _return;
}

auto zpt::net::ping(std::string const& _address, std::uint16_t _port, std::uint32_t _timeout)
  -> bool {
    addrinfo _hints{};
    _hints.ai_family = AF_UNSPEC; // IPv4 and IPv6
    _hints.ai_socktype = SOCK_STREAM;

    addrinfo* _results = nullptr;
    int _err = getaddrinfo(_address.c_str(), std::to_string(_port).c_str(), &_hints, &_results);
    expect(_err == 0, "getaddrinfo error: " << gai_strerror(_err));

    bool _reachable = false;
    for (addrinfo* _result = _results; _result != nullptr && !_reachable;
         _result = _result->ai_next) {
        int _fd = socket(_result->ai_family, _result->ai_socktype, _result->ai_protocol);
        if (_fd < 0) { continue; }

        fcntl(_fd, F_SETFL, O_NONBLOCK);
        _err = connect(_fd, _result->ai_addr, _result->ai_addrlen);
        if (_err == 0) {
            _reachable = true;
        }
        else if (_err == -1 && (errno == EINPROGRESS || errno == EAGAIN)) {
            pollfd _poll{ _fd, POLLOUT, 0 };
            if (poll(&_poll, 1, static_cast<int>(_timeout * 1000)) == 1 &&
                (_poll.revents & POLLOUT)) {
                int _socket_error{ 0 };
                socklen_t _length = sizeof(_socket_error);
                getsockopt(_fd, SOL_SOCKET, SO_ERROR, &_socket_error, &_length);
                _reachable = _socket_error == 0;
            }
        }

        close(_fd);
    }

    freeaddrinfo(_results);
    return _reachable;
}

namespace {
auto split(std::string const& _to_split, std::string const& _separator)
  -> std::vector<std::string> {
    std::vector<std::string> _result;
    std::istringstream _iss;
    _iss.str(_to_split);

    while (_iss.good()) {
        std::string _part;
        std::getline(_iss, _part, _separator[0]);
        _result.push_back(_part);
    }

    return _result;
}
} // namespace
