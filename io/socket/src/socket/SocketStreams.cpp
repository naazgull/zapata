#include <zapata/net/socket/SocketStreams.h>

std::string
zpt::ssl_error_print(SSL* _ssl, int _ret) {
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

std::string
zpt::ssl_error_print(unsigned long _error) {
    if (_error == 0) {
        _error = ERR_get_error();
    }
    return std::string("SSL_ERROR_") + std::to_string(_error) + std::string(": ") +
           std::string(ERR_error_string(_error, nullptr));
}
