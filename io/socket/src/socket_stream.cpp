#include <zapata/net/socket/socket_stream.h>

auto zpt::ssl_error_print(SSL* _ssl, int _ret) -> std::string {
    switch (SSL_get_error(_ssl, _ret)) {
        case SSL_ERROR_NONE: {
            return std::string("SSL_ERROR_NONE: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
        case SSL_ERROR_ZERO_RETURN: {
            return std::string("SSL_ERROR_ZERO_RETURN: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
        case SSL_ERROR_WANT_READ: {
            return std::string("SSL_ERROR_WANT_READ: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
        case SSL_ERROR_WANT_WRITE: {
            return std::string("SSL_ERROR_WANT_WRITE: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
        case SSL_ERROR_WANT_CONNECT: {
            return std::string("SSL_ERROR_WANT_CONNECT: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
        case SSL_ERROR_WANT_ACCEPT: {
            return std::string("SSL_ERROR_WANT_ACCEPT: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
        case SSL_ERROR_WANT_X509_LOOKUP: {
            return std::string("SSL_ERROR_WANT_X509_LOOKUP: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
        /*case SSL_ERROR_WANT_ASYNC : {
         return std::string("SSL_ERROR_WANT_ASYNC: ") +
         std::string(ERR_error_string(ERR_get_error(),
         nullptr));
         }*/
        case SSL_ERROR_SYSCALL: {
            return std::string("SSL_ERROR_SYSCALL: ") + std::string(strerror(errno));
        }
        case SSL_ERROR_SSL: {
            return std::string("SSL_ERROR_SSL: ") +
                   std::string(ERR_error_string(ERR_get_error(), nullptr));
        }
    }
    return "UNKNOW ERROR";
}

auto zpt::ssl_error_print(unsigned long _error) -> std::string {
    if (_error == 0) { _error = ERR_get_error(); }
    return std::string("SSL_ERROR_") + std::to_string(_error) + std::string(": ") +
           std::string(ERR_error_string(_error, nullptr));
}

auto zpt::is_multicast_address(std::string const& _ip) -> bool {
    std::istringstream _iss;
    _iss.str(_ip.substr(0, 3));
    unsigned int _ip_range{ 0 };
    _iss >> _ip_range;
    return _ip_range >= 224 && _ip_range <= 239;
}

auto zpt::bind_to_address(zpt::sockaddrin_t& _to_bind, std::string const& _address) -> bool {
    in_addr_t _addr = inet_addr(_address.c_str());
    if (_addr == INADDR_NONE) {
        addrinfo _hints{};
        _hints.ai_family = AF_INET;
        _hints.ai_socktype = SOCK_STREAM;
        addrinfo* _results = nullptr;
        if (getaddrinfo(_address.c_str(), nullptr, &_hints, &_results) == 0 &&
            _results != nullptr) {
            _addr = reinterpret_cast<sockaddr_in*>(_results->ai_addr)->sin_addr.s_addr;
            freeaddrinfo(_results);
        }
        else { return false; }
    }
    _to_bind.sin_addr.s_addr = _addr;
    return true;
}

zpt::serversocketstream::serversocketstream()
  : __underlying{ zpt::allocate_shared<zpt::basic_serversocketstream<char>>() } {}

zpt::serversocketstream::serversocketstream(std::string const& _address, std::uint16_t _port)
  : __underlying{ zpt::allocate_shared<zpt::basic_serversocketstream<char>>(_address, _port) } {}

zpt::serversocketstream::serversocketstream(std::string const& _path)
  : __underlying{ zpt::allocate_shared<zpt::basic_serversocketstream<char>>(_path) } {}

zpt::serversocketstream::serversocketstream(const zpt::serversocketstream& _rhs) { (*this) = _rhs; }

zpt::serversocketstream::serversocketstream(zpt::serversocketstream&& _rhs) { (*this) = _rhs; }

auto zpt::serversocketstream::operator=(const zpt::serversocketstream& _rhs)
  -> zpt::serversocketstream& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::serversocketstream::operator=(zpt::serversocketstream&& _rhs)
  -> zpt::serversocketstream& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::serversocketstream::operator->() -> zpt::basic_serversocketstream<char>* {
    return this->__underlying.get();
}

auto zpt::serversocketstream::operator*() -> zpt::basic_serversocketstream<char>& {
    return *this->__underlying.get();
}

zpt::wserversocketstream::wserversocketstream()
  : __underlying{ zpt::allocate_shared<zpt::basic_serversocketstream<wchar_t>>() } {}

zpt::wserversocketstream::wserversocketstream(std::string const& _address, std::uint16_t _port)
  : __underlying{ zpt::allocate_shared<zpt::basic_serversocketstream<wchar_t>>(_address, _port) } {}

zpt::wserversocketstream::wserversocketstream(std::string const& _path)
  : __underlying{ zpt::allocate_shared<zpt::basic_serversocketstream<wchar_t>>(_path) } {}

zpt::wserversocketstream::wserversocketstream(const zpt::wserversocketstream& _rhs) {
    (*this) = _rhs;
}

zpt::wserversocketstream::wserversocketstream(zpt::wserversocketstream&& _rhs) { (*this) = _rhs; }

auto zpt::wserversocketstream::operator=(const zpt::wserversocketstream& _rhs)
  -> zpt::wserversocketstream& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto zpt::wserversocketstream::operator=(zpt::wserversocketstream&& _rhs)
  -> zpt::wserversocketstream& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::wserversocketstream::operator->() -> zpt::basic_serversocketstream<wchar_t>* {
    return this->__underlying.get();
}

auto zpt::wserversocketstream::operator*() -> zpt::basic_serversocketstream<wchar_t>& {
    return *this->__underlying.get();
}
