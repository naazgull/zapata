#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <istream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <ostream>
#include <streambuf>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <zapata/base/expect.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/log/log.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

namespace zpt {

std::string
ssl_error_print(SSL* _ssl, int _ret);
std::string
ssl_error_print(unsigned long _error = 0);

template<typename Char>
class basic_socketbuf : public std::basic_streambuf<Char> {
  public:
    typedef Char __char_type;
    typedef std::basic_streambuf<__char_type> __buf_type;
    typedef std::basic_ostream<__char_type> __stream_type;
    typedef typename __buf_type::int_type __int_type;
    typedef typename std::basic_streambuf<Char>::traits_type __traits_type;

  protected:
    static const int char_size = sizeof(__char_type);
    static const int SIZE = 1024;
    __char_type obuf[SIZE];
    __char_type ibuf[SIZE];

    int __sock;
    bool __ssl;
    struct sockaddr_in __server;
    struct sockaddr_in __peer;
    std::string __host;
    short __port;
    short __protocol;
    SSL* __sslstream;
    SSL_CTX* __context;
    unsigned long long __timeout;
    unsigned int __error_code;
    std::string __error_string;

  public:
    basic_socketbuf()
      : __sock(0)
      , __ssl(false)
      , __port(-1)
      , __protocol(0)
      , __sslstream(nullptr)
      , __context(nullptr)
      , __timeout(0)
      , __error_code(0)
      , __error_string("") {
        __buf_type::setp(obuf, obuf + (SIZE - 1));
        __buf_type::setg(ibuf, ibuf, ibuf);
    };

    virtual ~basic_socketbuf() {
        sync();
        if (this->__sslstream != nullptr) {
            SSL_free(this->__sslstream);
            SSL_CTX_free(this->__context);
        }
    }

    auto set_socket(int _sock) -> void {
        this->__sock = _sock;
        if (_sock != 0) {
            int iOption = 1;
            setsockopt(this->__sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&iOption, sizeof(int));
            struct linger a;
            a.l_onoff = 1;
            a.l_linger = 5;
            setsockopt(this->__sock, SOL_SOCKET, SO_LINGER, (char*)&a, sizeof a);
            if (this->__timeout) {
                struct timeval _tv;
                _tv.tv_sec = 5;
                _tv.tv_usec = 0;
                setsockopt(
                  this->__sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&_tv, sizeof(struct timeval));
            }
        }
    }

    auto get_socket() -> int { return this->__sock; }

    auto set_context(SSL_CTX* _ctx) -> void {
        this->__ssl = true;
        this->__context = _ctx;
        this->__sslstream = SSL_new(_ctx);
        SSL_set_tlsext_host_name(this->__sslstream, this->__host.data());
        SSL_set_fd(this->__sslstream, this->__sock);
        SSL_connect(this->__sslstream);
    }

    auto server() -> struct sockaddr_in& { return this->__server; }

    auto ssl() -> bool& { return this->__ssl; }

    auto host() -> std::string& { return this->__host; }

    auto port() -> short& { return this->__port; }

    auto protocol() -> short& { return this->__protocol; }

    auto timeout() -> unsigned long long& { return this->__timeout; }

    auto error_code() -> unsigned int& { return this->__error_code; }

    auto error_string() -> std::string& { return this->__error_string; }

    virtual auto __good() -> bool {
        return this->__sock != 0 && (!this->__ssl || (this->__ssl && this->__sslstream != nullptr &&
                                                      this->__context != nullptr));
    }

  protected:
    auto output_buffer() -> __int_type {
        if (!__good()) {
            return __traits_type::eof();
        }

        if (!this->__ssl) {
            switch (protocol()) {
                case IPPROTO_IP: {
                    int _num = __buf_type::pptr() - __buf_type::pbase();
                    int _actually_written = -1;
                    if ((_actually_written = ::send(
                           __sock, reinterpret_cast<char*>(obuf), _num * char_size, MSG_NOSIGNAL)) <
                        0) {
                        ::shutdown(this->__sock, SHUT_RDWR);
                        ::close(this->__sock);
                        this->__sock = 0;
                        this->__error_code = errno;
                        this->__error_string = std::string(std::strerror(errno));
                        return __traits_type::eof();
                    }
                    __buf_type::pbump(-_actually_written);
                    return _actually_written;
                }
                case IPPROTO_UDP: {
                    int _num = __buf_type::pptr() - __buf_type::pbase();
                    int _actually_written = -1;
                    if ((_actually_written = ::sendto(__sock,
                                                      reinterpret_cast<char*>(obuf),
                                                      _num * char_size,
                                                      0,
                                                      (struct sockaddr*)&__server,
                                                      sizeof __server)) < 0) {
                        if (_actually_written < 0) {
                            ::shutdown(this->__sock, SHUT_RDWR);
                            ::close(this->__sock);
                            this->__sock = 0;
                            this->__error_code = errno;
                            this->__error_string = std::string(std::strerror(errno));
                        }
                        return __traits_type::eof();
                    }
                    __buf_type::pbump(-_actually_written);
                    return _actually_written;
                }
                default: {
                    return __traits_type::eof();
                }
            }
        }
        else {
            int _num = __buf_type::pptr() - __buf_type::pbase();
            int _actually_written = 0;
            do {
                if ((_actually_written = SSL_write(
                       this->__sslstream, reinterpret_cast<char*>(obuf), _num * char_size)) < 0) {
                    if (SSL_get_error(this->__sslstream, _actually_written) !=
                        SSL_ERROR_WANT_WRITE) {
                        SSL_free(this->__sslstream);
                        SSL_CTX_free(this->__context);
                        ::shutdown(this->__sock, SHUT_RDWR);
                        ::close(this->__sock);
                        this->__sock = 0;
                        this->__sslstream = nullptr;
                        this->__context = nullptr;
                        this->__error_code = SSL_get_error(this->__sslstream, _actually_written);
                        this->__error_string =
                          zpt::ssl_error_print(this->__sslstream, _actually_written);
                        return __traits_type::eof();
                    }
                }
            } while (SSL_get_error(this->__sslstream, _actually_written) == SSL_ERROR_WANT_WRITE);
            __buf_type::pbump(-_actually_written);
            return _actually_written;
        }
        return __traits_type::eof();
    }

    virtual auto overflow(__int_type c) -> __int_type {
        if (c != __traits_type::eof()) {
            *__buf_type::pptr() = c;
            __buf_type::pbump(1);
        }

        if (this->output_buffer() == __traits_type::eof()) {
            return __traits_type::eof();
        }
        return c;
    }

    virtual auto sync() -> int {
        if (output_buffer() == __traits_type::eof()) {
            return __traits_type::eof();
        }
        return 0;
    }

    virtual auto underflow() -> __int_type {
        if (__buf_type::gptr() < __buf_type::egptr()) {
            return *__buf_type::gptr();
        }

        if (!__good()) {
            return __traits_type::eof();
        }

        if (!this->__ssl) {
            switch (protocol()) {
                case IPPROTO_IP: {
                    int _actually_read = -1;
                    if ((_actually_read = ::recv(
                           __sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, 0)) < 0) {
                        ::shutdown(this->__sock, SHUT_RDWR);
                        ::close(this->__sock);
                        this->__sock = 0;
                        this->__error_code = errno;
                        this->__error_string = std::string(std::strerror(errno));
                        return __traits_type::eof();
                    }
                    if (_actually_read == 0) {
                        return __traits_type::eof();
                    }
                    __buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
                    return *__buf_type::gptr();
                }
                case IPPROTO_UDP: {
                    int _actually_read = -1;
                    socklen_t _peer_addr_len = sizeof __peer;
                    if ((_actually_read = ::recvfrom(__sock,
                                                     reinterpret_cast<char*>(ibuf),
                                                     SIZE * char_size,
                                                     0,
                                                     (struct sockaddr*)&__peer,
                                                     &_peer_addr_len)) < 0) {
                        ::shutdown(this->__sock, SHUT_RDWR);
                        ::close(this->__sock);
                        this->__sock = 0;
                        this->__error_code = errno;
                        this->__error_string = std::string(std::strerror(errno));
                        return __traits_type::eof();
                    }
                    if (_actually_read == 0) {
                        return __traits_type::eof();
                    }
                    __buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
                    return *__buf_type::gptr();
                }
            }
        }
        else {
            int _actually_read = -1;
            do {
                if ((_actually_read = SSL_read(
                       this->__sslstream, reinterpret_cast<char*>(ibuf), SIZE * char_size)) < 0) {
                    if (SSL_get_error(this->__sslstream, _actually_read) != SSL_ERROR_WANT_READ) {
                        SSL_free(this->__sslstream);
                        SSL_CTX_free(this->__context);
                        ::shutdown(this->__sock, SHUT_RDWR);
                        ::close(this->__sock);
                        this->__sock = 0;
                        this->__sslstream = nullptr;
                        this->__context = nullptr;
                        this->__error_code = SSL_get_error(this->__sslstream, _actually_read);
                        this->__error_string =
                          zpt::ssl_error_print(this->__sslstream, _actually_read);
                        return __traits_type::eof();
                    }
                }
            } while (SSL_get_error(this->__sslstream, _actually_read) == SSL_ERROR_WANT_READ);
            if (_actually_read == 0) {
                return __traits_type::eof();
            }
            __buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
            return *__buf_type::gptr();
        }
        return __traits_type::eof();
    }
};

using socketbuf = basic_socketbuf<char>;
using wsocketbuf = basic_socketbuf<wchar_t>;

template<typename Char>
class basic_socketstream : public std::basic_iostream<Char> {
  public:
    using __char_type = Char;
    using __stream_type = std::basic_iostream<__char_type>;
    using __buf_type = basic_socketbuf<__char_type>;

  protected:
    __buf_type __buf;
    bool __is_error;

  public:
    basic_socketstream()
      : __stream_type(&__buf)
      , __is_error(false){};

    basic_socketstream(int s, bool _ssl = false, short _protocol = IPPROTO_IP)
      : __stream_type(&__buf)
      , __is_error(false) {
        __buf.set_socket(s);
        __buf.protocol() = _protocol;
        __buf.ssl() = _ssl;
    }

    basic_socketstream(const basic_socketstream&) = delete;
    basic_socketstream(basic_socketstream&&) = delete;

    virtual ~basic_socketstream() { this->close(); }

    auto operator=(const basic_socketstream&) -> basic_socketstream& = delete;
    auto operator=(basic_socketstream &&) -> basic_socketstream& = delete;

    auto ssl() -> bool& { return __buf.ssl(); }

    auto host() -> std::string& { return __buf.host(); }

    auto port() -> short& { return __buf.port(); }

    auto protocol() -> short& { return __buf.protocol(); }

    auto assign(int _sockfd) -> void {
        __buf.set_socket(_sockfd);
        __buf.ssl() = false;
    }

    auto assign(int _sockfd, SSL_CTX* _ctx) -> void {
        __buf.set_socket(_sockfd);
        __buf.set_context(_ctx);
        __buf.ssl() = true;
    }

    auto unassign() -> void { __buf.set_socket(0); }

    auto close() -> void {
        __stream_type::flush();
        __stream_type::clear();
        if (__buf.get_socket() != 0) {
            ::shutdown(__buf.get_socket(), SHUT_RDWR);
            ::close(__buf.get_socket());
        }
        __buf.set_socket(0);
    }

    auto is_open() -> bool { return (!__is_error && __buf.get_socket() != 0 && __buf.__good()); }

    auto ready() -> bool {
        fd_set sockset;
        FD_ZERO(&sockset);
        FD_SET(__buf.get_socket(), &sockset);
        return select(__buf.get_socket() + 1, &sockset, nullptr, nullptr, nullptr) == 1;
    }

    auto buffer() -> __buf_type& { return (*this->__buf.get()); }

    auto is_error() -> bool { return this->__buf.error_code() != 0; }

    auto error_code() -> unsigned int& { return this->__buf.error_code(); }

    auto error_string() -> std::string& { return this->__buf.error_string(); }

    auto open(const std::string& _host,
              uint16_t _port,
              bool _ssl = false,
              short _protocol = IPPROTO_IP) -> bool {
        this->close();
        __buf.host() = _host;
        __buf.port() = _port;
        __buf.protocol() = _protocol;
        __buf.ssl() = _ssl;

        ::hostent* _he = gethostbyname(_host.c_str());
        if (_he == nullptr) {
            return false;
        }

        std::string _addr(reinterpret_cast<char*>(_he->h_addr), _he->h_length);
        std::copy(_addr.c_str(),
                  _addr.c_str() + _addr.length(),
                  reinterpret_cast<char*>(&__buf.server().sin_addr.s_addr));
        __buf.server().sin_family = AF_INET;
        __buf.server().sin_port = htons(_port);

        if (!_ssl) {
            switch (_protocol) {
                case IPPROTO_IP: {
                    int _sd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

                    if (::connect(_sd,
                                  reinterpret_cast<sockaddr*>(&__buf.server()),
                                  sizeof __buf.server()) < 0) {
                        __stream_type::setstate(std::ios::failbit);
                        __buf.set_socket(0);
                        __is_error = true;
                        this->__buf.error_code() = errno;
                        this->__buf.error_string() = std::strerror(errno);
                        return false;
                    }
                    else {
                        __buf.set_socket(_sd);
                    }
                    return true;
                }
                case IPPROTO_UDP: {
                    int _sd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                    int _reuse = 1;
                    setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, (char*)&_reuse, sizeof _reuse);
                    int _ret = inet_aton(__buf.host().c_str(), &__buf.server().sin_addr);
                    if (_ret == 0) {
                        struct addrinfo _hints, *_result = NULL;
                        std::memset(&_hints, 0, sizeof _hints);
                        _hints.ai_family = AF_INET;
                        _hints.ai_socktype = SOCK_DGRAM;

                        _ret = getaddrinfo(__buf.host().c_str(), NULL, &_hints, &_result);
                        if (_ret) {
                            __stream_type::setstate(std::ios::failbit);
                            __buf.set_socket(0);
                            __is_error = true;
                            this->__buf.error_code() = errno;
                            this->__buf.error_string() = std::strerror(errno);
                            return false;
                        }
                        struct sockaddr_in* _host_addr = (struct sockaddr_in*)_result->ai_addr;
                        memcpy(
                          &__buf.server().sin_addr, &_host_addr->sin_addr, sizeof(struct in_addr));
                        freeaddrinfo(_result);
                    }
                    __buf.set_socket(_sd);
                    return true;
                }
                default: {
                    break;
                }
            }
            return false;
        }
        else {
            int _sd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (::connect(
                  _sd, reinterpret_cast<sockaddr*>(&__buf.server()), sizeof __buf.server()) < 0) {
                __stream_type::setstate(std::ios::failbit);
                __buf.set_socket(0);
                __is_error = true;
                this->__buf.error_code() = errno;
                this->__buf.error_string() = std::strerror(errno);
                return false;
            }
            else {
                SSL_library_init();
                OpenSSL_add_all_algorithms();
                SSL_load_error_strings();
                SSL_CTX* _context = SSL_CTX_new(SSLv23_method());
                if (_context == nullptr) {
                    __stream_type::setstate(std::ios::failbit);
                    __buf.set_socket(0);
                    __is_error = true;
                    this->__buf.error_code() = ERR_get_error();
                    this->__buf.error_string() = zpt::ssl_error_print(this->__buf.error_code());
                    return false;
                }
                else {
                    this->assign(_sd, _context);
                }
            }
            return true;
        }
    }
};

// using socketstream = basic_socketstream<char>;
// using wsocketstream = basic_socketstream<wchar_t>;

class socketstream {
  public:
    typedef std::ostream& (*ostream_manipulator)(std::ostream&);

    socketstream()
      : __underlying{ std::make_shared<zpt::basic_socketstream<char>>() } {}
    socketstream(int _fd, bool _ssl = false, short _protocol = IPPROTO_IP)
      : __underlying{ std::make_shared<zpt::basic_socketstream<char>>(_fd, _ssl, _protocol) } {}
    socketstream(const socketstream& _rhs) { (*this) = _rhs; }
    socketstream(socketstream&& _rhs) { (*this) = _rhs; }
    virtual ~socketstream() = default;

    auto operator=(const socketstream& _rhs) -> socketstream& {
        this->__underlying = _rhs.__underlying;
        return (*this);
    }
    auto operator=(socketstream&& _rhs) -> socketstream& {
        this->__underlying = std::move(_rhs.__underlying);
        return (*this);
    }

    template<typename T>
    auto operator>>(T& _out) -> socketstream& {
        (*this->__underlying.get()) >> _out;
        return (*this);
    }
    template<typename T>
    auto operator<<(T _in) -> socketstream& {
        (*this->__underlying.get()) << _in;
        return (*this);
    }
    auto operator<<(ostream_manipulator _in) -> socketstream& {
        (*this->__underlying.get()) << _in;
        return (*this);
    }

    auto operator-> () -> std::shared_ptr<zpt::basic_socketstream<char>>& {
        return this->__underlying;
    }
    auto operator*() -> std::shared_ptr<zpt::basic_socketstream<char>>& {
        return this->__underlying;
    }

  private:
    std::shared_ptr<zpt::basic_socketstream<char>> __underlying;
};

class wsocketstream {
  public:
    typedef std::wostream& (*ostream_manipulator)(std::wostream&);

    wsocketstream()
      : __underlying{ std::make_shared<zpt::basic_socketstream<wchar_t>>() } {}
    wsocketstream(int _fd, bool _ssl = false, short _protocol = IPPROTO_IP)
      : __underlying{ std::make_shared<zpt::basic_socketstream<wchar_t>>(_fd, _ssl, _protocol) } {}
    wsocketstream(const wsocketstream& _rhs) { (*this) = _rhs; }
    wsocketstream(wsocketstream&& _rhs) { (*this) = _rhs; }
    virtual ~wsocketstream() = default;

    auto operator=(const wsocketstream& _rhs) -> wsocketstream& {
        this->__underlying = _rhs.__underlying;
        return (*this);
    }
    auto operator=(wsocketstream&& _rhs) -> wsocketstream& {
        this->__underlying = std::move(_rhs.__underlying);
        return (*this);
    }

    template<typename T>
    auto operator>>(T& _out) -> wsocketstream& {
        (*this->__underlying.get()) >> _out;
        return (*this);
    }
    template<typename T>
    auto operator<<(T _in) -> wsocketstream& {
        (*this->__underlying.get()) << _in;
        return (*this);
    }
    auto operator<<(ostream_manipulator _in) -> wsocketstream& {
        (*this->__underlying.get()) << _in;
        return (*this);
    }

    auto operator-> () -> std::shared_ptr<zpt::basic_socketstream<wchar_t>>& {
        return this->__underlying;
    }
    auto operator*() -> std::shared_ptr<zpt::basic_socketstream<wchar_t>>& {
        return this->__underlying;
    }

  private:
    std::shared_ptr<zpt::basic_socketstream<wchar_t>> __underlying;
};

template<typename Char>
class basic_serversocketstream {
  protected:
    int __sockfd;

  public:
    basic_serversocketstream()
      : __sockfd{ 0 } {};

    basic_serversocketstream(uint16_t _port)
      : __sockfd{ 0 } {
        this->bind(_port);
    }
    virtual ~basic_serversocketstream() { this->close(); }

    auto close() -> void {
        ::shutdown(this->__sockfd, SHUT_RDWR);
        ::close(this->__sockfd);
        this->__sockfd = 0;
    }

    auto is_open() -> bool { return __sockfd != 0; }

    auto ready() -> bool {
        fd_set sockset;
        FD_ZERO(&sockset);
        FD_SET(__sockfd, &sockset);
        return select(__sockfd + 1, &sockset, nullptr, nullptr, nullptr) == 1;
    }

    auto bind(uint16_t _port) -> bool {
        this->__sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->__sockfd < 0) {
            return false;
        }

        int _opt = 1;
        if (setsockopt(this->__sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&_opt, sizeof _opt) ==
            SO_ERROR) {
            ::shutdown(this->__sockfd, SHUT_RDWR);
            ::close(this->__sockfd);
            this->__sockfd = 0;
            return false;
        }

        struct sockaddr_in _serv_addr;
        bzero((char*)&_serv_addr, sizeof _serv_addr);
        _serv_addr.sin_family = AF_INET;
        _serv_addr.sin_addr.s_addr = INADDR_ANY;
        _serv_addr.sin_port = htons(_port);
        if (::bind(this->__sockfd, (struct sockaddr*)&_serv_addr, sizeof _serv_addr) < 0) {
            ::shutdown(this->__sockfd, SHUT_RDWR);
            ::close(this->__sockfd);
            this->__sockfd = 0;
            return false;
        }
        ::listen(this->__sockfd, 100);
        return true;
    }

    auto accept() -> zpt::socketstream {
        expect(this->__sockfd != -1, "server socket file descriptor is invalid", 500, 0);
        struct sockaddr_in* _cli_addr = new struct sockaddr_in();
        socklen_t _clilen = sizeof(struct sockaddr_in);
        int _newsockfd = ::accept(this->__sockfd, (struct sockaddr*)_cli_addr, &_clilen);

        expect(_newsockfd > 0, "accepted file descriptor is invalid", 500, 0);

        struct linger _so_linger;
        _so_linger.l_onoff = 1;
        _so_linger.l_linger = 30;
        ::setsockopt(_newsockfd, SOL_SOCKET, SO_LINGER, &_so_linger, sizeof _so_linger);
        return zpt::socketstream{ _newsockfd };
    }
};

using serversocketstream = basic_serversocketstream<char>;
using wserversocketstream = basic_serversocketstream<wchar_t>;

#define CRLF "\r\n"
} // namespace zpt
