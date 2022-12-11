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

zpt::serversocketstream::serversocketstream()
  : __underlying{ std::make_shared<zpt::basic_serversocketstream<char>>() } {}

zpt::serversocketstream::serversocketstream(std::uint16_t _port)
  : __underlying{ std::make_shared<zpt::basic_serversocketstream<char>>(_port) } {}

zpt::serversocketstream::serversocketstream(std::string const& _path)
  : __underlying{ std::make_shared<zpt::basic_serversocketstream<char>>(_path) } {}

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
  : __underlying{ std::make_shared<zpt::basic_serversocketstream<wchar_t>>() } {}

zpt::wserversocketstream::wserversocketstream(std::uint16_t _port)
  : __underlying{ std::make_shared<zpt::basic_serversocketstream<wchar_t>>(_port) } {}

zpt::wserversocketstream::wserversocketstream(std::string const& _path)
  : __underlying{ std::make_shared<zpt::basic_serversocketstream<wchar_t>>(_path) } {}

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
