/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file socket_stream.h
 * @brief TCP/Unix socket stream buffer and iostream wrapper.
 *
 * Provides std::iostream-compatible streams backed by TCP sockets (with
 * optional SSL/TLS) and Unix domain sockets. Includes both client
 * (socketstream) and server (serversocketstream) variants.
 *
 * @see zpt::socketstream
 * @see zpt::serversocketstream
 */

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
#include <sys/un.h>
#include <unistd.h>
#include <zapata/base/expect.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/log/log.h>
#include <zapata/streams.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

#define UNIXPROTO_RAW -2

namespace zpt {
using sockaddr_t = struct sockaddr;
using sockaddrin_t = struct sockaddr_in;
using sockaddrun_t = struct sockaddr_un;

constexpr char const* ADDR_ANONYMOUS = "";
constexpr bool NO_SSL = false;
constexpr bool USE_SSL = true;

/** @brief Returns a human-readable SSL error description. */
auto ssl_error_print(SSL* _ssl, int _ret) -> std::string;
/** @brief Returns a human-readable SSL error description for a given error code. */
auto ssl_error_print(unsigned long _error = 0) -> std::string;
/** @brief Tests if an IP address is a multicast address. */
auto is_multicast_address(std::string const& _ip) -> bool;
auto bind_to_address(zpt::sockaddrin_t& _to_bind, std::string const& _address) -> bool;

/**
 * @brief Stream buffer backed by a network socket.
 *
 * Wraps a TCP, UDP, or Unix domain socket file descriptor as a
 * std::basic_streambuf, enabling standard C++ stream I/O over sockets.
 * Supports optional SSL/TLS encryption.
 *
 * @tparam Char Character type (typically char or wchar_t).
 *
 * @see zpt::basic_socketstream
 */
template<typename Char>
class basic_socketbuf : public std::basic_streambuf<Char> {
  public:
    using __char_type = Char;
    using __buf_type = std::basic_streambuf<__char_type>;
    using __stream_type = std::basic_ostream<__char_type>;
    using __int_type = typename __buf_type::int_type;
    using __traits_type = typename std::basic_streambuf<Char>::traits_type;

    basic_socketbuf();
    virtual ~basic_socketbuf();

    /** @brief Returns the underlying socket file descriptor. */
    auto get_socket() -> int;
    /** @brief Sets the socket file descriptor and configures socket options. */
    auto set_socket(int _sock) -> void;
    /** @brief Configures SSL/TLS context and initiates handshake. */
    auto set_context(SSL_CTX* _ctx) -> void;
    /** @brief Sets the protocol (IPPROTO_TCP, IPPROTO_UDP, or UNIXPROTO_RAW). */
    auto set_protocol(short _protocol) -> void;

    /** @brief Returns the local socket address. */
    auto address() -> zpt::sockaddr_t&;
    /** @brief Returns the peer socket address (UDP only). */
    auto peer() -> zpt::sockaddr_t&;
    /** @brief Returns whether SSL is enabled. */
    auto ssl() -> bool&;
    /** @brief Returns the remote hostname or Unix socket path. */
    auto host() -> std::string&;
    /** @brief Returns the remote port number. */
    auto port() -> int&;
    /** @brief Returns the socket protocol. */
    auto protocol() -> short;
    /** @brief Returns the socket timeout in milliseconds. */
    auto timeout() -> unsigned long long&;

    /** @brief Returns the last error code. */
    auto error_code() -> unsigned int&;
    /** @brief Returns the last error description. */
    auto error_string() -> std::string&;

    /** @brief Returns true if the socket is in a valid state. */
    virtual auto __good() -> bool;

  protected:
    static constexpr int char_size = sizeof(__char_type);
    static constexpr int SIZE = 4096;
    __char_type obuf[SIZE] = { 0 };
    __char_type ibuf[SIZE] = { 0 };

    int __sock{ -1 };
    bool __ssl{ false };
    std::unique_ptr<zpt::sockaddr_t> __server{ nullptr };
    std::unique_ptr<zpt::sockaddr_t> __peer{ nullptr };
    std::string __host;
    int __port{ -1 };
    short __protocol{ -1 };
    SSL* __sslstream{ nullptr };
    SSL_CTX* __context{ nullptr };
    unsigned long long __timeout{ 0 };
    unsigned int __error_code{ 0 };
    std::string __error_string;

    virtual auto output_buffer() -> __int_type;
    virtual auto overflow(__int_type c) -> __int_type override;
    virtual auto sync() -> int override;
    virtual auto underflow() -> __int_type override;

  private:
    auto output_buffer_ip() -> __int_type;
    auto output_buffer_udp() -> __int_type;
    auto output_buffer_ssl() -> __int_type;
    auto underflow_ip() -> __int_type;
    auto underflow_udp() -> __int_type;
    auto underflow_ssl() -> __int_type;
    auto report_error() -> void;
    auto report_ssl_error() -> void;
};

using socketbuf = basic_socketbuf<char>;
using wsocketbuf = basic_socketbuf<wchar_t>;

/**
 * @brief iostream wrapper around a network socket.
 *
 * Provides bidirectional stream I/O over TCP, UDP, or Unix domain sockets
 * with optional SSL/TLS encryption. Can connect to remote hosts or wrap
 * an existing socket file descriptor.
 *
 * @tparam Char Character type (typically char or wchar_t).
 *
 * @par Example
 * @code
 * // TCP connection
 * zpt::socketstream sock("example.com", 8080, zpt::NO_SSL, IPPROTO_TCP);
 * sock << "GET / HTTP/1.1\r\n\r\n" << std::flush;
 *
 * // Unix domain socket
 * zpt::socketstream unix_sock("/var/run/app.sock");
 * @endcode
 *
 * @see zpt::basic_serversocketstream
 */
template<typename Char>
class basic_socketstream : public std::basic_iostream<Char> {
  public:
    using __char_type = Char;
    using __stream_type = std::basic_iostream<__char_type>;
    using __buf_type = basic_socketbuf<__char_type>;

    /** @brief Default constructor (unconnected). */
    basic_socketstream();
    /** @brief Wraps an existing TCP socket with address info. */
    basic_socketstream(int s, zpt::sockaddrin_t const& _address, bool _ssl, short _protocol);
    /** @brief Connects to a remote host. */
    basic_socketstream(std::string const& _host, std::uint16_t _port, bool _ssl, short _protocol);
    /** @brief Creates a UDP client socket. */
    basic_socketstream(bool _ssl, short _protocol);
    /** @brief Wraps an existing Unix domain socket. */
    basic_socketstream(int s, zpt::sockaddrun_t const& _address);
    /** @brief Connects to a Unix domain socket by path. */
    basic_socketstream(std::string const& _path);
    basic_socketstream(const basic_socketstream&) = delete;
    basic_socketstream(basic_socketstream&&) = delete;
    virtual ~basic_socketstream();

    auto operator=(const basic_socketstream&) -> basic_socketstream& = delete;
    auto operator=(basic_socketstream&&) -> basic_socketstream& = delete;

    /** @brief Returns the socket file descriptor. */
    operator int();
    /** @brief Returns a URI representation (e.g., "tcp://host:port"). */
    operator std::string();

    /** @brief Sets the peer address for UDP communication. */
    auto set_peer(std::string const& address, int port) -> void;

    /** @brief Returns whether SSL is enabled. */
    auto ssl() -> bool&;
    /** @brief Returns the remote hostname. */
    auto host() -> std::string&;
    /** @brief Returns the remote port. */
    auto port() -> int&;
    /** @brief Returns the socket protocol. */
    auto protocol() -> short;

    /** @brief Assigns a raw socket file descriptor (no SSL). */
    auto assign(int _sockfd) -> void;
    /** @brief Assigns a raw socket file descriptor with SSL context. */
    auto assign(int _sockfd, SSL_CTX* _ctx) -> void;
    /** @brief Detaches from the current socket. */
    auto unassign() -> void;

    /** @brief Closes the socket connection. */
    auto close() -> void;
    /** @brief Returns true if the socket is open and valid. */
    auto is_open() -> bool;
    /** @brief Returns true if data is available for reading. */
    auto ready() -> bool;

    /** @brief Returns the underlying stream buffer. */
    auto buffer() -> __buf_type&;
    /** @brief Returns true if an error has occurred. */
    auto is_error() -> bool;
    /** @brief Returns the last error code. */
    auto error_code() -> unsigned int&;
    /** @brief Returns the last error description. */
    auto error_string() -> std::string&;

    /**
     * @brief Opens a TCP/UDP connection to a remote host.
     * @param _host Hostname or IP address.
     * @param _port Port number.
     * @param _ssl Whether to use SSL/TLS.
     * @param _protocol IPPROTO_TCP or IPPROTO_UDP.
     * @return True on success.
     */
    auto open(std::string const& _host,
              std::uint16_t _port,
              bool _ssl = false,
              short _protocol = IPPROTO_TCP) -> bool;
    /**
     * @brief Opens a Unix domain socket connection.
     * @param _path Filesystem path to the socket.
     * @return True on success.
     */
    auto open(std::string const& _path) -> bool;

  protected:
    __buf_type __buf;
    bool __is_error{ false };
    bool __is_accepted{ false };

  private:
    auto open_ip() -> bool;
    auto open_udp() -> bool;
    auto open_ssl() -> bool;
    auto report_error() -> void;
    auto report_ssl_error() -> void;
    auto extract_ip() -> void;
};

using socketstream = zpt::basic_socketstream<char>;
using wsocketstream = zpt::basic_socketstream<wchar_t>;

/**
 * @brief Server-side socket that listens for incoming connections.
 *
 * Binds to a TCP port or Unix domain socket path and accepts incoming
 * client connections, returning them as zpt::stream instances.
 *
 * @tparam Char Character type (typically char or wchar_t).
 *
 * @par Example
 * @code
 * zpt::serversocketstream server(8080);
 * while (server->is_open()) {
 *     auto client = server->accept();
 *     // Handle client connection...
 * }
 * @endcode
 *
 * @see zpt::basic_socketstream
 */
template<typename Char>
class basic_serversocketstream {
  public:
    /** @brief Default constructor (unbound). */
    basic_serversocketstream();
    /** @brief Binds to a TCP port. */
    basic_serversocketstream(std::string const& _transport,
                             std::string const& _address,
                             std::uint16_t _port);
    /** @brief Binds to a Unix domain socket path. */
    basic_serversocketstream(std::string const& _transport, std::string const& _path);
    virtual ~basic_serversocketstream();

    /** @brief Returns a URI representation (e.g., "tcp://host:port"). */
    operator std::string();
    /** @brief Closes the server socket. */
    auto close() -> void;
    /** @brief Returns true if the server socket is open. */
    auto is_open() -> bool;
    /** @brief Returns true if a connection is pending. */
    auto ready() -> bool;
    /**
     * @brief Binds to a TCP port and starts listening.
     * @param _port Port number to bind to.
     * @return True on success.
     */
    auto bind(std::string const& _address, std::uint16_t _port) -> bool;
    /**
     * @brief Binds to a Unix domain socket path and starts listening.
     * @param _path Filesystem path for the socket.
     * @return True on success.
     */
    auto bind(std::string const& _path) -> bool;
    /**
     * @brief Accepts an incoming connection.
     * @return Stream wrapping the new client connection.
     * @throws zpt::ClosedException If the server socket is closed.
     */
    auto accept() -> zpt::stream;

  protected:
    int __sockfd{ -1 };
    short __protocol{ -1 };
    std::string __address{ "" };
    std::string __path{ "" };
    std::uint16_t __port{ 0 };
    std::string __transport{ "" };
};

/**
 * @brief Shared-pointer wrapper for basic_serversocketstream<char>.
 *
 * Provides copyable/movable semantics for server sockets via
 * internal shared_ptr ownership.
 */
class serversocketstream {
  public:
    serversocketstream();
    /** @brief Binds to a TCP port. */
    serversocketstream(std::string const& _transport,
                       std::string const& _address,
                       std::uint16_t _port);
    /** @brief Binds to a Unix domain socket path. */
    serversocketstream(std::string const& _transport, std::string const& _path);
    serversocketstream(const serversocketstream& _rhs);
    serversocketstream(serversocketstream&& _rhs);
    virtual ~serversocketstream() = default;

    auto operator=(const zpt::serversocketstream& _rhs) -> zpt::serversocketstream&;
    auto operator=(zpt::serversocketstream&& _rhs) -> zpt::serversocketstream&;

    /** @brief Access the underlying server socket. */
    auto operator->() -> zpt::basic_serversocketstream<char>*;
    /** @brief Dereference the underlying server socket. */
    auto operator*() -> zpt::basic_serversocketstream<char>&;

  private:
    std::shared_ptr<zpt::basic_serversocketstream<char>> __underlying;
};

/**
 * @brief Shared-pointer wrapper for basic_serversocketstream<wchar_t>.
 *
 * Wide-character variant of serversocketstream.
 */
class wserversocketstream {
  public:
    wserversocketstream();
    /** @brief Binds to a TCP port. */
    wserversocketstream(std::string const& _transport,
                        std::string const& _address,
                        std::uint16_t _port);
    /** @brief Binds to a Unix domain socket path. */
    wserversocketstream(std::string const& _transport, std::string const& _path);
    wserversocketstream(const zpt::wserversocketstream& _rhs);
    wserversocketstream(zpt::wserversocketstream&& _rhs);
    virtual ~wserversocketstream() = default;

    auto operator=(const zpt::wserversocketstream& _rhs) -> zpt::wserversocketstream&;
    auto operator=(zpt::wserversocketstream&& _rhs) -> zpt::wserversocketstream&;

    /** @brief Access the underlying server socket. */
    auto operator->() -> zpt::basic_serversocketstream<wchar_t>*;
    /** @brief Dereference the underlying server socket. */
    auto operator*() -> zpt::basic_serversocketstream<wchar_t>&;

  private:
    std::shared_ptr<zpt::basic_serversocketstream<wchar_t>> __underlying;
};

#define CRLF "\r\n"
} // namespace zpt

template<typename Char>
zpt::basic_socketbuf<Char>::basic_socketbuf()
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
}

template<typename Char>
zpt::basic_socketbuf<Char>::~basic_socketbuf() {
    this->sync();
    if (this->__sslstream != nullptr) {
        SSL_free(this->__sslstream);
        SSL_CTX_free(this->__context);
    }
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::set_socket(int _sock) -> void {
    this->__sock = _sock;
    if (_sock != 0) {
        auto iOption = 1;
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

template<typename Char>
auto zpt::basic_socketbuf<Char>::get_socket() -> int {
    return this->__sock;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::set_context(SSL_CTX* _ctx) -> void {
    this->__ssl = true;
    this->__context = _ctx;
    this->__sslstream = SSL_new(_ctx);
    SSL_set_tlsext_host_name(this->__sslstream, this->__host.data());
    SSL_set_fd(this->__sslstream, this->__sock);
    SSL_connect(this->__sslstream);
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::set_protocol(short _protocol) -> void {
    this->__protocol = _protocol;
    switch (this->__protocol) {
        case IPPROTO_UDP: {
            this->__peer.reset((zpt::sockaddr_t*)new zpt::sockaddrin_t());
            [[fallthrough]];
        }
        case IPPROTO_TCP: {
            this->__server.reset((zpt::sockaddr_t*)new zpt::sockaddrin_t());
            break;
        }
        case UNIXPROTO_RAW: {
            this->__server.reset((zpt::sockaddr_t*)new zpt::sockaddrun_t());
            break;
        }
    }
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::address() -> zpt::sockaddr_t& {
    return *this->__server.get();
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::peer() -> zpt::sockaddr_t& {
    return *this->__peer.get();
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::ssl() -> bool& {
    return this->__ssl;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::host() -> std::string& {
    return this->__host;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::port() -> int& {
    return this->__port;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::protocol() -> short {
    return this->__protocol;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::timeout() -> unsigned long long& {
    return this->__timeout;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::error_code() -> unsigned int& {
    return this->__error_code;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::error_string() -> std::string& {
    return this->__error_string;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::__good() -> bool {
    return this->__sock != 0 && (!this->__ssl || (this->__ssl && this->__sslstream != nullptr &&
                                                  this->__context != nullptr));
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::output_buffer() -> __int_type {
    if (!__good()) { return __traits_type::eof(); }

    if (!this->__ssl) {
        switch (protocol()) {
            case IPPROTO_TCP: {
                return this->output_buffer_ip();
            }
            case IPPROTO_UDP: {
                return this->output_buffer_udp();
            }
            case UNIXPROTO_RAW: {
                return this->output_buffer_ip();
            }
            default: {
                return __traits_type::eof();
            }
        }
    }
    else { return this->output_buffer_ssl(); }
    return __traits_type::eof();
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::overflow(__int_type c) -> __int_type {
    if (c != __traits_type::eof()) {
        *__buf_type::pptr() = c;
        __buf_type::pbump(1);
    }

    if (this->output_buffer() == __traits_type::eof()) { return __traits_type::eof(); }
    return c;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::sync() -> int {
    if (output_buffer() == __traits_type::eof()) { return __traits_type::eof(); }
    return 0;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::underflow() -> __int_type {
    if (__buf_type::gptr() < __buf_type::egptr()) { return *__buf_type::gptr(); }

    if (!__good()) { return __traits_type::eof(); }

    if (!this->__ssl) {
        switch (protocol()) {
            case IPPROTO_TCP: {
                return this->underflow_ip();
            }
            case IPPROTO_UDP: {
                return this->underflow_udp();
            }
            case UNIXPROTO_RAW: {
                return this->underflow_ip();
            }
        }
    }
    else { return this->underflow_ssl(); }
    return __traits_type::eof();
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::output_buffer_ip() -> __int_type {
    auto _num = __buf_type::pptr() - __buf_type::pbase();
    if (_num == 0) { return 0; }
    auto _actually_written = -1;
    if ((_actually_written =
           ::send(__sock, reinterpret_cast<char*>(obuf), _num * char_size, MSG_NOSIGNAL)) < 0) {
        ::shutdown(this->__sock, SHUT_RDWR);
        ::close(this->__sock);
        this->report_error();
    }
    __buf_type::pbump(-_actually_written);
    return _actually_written;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::output_buffer_udp() -> __int_type {
    auto _num = __buf_type::pptr() - __buf_type::pbase();
    auto _actually_written = -1;
    if ((_actually_written = ::sendto(__sock,
                                      reinterpret_cast<char*>(obuf),
                                      _num * char_size,
                                      0,
                                      (struct sockaddr*)this->__peer.get(),
                                      sizeof(*this->__peer.get()))) < 0) {
        ::shutdown(this->__sock, SHUT_RDWR);
        ::close(this->__sock);
        this->report_error();
    }
    __buf_type::pbump(-_actually_written);
    return _actually_written;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::output_buffer_ssl() -> __int_type {
    auto _num = __buf_type::pptr() - __buf_type::pbase();
    auto _actually_written = 0;
    do {
        if ((_actually_written =
               SSL_write(this->__sslstream, reinterpret_cast<char*>(obuf), _num * char_size)) < 0) {
            if (SSL_get_error(this->__sslstream, _actually_written) != SSL_ERROR_WANT_WRITE) {
                SSL_free(this->__sslstream);
                SSL_CTX_free(this->__context);
                ::shutdown(this->__sock, SHUT_RDWR);
                ::close(this->__sock);
                this->report_ssl_error();
            }
        }
    } while (SSL_get_error(this->__sslstream, _actually_written) == SSL_ERROR_WANT_WRITE);
    __buf_type::pbump(-_actually_written);
    return _actually_written;
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::underflow_ip() -> __int_type {
    auto _actually_read = -1;
    if ((_actually_read = ::recv(__sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, 0)) < 0) {
        ::shutdown(this->__sock, SHUT_RDWR);
        ::close(this->__sock);
        this->report_error();
    }
    if (_actually_read == 0) { return __traits_type::eof(); }
    __buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
    return *__buf_type::gptr();
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::underflow_udp() -> __int_type {
    auto _actually_read = -1;
    socklen_t _peer_addr_len = sizeof(*this->__peer.get());
    if ((_actually_read = ::recvfrom(__sock,
                                     reinterpret_cast<char*>(ibuf),
                                     SIZE * char_size,
                                     0,
                                     (struct sockaddr*)this->__peer.get(),
                                     &_peer_addr_len)) < 0) {
        ::shutdown(this->__sock, SHUT_RDWR);
        ::close(this->__sock);
        this->report_error();
    }
    if (_actually_read == 0) { return __traits_type::eof(); }
    __buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
    return *__buf_type::gptr();
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::underflow_ssl() -> __int_type {
    auto _actually_read = -1;
    do {
        if ((_actually_read =
               SSL_read(this->__sslstream, reinterpret_cast<char*>(ibuf), SIZE * char_size)) < 0) {
            if (SSL_get_error(this->__sslstream, _actually_read) != SSL_ERROR_WANT_READ) {
                SSL_free(this->__sslstream);
                SSL_CTX_free(this->__context);
                ::shutdown(this->__sock, SHUT_RDWR);
                ::close(this->__sock);
                this->report_ssl_error();
            }
        }
    } while (SSL_get_error(this->__sslstream, _actually_read) == SSL_ERROR_WANT_READ);
    if (_actually_read == 0) { return __traits_type::eof(); }
    __buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
    return *__buf_type::gptr();
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::report_error() -> void {
    this->__sock = 0;
    this->__error_code = errno;
    this->__error_string = std::string(std::strerror(errno));
    zlog(this->__error_string, zpt::error);
    throw zpt::ClosedException(this->__error_string);
}

template<typename Char>
auto zpt::basic_socketbuf<Char>::report_ssl_error() -> void {
    this->__sock = 0;
    this->__sslstream = nullptr;
    this->__context = nullptr;
    this->__error_code = ERR_get_error();
    this->__error_string = zpt::ssl_error_print(this->__error_code);
    throw zpt::ClosedException(this->__error_string);
}

template<typename Char>
zpt::basic_socketstream<Char>::basic_socketstream()
  : __stream_type(&__buf)
  , __is_error(false) {}

template<typename Char>
zpt::basic_socketstream<Char>::basic_socketstream(int s,
                                                  zpt::sockaddrin_t const& _address,
                                                  bool _ssl,
                                                  short _protocol)
  : __stream_type(&__buf)
  , __is_error(false) {
    this->__buf.set_socket(s);
    this->__buf.set_protocol(_protocol);
    this->__buf.ssl() = _ssl;
    auto& _in_addr = reinterpret_cast<zpt::sockaddrin_t&>(__buf.address());
    _in_addr.sin_family = _address.sin_family;
    _in_addr.sin_port = _address.sin_port;
    _in_addr.sin_addr.s_addr = _address.sin_addr.s_addr;
    this->extract_ip();
    this->__is_accepted = true;
}

template<typename Char>
zpt::basic_socketstream<Char>::basic_socketstream(std::string const& _host,
                                                  std::uint16_t _port,
                                                  bool _ssl,
                                                  short _protocol)
  : __stream_type(&__buf)
  , __is_error(false) {
    this->open(_host, _port, _ssl, _protocol);
}

template<typename Char>
zpt::basic_socketstream<Char>::basic_socketstream(bool _ssl, short _protocol)
  : __stream_type(&__buf)
  , __is_error(false) {
    expect(_protocol == IPPROTO_UDP,
           "creating a socket with no pre-defined address and port must for UDP client");
    this->open("", 0, _ssl, _protocol);
}

template<typename Char>
zpt::basic_socketstream<Char>::basic_socketstream(int s, zpt::sockaddrun_t const& _address)
  : __stream_type(&__buf)
  , __is_error(false) {
    this->__buf.set_socket(s);
    this->__buf.set_protocol(UNIXPROTO_RAW);
    this->__buf.ssl() = false;
    auto& _in_addr = reinterpret_cast<zpt::sockaddrun_t&>(__buf.address());
    bzero((char*)&_in_addr, sizeof _in_addr);
    _in_addr.sun_family = AF_UNIX;
    strncpy(_in_addr.sun_path, _address.sun_path, sizeof _in_addr.sun_path);
}

template<typename Char>
zpt::basic_socketstream<Char>::basic_socketstream(std::string const& _path)
  : __stream_type(&__buf)
  , __is_error(false) {
    this->open(_path);
}

template<typename Char>
zpt::basic_socketstream<Char>::~basic_socketstream() {
    this->close();
}

template<typename Char>
zpt::basic_socketstream<Char>::operator int() {
    return this->__buf.get_socket();
}

template<typename Char>
zpt::basic_socketstream<Char>::operator std::string() {
    std::ostringstream _oss;
    switch (this->__buf.protocol()) {
        case IPPROTO_TCP: {
            _oss << "tcp" << (this->__buf.ssl() ? "+ssl" : "") << "://"
                 << (this->__is_accepted ? std::format("{}@", this->__buf.get_socket()) : "")
                 << this->host() << ":" << this->port();
            break;
        }
        case IPPROTO_UDP: {
            _oss << "udp" << (this->__buf.ssl() ? "+ssl" : "") << "://" << this->host() << ":"
                 << this->port();
            break;
        }
        case UNIXPROTO_RAW: {
            _oss << "unix:" << this->host()
                 << (this->__is_accepted ? std::format("@{}", this->__buf.get_socket()) : "");
            break;
        }
        default: {
            _oss << "raw";
        }
    }
    _oss << std::flush;
    return _oss.str();
}

template<typename Char>
auto zpt::basic_socketstream<Char>::set_peer(std::string const& _address, int _port) -> void {
    auto& _peer = reinterpret_cast<zpt::sockaddrin_t&>(this->__buf.peer());

    ::hostent* _he = gethostbyname(_address.c_str());
    expect(_he != nullptr, "couldn't translate the provided address");

    std::string _addr{ reinterpret_cast<char*>(_he->h_addr), static_cast<size_t>(_he->h_length) };
    std::copy(_addr.c_str(),
              _addr.c_str() + _addr.length(),
              reinterpret_cast<char*>(&_peer.sin_addr.s_addr));
    _peer.sin_family = AF_INET;
    _peer.sin_port = htons(_port);
}

template<typename Char>
auto zpt::basic_socketstream<Char>::ssl() -> bool& {
    return this->__buf.ssl();
}

template<typename Char>
auto zpt::basic_socketstream<Char>::host() -> std::string& {
    return this->__buf.host();
}

template<typename Char>
auto zpt::basic_socketstream<Char>::port() -> int& {
    return this->__buf.port();
}

template<typename Char>
auto zpt::basic_socketstream<Char>::protocol() -> short {
    return this->__buf.protocol();
}

template<typename Char>
auto zpt::basic_socketstream<Char>::assign(int _sockfd) -> void {
    this->__buf.set_socket(_sockfd);
    this->__buf.ssl() = false;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::assign(int _sockfd, SSL_CTX* _ctx) -> void {
    this->__buf.set_socket(_sockfd);
    this->__buf.set_context(_ctx);
    this->__buf.ssl() = true;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::unassign() -> void {
    this->__buf.set_socket(0);
}

template<typename Char>
auto zpt::basic_socketstream<Char>::close() -> void {
    if (this->__buf.get_socket() != 0) {
        ::shutdown(this->__buf.get_socket(), SHUT_RDWR);
        ::close(this->__buf.get_socket());
    }
    this->__buf.set_socket(0);
}

template<typename Char>
auto zpt::basic_socketstream<Char>::is_open() -> bool {
    return (!__is_error && this->__buf.get_socket() != 0 && this->__buf.__good());
}

template<typename Char>
auto zpt::basic_socketstream<Char>::ready() -> bool {
    fd_set sockset;
    FD_ZERO(&sockset);
    FD_SET(this->__buf.get_socket(), &sockset);
    return select(this->__buf.get_socket() + 1, &sockset, nullptr, nullptr, nullptr) == 1;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::buffer() -> __buf_type& {
    return (*this->__buf.get());
}

template<typename Char>
auto zpt::basic_socketstream<Char>::is_error() -> bool {
    return this->__buf.error_code() != 0;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::error_code() -> unsigned int& {
    return this->__buf.error_code();
}

template<typename Char>
auto zpt::basic_socketstream<Char>::error_string() -> std::string& {
    return this->__buf.error_string();
}

template<typename Char>
auto zpt::basic_socketstream<Char>::open(std::string const& _host,
                                         std::uint16_t _port,
                                         bool _ssl,
                                         short _protocol) -> bool {
    if (this->is_open()) { this->close(); }
    this->__buf.host() = _host;
    this->__buf.port() = _port;
    this->__buf.set_protocol(_protocol);
    this->__buf.ssl() = _ssl;

    auto& _in_address = reinterpret_cast<zpt::sockaddrin_t&>(this->__buf.address());
    _in_address.sin_family = AF_INET;
    _in_address.sin_addr.s_addr = htonl(INADDR_ANY);
    _in_address.sin_port = htons(_port);

    if (!_ssl) {
        switch (_protocol) {
            case IPPROTO_TCP: {
                return this->open_ip();
            }
            case IPPROTO_UDP: {
                return this->open_udp();
            }
            default: {
                break;
            }
        }
        return false;
    }
    else { return this->open_ssl(); }
}

template<typename Char>
auto zpt::basic_socketstream<Char>::open(std::string const& _path) -> bool {
    if (this->is_open()) { this->close(); }
    this->__buf.host() = _path;
    this->__buf.set_protocol(UNIXPROTO_RAW);
    this->__buf.ssl() = false;

    auto& _in_address = reinterpret_cast<zpt::sockaddrun_t&>(this->__buf.address());
    bzero((char*)&_in_address, sizeof _in_address);
    _in_address.sun_family = AF_UNIX;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(_in_address.sun_path, _path.data(), sizeof _in_address.sun_path);
#pragma GCC diagnostic pop

    auto _sd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (::connect(_sd, &this->__buf.address(), sizeof this->__buf.address()) < 0) {
        this->report_error();
    }
    else { this->__buf.set_socket(_sd); }
    return true;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::open_ip() -> bool {
    auto& _in_address = reinterpret_cast<zpt::sockaddrin_t&>(this->__buf.address());
    zpt::bind_to_address(_in_address, this->__buf.host());

    auto _sd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (::connect(_sd, &this->__buf.address(), sizeof this->__buf.address()) < 0) {
        this->report_error();
        return false;
    }
    this->__buf.set_socket(_sd);
    return true;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::open_udp() -> bool {
    auto _sd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int _reuse = 1;
    setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, &_reuse, sizeof _reuse);
    int _broadcast_enable = 1;
    setsockopt(_sd, SOL_SOCKET, SO_BROADCAST, &_broadcast_enable, sizeof(_broadcast_enable));
    int _buffer_len = 1024 * 512;
    setsockopt(_sd, SOL_SOCKET, SO_RCVBUF, &_buffer_len, sizeof(_buffer_len));

    if (this->__buf.host() != zpt::ADDR_ANONYMOUS) {
        auto& _in_address = reinterpret_cast<zpt::sockaddrin_t&>(this->__buf.address());
        auto _is_multicast = zpt::is_multicast_address(this->__buf.host());

        if (::bind(_sd, reinterpret_cast<zpt::sockaddr_t*>(&_in_address), sizeof(_in_address)) <
            0) {
            ::shutdown(_sd, SHUT_RDWR);
            ::close(_sd);
            this->report_error();
        }

        if (_is_multicast) {
            struct ip_mreq _mreq;
            _mreq.imr_multiaddr.s_addr = inet_addr(this->__buf.host().data());
            _mreq.imr_interface.s_addr = _in_address.sin_addr.s_addr;

            if (setsockopt(_sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&_mreq, sizeof(_mreq)) < 0) {
                ::shutdown(_sd, SHUT_RDWR);
                ::close(_sd);
                this->report_error();
            }
        }
    }

    this->__buf.set_socket(_sd);
    return true;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::open_ssl() -> bool {
    auto& _in_address = reinterpret_cast<zpt::sockaddrin_t&>(this->__buf.address());
    zpt::bind_to_address(_in_address, this->__buf.host());

    auto _sd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (::connect(_sd,
                  reinterpret_cast<sockaddr*>(&this->__buf.address()),
                  sizeof this->__buf.address()) < 0) {
        this->report_error();
    }
    else {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        SSL_CTX* _context = SSL_CTX_new(SSLv23_method());
        if (_context == nullptr) { this->report_ssl_error(); }
        else { this->assign(_sd, _context); }
    }
    return true;
}

template<typename Char>
auto zpt::basic_socketstream<Char>::report_error() -> void {
    __stream_type::setstate(std::ios::failbit);
    this->__buf.set_socket(0);
    this->__is_error = true;
    this->__buf.error_code() = errno;
    this->__buf.error_string() =
      std::format("{}: {}", std::strerror(errno), static_cast<std::string>(*this));
    throw zpt::ClosedException(this->__buf.error_string());
}

template<typename Char>
auto zpt::basic_socketstream<Char>::report_ssl_error() -> void {
    __stream_type::setstate(std::ios::failbit);
    this->__buf.set_socket(0);
    this->__is_error = true;
    this->__buf.error_code() = ERR_get_error();
    this->__buf.error_string() = zpt::ssl_error_print(this->__buf.error_code());
    throw zpt::ClosedException(this->__buf.error_string());
}

template<typename Char>
auto zpt::basic_socketstream<Char>::extract_ip() -> void {
    struct sockaddr_in _my_addr;
    bzero(&_my_addr, sizeof(_my_addr));
    socklen_t _len = sizeof(_my_addr);
    ::getsockname(this->__buf.get_socket(), (struct sockaddr*)&_my_addr, &_len);

    char _my_ip[16];
    ::inet_ntop(AF_INET, &_my_addr.sin_addr, _my_ip, sizeof(_my_ip));
    auto _my_port = ntohs(_my_addr.sin_port);

    this->__buf.host() = _my_ip;
    this->__buf.port() = _my_port;
}

template<typename Char>
zpt::basic_serversocketstream<Char>::basic_serversocketstream()
  : __sockfd{ 0 } {}

template<typename Char>
zpt::basic_serversocketstream<Char>::basic_serversocketstream(std::string const& _transport,
                                                              std::string const& _address,
                                                              std::uint16_t _port)
  : __sockfd{ 0 }
  , __address{ _address }
  , __transport{ _transport } {
    this->bind(_address, _port);
}

template<typename Char>
zpt::basic_serversocketstream<Char>::basic_serversocketstream(std::string const& _transport,
                                                              std::string const& _path)
  : __sockfd{ 0 }
  , __transport{ _transport } {
    this->bind(_path);
}

template<typename Char>
zpt::basic_serversocketstream<Char>::~basic_serversocketstream() {
    this->close();
}

template<typename Char>
zpt::basic_serversocketstream<Char>::operator std::string() {
    std::ostringstream _oss;
    switch (this->__protocol) {
        case IPPROTO_TCP: {
            _oss << "tcp" << "://" << this->__address << ":" << this->__port;
            break;
        }
        case UNIXPROTO_RAW: {
            _oss << "unix:" << this->__path;
            break;
        }
        default: {
            _oss << "raw";
        }
    }
    _oss << std::flush;
    return _oss.str();
}

template<typename Char>
auto zpt::basic_serversocketstream<Char>::close() -> void {
    ::shutdown(this->__sockfd, SHUT_RDWR);
    ::close(this->__sockfd);
    this->__sockfd = 0;
}

template<typename Char>
auto zpt::basic_serversocketstream<Char>::is_open() -> bool {
    return __sockfd != 0;
}

template<typename Char>
auto zpt::basic_serversocketstream<Char>::ready() -> bool {
    fd_set sockset;
    FD_ZERO(&sockset);
    FD_SET(__sockfd, &sockset);
    return select(__sockfd + 1, &sockset, nullptr, nullptr, nullptr) == 1;
}

template<typename Char>
auto zpt::basic_serversocketstream<Char>::bind(std::string const& _address, std::uint16_t _port)
  -> bool {
    this->__port = _port;
    this->__protocol = IPPROTO_TCP;
    this->__sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (this->__sockfd < 0) { return false; }

    auto _opt = 1;
    if (setsockopt(this->__sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&_opt, sizeof _opt) ==
        SO_ERROR) {
        ::shutdown(this->__sockfd, SHUT_RDWR);
        ::close(this->__sockfd);
        this->__sockfd = 0;
        throw zpt::ClosedException(std::strerror(errno));
    }

    struct sockaddr_in _serv_addr;
    bzero((char*)&_serv_addr, sizeof _serv_addr);
    _serv_addr.sin_family = AF_INET;
    _serv_addr.sin_port = htons(_port);
    if (!zpt::bind_to_address(_serv_addr, _address)) {
        ::shutdown(this->__sockfd, SHUT_RDWR);
        ::close(this->__sockfd);
        this->__sockfd = 0;
        return false;
    }

    if (::bind(this->__sockfd, reinterpret_cast<zpt::sockaddr_t*>(&_serv_addr), sizeof _serv_addr) <
        0) {
        ::shutdown(this->__sockfd, SHUT_RDWR);
        ::close(this->__sockfd);
        this->__sockfd = 0;
        throw zpt::ClosedException(std::strerror(errno));
    }
    ::listen(this->__sockfd, 100);
    return true;
}

template<typename Char>
auto zpt::basic_serversocketstream<Char>::bind(std::string const& _path) -> bool {
    this->__path = _path;
    this->__protocol = UNIXPROTO_RAW;
    this->__sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (this->__sockfd < 0) { return false; }

    struct sockaddr_un _serv_addr;
    bzero((char*)&_serv_addr, sizeof _serv_addr);
    _serv_addr.sun_family = AF_UNIX;
    strncpy(_serv_addr.sun_path, _path.data(), (sizeof _serv_addr.sun_path) - 1);
    if (::bind(this->__sockfd, reinterpret_cast<zpt::sockaddr_t*>(&_serv_addr), sizeof _serv_addr) <
        0) {
        ::shutdown(this->__sockfd, SHUT_RDWR);
        ::close(this->__sockfd);
        this->__sockfd = 0;
        throw zpt::ClosedException(std::strerror(errno));
    }
    ::listen(this->__sockfd, 100);
    return true;
}

template<typename Char>
auto zpt::basic_serversocketstream<Char>::accept() -> zpt::stream {
    expect(this->__sockfd != -1, "server socket file descriptor is invalid");
    switch (this->__protocol) {
        case IPPROTO_TCP: {
            zpt::sockaddrin_t _cli_addr{};
            socklen_t _clilen = sizeof(zpt::sockaddrin_t);
            auto _newsockfd =
              ::accept(this->__sockfd, reinterpret_cast<zpt::sockaddr_t*>(&_cli_addr), &_clilen);

            if (this->__sockfd == 0) {
                throw zpt::ClosedException("server socket file descriptor has been closed");
            }

            expect(_newsockfd > 0,
                   "error while accepting new connection: " << std::string(std::strerror(errno)));

            struct linger _so_linger;
            _so_linger.l_onoff = 1;
            _so_linger.l_linger = 30;
            ::setsockopt(_newsockfd, SOL_SOCKET, SO_LINGER, &_so_linger, sizeof _so_linger);
            return zpt::make_stream<zpt::basic_socketstream<Char>>(
              this->__transport, _newsockfd, _cli_addr, zpt::NO_SSL, this->__protocol);
        }
        case UNIXPROTO_RAW: {
            zpt::sockaddrun_t _cli_addr{};
            bzero((char*)&_cli_addr, sizeof _cli_addr);
            _cli_addr.sun_family = AF_UNIX;
            strncpy(_cli_addr.sun_path, this->__path.data(), (sizeof _cli_addr.sun_path) - 1);
            auto _newsockfd = ::accept(this->__sockfd, nullptr, nullptr);

            if (this->__sockfd == 0) {
                throw zpt::ClosedException("server socket file descriptor has been closed");
            }

            expect(_newsockfd > 0, "error while accepting new connection");

            return zpt::make_stream<zpt::basic_socketstream<Char>>(
              this->__transport, _newsockfd, _cli_addr);
        }
    }
    return nullptr;
}
