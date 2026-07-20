#include <format>
#include <fstream>
#include <iostream>
#include <set>
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
