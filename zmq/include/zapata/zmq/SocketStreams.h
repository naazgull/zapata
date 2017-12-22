/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publi, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <streambuf>
#include <iostream>
#include <istream>
#include <ostream>
#include <memory>
#include <strings.h>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <openssl/err.h>
#include <zapata/base/assert.h>
#include <zapata/exceptions/ClosedException.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	std::string ssl_error_print(SSL * _ssl, int _ret);

	template<typename Char>
	class basic_socketbuf : public std::basic_streambuf<Char> {
	public:
		typedef Char __char_type;
		typedef std::basic_streambuf<__char_type> __buf_type;
		typedef std::basic_ostream<__char_type> __stream_type;
		typedef typename __buf_type::int_type __int_type;
		typedef typename std::basic_streambuf<Char>::traits_type __traits_type;

	protected:

		static const int char_size = sizeof (__char_type);
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

	public:
		basic_socketbuf() : __sock(0), __ssl(false), __port(-1), __protocol(0), __sslstream(nullptr), __context(nullptr) {
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

		void set_socket(int _sock) {
			this->__sock = _sock;
			if (_sock != 0) {
				int iOption = 1;
				setsockopt(this->__sock, SOL_SOCKET, SO_KEEPALIVE, (const char *) &iOption,  sizeof (int));
				struct linger a;
				a.l_onoff = 1;
				a.l_linger = 5;
				setsockopt(this->__sock, SOL_SOCKET, SO_LINGER, (char*) &a, sizeof a);
			}
		}

		int get_socket() {
			return this->__sock;
		}

		void set_context(SSL_CTX* _ctx) {
			this->__ssl = true;
			this->__context = _ctx;
			this->__sslstream = SSL_new(_ctx);
			SSL_set_fd(this->__sslstream, this->__sock);
			SSL_connect(this->__sslstream);
		}

		struct sockaddr_in& server() {
			return this->__server;
		}

		bool& ssl() {
			return this->__ssl;
		}

		std::string& host() {
			return this->__host;
		}

		short& port() {
			return this->__port;
		}

		short& protocol() {
			return this->__protocol;
		}

		virtual bool __good() {
			return this->__sock != 0 && (!this->__ssl || (this->__ssl && this->__sslstream != nullptr && this->__context != nullptr));
		}

	protected:

		__int_type output_buffer() {
			if (!__good()) {
				return __traits_type::eof();
			}

			if (!this->__ssl) {
				switch(protocol()) {
					case IPPROTO_IP: {
						int _num = __buf_type::pptr() - __buf_type::pbase();
						int _actually_written = -1;
						if ((_actually_written = ::send(__sock, reinterpret_cast<char*>(obuf), _num * char_size, MSG_NOSIGNAL)) < 0) {
							::shutdown(this->__sock, SHUT_RDWR);
							::close(this->__sock);
							this->__sock = 0;
							assertz(_actually_written > 0, "write operation failed", 503, 0);
						}
						__buf_type::pbump(-_actually_written);
						return _actually_written;
					}
					case IPPROTO_UDP: {
						int _num = __buf_type::pptr() - __buf_type::pbase();
						int _actually_written = -1;
						if ((_actually_written = ::sendto(__sock, reinterpret_cast<char*>(obuf), _num * char_size, 0, (struct sockaddr *) & __server, sizeof __server)) < 0) {
							if (_actually_written < 0) {
								::shutdown(this->__sock, SHUT_RDWR);
								::close(this->__sock);
								this->__sock = 0;
								assertz(_actually_written > 0, "write operation failed", 503, 0);
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
					if ((_actually_written = SSL_write(this->__sslstream, reinterpret_cast<char*>(obuf), _num * char_size)) < 0) {
						if (SSL_get_error(this->__sslstream, _actually_written) != SSL_ERROR_WANT_WRITE) {
							SSL_free(this->__sslstream);
							SSL_CTX_free(this->__context);
							::shutdown(this->__sock, SHUT_RDWR);
							::close(this->__sock);
							this->__sock = 0;
							this->__sslstream = nullptr;
							this->__context = nullptr;
							assertz(_actually_written > 0, std::string("write operation failed: ") + zpt::ssl_error_print(this->__sslstream, _actually_written), 503, 0);
						}
					}
				}
				while (SSL_get_error(this->__sslstream, _actually_written) == SSL_ERROR_WANT_WRITE);
				__buf_type::pbump(-_actually_written);
				return _actually_written;
			}
			return __traits_type::eof();
		}

		virtual __int_type overflow(__int_type c) {
			if (c != __traits_type::eof()) {
				*__buf_type::pptr() = c;
				__buf_type::pbump(1);
			}

			if (output_buffer() == __traits_type::eof()) {
				return __traits_type::eof();
			}
			return c;
		}

		virtual int sync() {
			if (output_buffer() == __traits_type::eof()) {
				return __traits_type::eof();
			}
			return 0;
		}

		virtual __int_type underflow() {
			if (__buf_type::gptr() < __buf_type::egptr()) {
				return *__buf_type::gptr();
			}

			if (!__good()) {
				return __traits_type::eof();
			}

			if (!this->__ssl) {
				switch(protocol()) {
					case IPPROTO_IP: {
						int _actually_read = -1;
						if ((_actually_read = ::recv(__sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, 0)) < 0) {
							::shutdown(this->__sock, SHUT_RDWR);
							::close(this->__sock);
							this->__sock = 0;
							assertz(_actually_read > 0, "read operation failed", 503, 0);
						}
						if (_actually_read == 0) {
							return __traits_type::eof();
						}
						__buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
						return *__buf_type::gptr();
					}
					case IPPROTO_UDP : {
						int _actually_read = -1;
						socklen_t _peer_addr_len = sizeof __peer;
						if ((_actually_read = ::recvfrom(__sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, 0, (struct sockaddr *) & __peer, &_peer_addr_len)) < 0) {
							::shutdown(this->__sock, SHUT_RDWR);
							::close(this->__sock);
							this->__sock = 0;
							assertz(_actually_read > 0, "read operation failed", 503, 0);
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
					if ((_actually_read = SSL_read(this->__sslstream, reinterpret_cast<char*>(ibuf), SIZE * char_size)) < 0) {
						if (SSL_get_error(this->__sslstream, _actually_read) != SSL_ERROR_WANT_READ) {
							SSL_free(this->__sslstream);
							SSL_CTX_free(this->__context);
							::shutdown(this->__sock, SHUT_RDWR);
							::close(this->__sock);
							this->__sock = 0;
							this->__sslstream = nullptr;
							this->__context = nullptr;
							assertz(_actually_read > 0, std::string("read operation failed: ") + zpt::ssl_error_print(this->__sslstream, _actually_read), 503, 0);
						}
					}
				}
				while (SSL_get_error(this->__sslstream, _actually_read) == SSL_ERROR_WANT_READ);
				if (_actually_read == 0) {
					return __traits_type::eof();
				}
				__buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
				return *__buf_type::gptr();
			}
			return __traits_type::eof();
		}
	};

	typedef basic_socketbuf<char> socketbuf;
	typedef basic_socketbuf<wchar_t> wsocketbuf;

	template<typename Char>
	class basic_socketstream : public std::basic_iostream<Char> {
	public:
		typedef Char __char_type;
		typedef std::basic_iostream<__char_type> __stream_type;
		typedef basic_socketbuf<__char_type> __buf_type;

	protected:
		__buf_type __buf;
		bool __is_error;

	public:
		basic_socketstream() : __stream_type(&__buf), __is_error(false) {
		};

		basic_socketstream(int s, bool _ssl = false, short _protocol = IPPROTO_IP) : __stream_type(&__buf), __is_error(false) {
			__buf.set_socket(s);
			__buf.protocol() = _protocol;
			__buf.ssl() = _ssl;
		}
		virtual ~basic_socketstream() {
			__stream_type::flush();
			__stream_type::clear();
			__buf.set_socket(0);
		}

		bool& ssl() {
			return __buf.ssl();
		}

		std::string& host() {
			return __buf.host();
		}

		short& port() {
			return __buf.port();
		}

		short& protocol() {
			return __buf.protocol();
		}

		void assign(int _sockfd) {
			__buf.set_socket(_sockfd);
			__buf.ssl() = false;
		}

		void assign(int _sockfd, SSL_CTX* _ctx) {
			__buf.set_socket(_sockfd);
			__buf.set_context(_ctx);
			__buf.ssl() = true;
		}

		void unassign() {
			__buf.set_socket(0);
		}

		void close() {
			__stream_type::flush();
			__stream_type::clear();
			if (__buf.get_socket() != 0) {
				::shutdown(__buf.get_socket(), SHUT_RDWR);
				::close(__buf.get_socket());
			}
			__buf.set_socket(0);
		}

		bool is_open() {
			return (!__is_error && __buf.get_socket() != 0 && __buf.__good());
		}

		bool ready() {
			fd_set sockset;
			FD_ZERO(&sockset);
			FD_SET(__buf.get_socket(), &sockset);
			return select(__buf.get_socket() + 1, &sockset, nullptr, nullptr, nullptr) == 1;
		}

		__buf_type& buffer() {
			return this->__buf;
		}

		bool open(const std::string& _host, uint16_t _port, bool _ssl = false, short _protocol = IPPROTO_IP) {
			this->close();
			__buf.host() = _host;
			__buf.port() = _port;
			__buf.protocol() = _protocol;
			__buf.ssl() = _ssl;

			::hostent *_he = gethostbyname(_host.c_str());
			if (_he == nullptr) {
				return false;
			}

			std::string _addr(reinterpret_cast<char*>(_he->h_addr), _he->h_length);
			std::copy(_addr.c_str(), _addr.c_str() + _addr.length(), reinterpret_cast<char*>(& __buf.server().sin_addr.s_addr));
			__buf.server().sin_family = AF_INET;
			__buf.server().sin_port = htons(_port);

			if (!_ssl) {
				switch(_protocol) {
					case IPPROTO_IP: {
						int _sd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
						if (::connect(_sd, reinterpret_cast<sockaddr*>(& __buf.server()), sizeof __buf.server()) < 0) {
							__stream_type::setstate(std::ios::failbit);
							__buf.set_socket(0);
							__is_error = true;
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
						setsockopt(_sd, SOL_SOCKET, SO_REUSEADDR, (char *) &_reuse, sizeof _reuse);
						int _ret = inet_aton(__buf.host().c_str(), & __buf.server().sin_addr);
						if (_ret == 0) {
							struct addrinfo _hints, * _result = NULL;
							std::memset(& _hints, 0, sizeof _hints);
							_hints.ai_family = AF_INET;
							_hints.ai_socktype = SOCK_DGRAM;

							_ret = getaddrinfo(__buf.host().c_str(), NULL, & _hints, & _result);
							if (_ret) {
								__stream_type::setstate(std::ios::failbit);
								__buf.set_socket(0);
								__is_error = true;
								return false;
							}
							struct sockaddr_in* _host_addr = (struct sockaddr_in *) _result->ai_addr;
							memcpy(& __buf.server().sin_addr, & _host_addr->sin_addr, sizeof (struct in_addr));
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
				if (::connect(_sd, reinterpret_cast<sockaddr*>(&__buf.server()), sizeof __buf.server()) < 0) {
					__stream_type::setstate(std::ios::failbit);
					__buf.set_socket(0);
					__is_error = true;
					return false;
				}
				else {
					SSL_library_init();
					OpenSSL_add_all_algorithms();
					SSL_load_error_strings();
					SSL_CTX* _context = SSL_CTX_new(TLSv1_client_method());
					if (_context == nullptr) {
						__stream_type::setstate(std::ios::failbit);
						__buf.set_socket(0);
						__is_error = true;
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

	typedef basic_socketstream<char> socketstream;
	typedef basic_socketstream<wchar_t> wsocketstream;

	// typedef std::shared_ptr< zpt::socketstream > socketstream_ptr;
	// typedef std::shared_ptr< zpt::wsocketstream > wsocketstream_ptr;

	class socketstream_ptr : public std::shared_ptr< zpt::socketstream > {
	public:
		socketstream_ptr() : std::shared_ptr< zpt::socketstream >(new zpt::socketstream()) {}
		socketstream_ptr(zpt::socketstream* _new) : std::shared_ptr< zpt::socketstream >(_new) {}
		socketstream_ptr(int _fd, bool _ssl = false, short _protocol = IPPROTO_IP) : std::shared_ptr< zpt::socketstream >(new zpt::socketstream(_fd, _ssl, _protocol)) {}
		virtual ~socketstream_ptr() {}
	};

	class wsocketstream_ptr : public std::shared_ptr< zpt::wsocketstream > {
	public:
		wsocketstream_ptr() : std::shared_ptr< zpt::wsocketstream >(new zpt::wsocketstream()) {}
		wsocketstream_ptr(zpt::wsocketstream* _new) : std::shared_ptr< zpt::wsocketstream >(_new) {}
		wsocketstream_ptr(int _fd, bool _ssl = false, short _protocol = IPPROTO_IP) : std::shared_ptr< zpt::wsocketstream >(new zpt::wsocketstream(_fd, _ssl, _protocol)) {}
		virtual ~wsocketstream_ptr() {}
	};

	template<typename Char>
	class basic_serversocketstream {
	protected:
		int __sockfd;

	public:
		basic_serversocketstream() :  __sockfd(0) {
		};

		basic_serversocketstream(int s) : __sockfd(s) {
		}
		virtual ~basic_serversocketstream() {
			this->close();
		}

		void close() {
			::shutdown(this->__sockfd, SHUT_RDWR);
			::close(this->__sockfd);
			this->__sockfd = 0;
		}

		bool is_open() {
			return __sockfd != 0;;
		}

		bool ready() {
			fd_set sockset;
			FD_ZERO(&sockset);
			FD_SET(__sockfd, &sockset);
			return select(__sockfd + 1, &sockset, nullptr, nullptr, nullptr) == 1;
		}

		bool bind(uint16_t _port) {
			this->__sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
			if (this->__sockfd < 0) {
				return false;
			}

			int _opt = 1;
			if (setsockopt(this->__sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &_opt, sizeof _opt) == SO_ERROR) {
				::shutdown(this->__sockfd, SHUT_RDWR);
				::close(this->__sockfd);
				this->__sockfd = 0;
				return false;
			}

			struct sockaddr_in _serv_addr;
			bzero((char *) &_serv_addr, sizeof _serv_addr);
			_serv_addr.sin_family = AF_INET;
			_serv_addr.sin_addr.s_addr = INADDR_ANY;
			_serv_addr.sin_port = htons(_port);
			if (::bind(this->__sockfd, (struct sockaddr *) &_serv_addr, sizeof _serv_addr) < 0) {
				::shutdown(this->__sockfd, SHUT_RDWR);
				::close(this->__sockfd);
				this->__sockfd = 0;
				return false;
			}
			::listen(this->__sockfd, 100);
			return true;
		}

		bool accept(socketstream* _out) {
			if (this->__sockfd != -1) {
				struct sockaddr_in* _cli_addr = new struct sockaddr_in();
				socklen_t _clilen = sizeof (struct sockaddr_in);
				int _newsockfd = ::accept(this->__sockfd, (struct sockaddr *) _cli_addr, &_clilen);

				if (_newsockfd < 0) {
					return false;
				}

				struct linger _so_linger;
				_so_linger.l_onoff = 1;
				_so_linger.l_linger = 30;
				::setsockopt(_newsockfd,SOL_SOCKET, SO_LINGER, &_so_linger, sizeof _so_linger);
				_out->assign(_newsockfd);
				return true;
			}
			return false;
		}

		bool accept(int* _out) {
			if (this->__sockfd != -1) {
				struct sockaddr_in* _cli_addr = new struct sockaddr_in();
				socklen_t _clilen = sizeof (struct sockaddr_in);
				int _newsockfd = ::accept(this->__sockfd, (struct sockaddr *) _cli_addr, &_clilen);

				if (_newsockfd < 0) {
					return false;
				}

				struct linger _so_linger;
				_so_linger.l_onoff = 1;
				_so_linger.l_linger = 30;
				::setsockopt(_newsockfd,SOL_SOCKET, SO_LINGER, &_so_linger, sizeof _so_linger);
				*_out = _newsockfd;
				return true;
			}
			return false;
		}
	};

	typedef basic_serversocketstream<char> serversocketstream;
	typedef basic_serversocketstream<wchar_t> wserversocketstream;

	class serversocketstream_ptr : public std::shared_ptr< zpt::serversocketstream > {
	public:
		serversocketstream_ptr() : std::shared_ptr< zpt::serversocketstream >(new zpt::serversocketstream()) {}
		serversocketstream_ptr(zpt::serversocketstream* _new) : std::shared_ptr< zpt::serversocketstream >(_new) {}
		serversocketstream_ptr(int _fd) : std::shared_ptr< zpt::serversocketstream >(new zpt::serversocketstream(_fd)) {}
		virtual ~serversocketstream_ptr() {}
	};

	class wserversocketstream_ptr : public std::shared_ptr< zpt::wserversocketstream > {
	public:
		wserversocketstream_ptr() : std::shared_ptr< zpt::wserversocketstream >(new zpt::wserversocketstream()) {}
		wserversocketstream_ptr(zpt::wserversocketstream* _new) : std::shared_ptr< zpt::wserversocketstream >(_new) {}
		wserversocketstream_ptr(int _fd) : std::shared_ptr< zpt::wserversocketstream >(new zpt::wserversocketstream(_fd)) {}
		virtual ~wserversocketstream_ptr() {}
	};

	#define CRLF "\r\n"

}
