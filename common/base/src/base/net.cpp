#include <zapata/net/manip.h>

auto
zpt::net::getip(std::string _if) -> std::string {
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
                if (_if_addr != nullptr)
                    freeifaddrs(_if_addr);
                if (_out.length() == 0 || _out == "::") {
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
                if (_if_addr != nullptr)
                    freeifaddrs(_if_addr);
                if (_out.length() == 0 || _out == "::") {
                    return "127.0.0.1";
                }
                return _out;
            }
        }
    }
    if (_if_addr != nullptr)
        freeifaddrs(_if_addr);
    if (_out.length() == 0 || _out == "::") {
        return "127.0.0.1";
    }
    return _out;
}
