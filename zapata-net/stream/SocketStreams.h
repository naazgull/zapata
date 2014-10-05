/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
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
#include <netdb.h>
#include <fcntl.h>
#include <streambuf>
#include <iostream>
#include <istream>
#include <ostream>
#include <strings.h>
#include <unistd.h>
#include <exceptions/ClosedException.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	template<typename Char>
	class basic_socketbuf : public std::basic_streambuf<Char>
	{
		public:
			typedef Char __char_type;
			typedef std::basic_streambuf<__char_type> __buf_type;
			typedef std::basic_ostream<__char_type> __stream_type;
			typedef typename __buf_type::int_type __int_type;
			typedef typename std::basic_streambuf<Char>::traits_type __traits_type;

		protected:

			static const int char_size = sizeof(__char_type);
			static const int SIZE = 128;
			__char_type obuf[SIZE];
			__char_type ibuf[SIZE];

			int __sock;

		public:
			basic_socketbuf() : __sock(0) {
				__buf_type::setp(obuf, obuf + (SIZE - 1));
				__buf_type::setg(ibuf, ibuf, ibuf);
			}

			virtual ~basic_socketbuf() {
				sync();
			}

			void set_socket(int _sock) {
				this->__sock = _sock;

//				int _opts = fcntl(this->__sock, F_GETFL);
//				_opts = _opts & ~O_NONBLOCK;
//				if (fcntl(this->__sock, F_SETFL, _opts) < 0) {
//				}
			}
			int get_socket() {
				return this->__sock;
			}

		protected:

			int output_buffer() {
				int num = __buf_type::pptr() - __buf_type::pbase();
				if (::send(__sock, reinterpret_cast<char*>(obuf), num * char_size, 0) != num) {
				                    return __traits_type::eof();
				}
				__buf_type::pbump(-num);
				return num;
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

				int num = -1;
				if ((num = ::recv(__sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, 0)) <= 0) {
				                    return __traits_type::eof();
				}
				__buf_type::setg(ibuf, ibuf, ibuf + num);
				return *__buf_type::gptr();
			}
	};

	typedef basic_socketbuf<char> socketbuf;
	typedef basic_socketbuf<wchar_t> wsocketbuf;

	template<typename Char>
	class basic_socketstream : public std::basic_iostream<Char>
	{
		public:
			typedef Char __char_type;
			typedef std::basic_iostream<__char_type> __stream_type;
			typedef basic_socketbuf<__char_type> __buf_type;

		protected:
			__buf_type __buf;

		public:
			basic_socketstream() :
				__stream_type(&__buf) {
			}
			basic_socketstream(int s) : __stream_type(&__buf) {
				__buf.set_socket(s);
			}
			virtual ~basic_socketstream() {
				this->close();
			}

			void assign(int _sockfd) {
				__buf.set_socket(_sockfd);
			}

			void close() {
				__stream_type::flush();
				__stream_type::clear();
				if (__buf.get_socket() != 0) {
					::close(__buf.get_socket());
				}
			}

			bool ready() {
				fd_set sockset;
				FD_ZERO(&sockset);
				FD_SET(__buf.get_socket(), &sockset);
				return select(__buf.get_socket() + 1, &sockset, NULL, NULL, NULL) == 1;
			}

			__buf_type& buffer() {
				return this->__buf;
			}

			bool open(const std::string& _host, uint16_t _port) {
				this->close();
				int _sd = socket(AF_INET, SOCK_STREAM, 0);
				sockaddr_in _sin;
				hostent *_he = gethostbyname(_host.c_str());

				std::copy(reinterpret_cast<char*>(_he->h_addr), reinterpret_cast<char*>(_he->h_addr) + _he->h_length, reinterpret_cast<char*>(&_sin.sin_addr.s_addr));
				_sin.sin_family = AF_INET;
				_sin.sin_port = htons(_port);

				if (connect(_sd, reinterpret_cast<sockaddr*>(&_sin), sizeof(_sin)) < 0) {
					__stream_type::setstate(std::ios::failbit);
				}
				else {
					__buf.set_socket(_sd);
				}
				return true;
			}
	};

	typedef basic_socketstream<char> socketstream;
	typedef basic_socketstream<wchar_t> wsocketstream;

	template<typename Char>
	class basic_serversocketstream : public std::basic_iostream<Char>
	{
		public:
			typedef Char __char_type;
			typedef std::basic_iostream<__char_type> __stream_type;
			typedef basic_socketbuf<__char_type> __buf_type;

		protected:
			__buf_type __buf;
			int __sockfd;

		public:
			basic_serversocketstream() :
				__stream_type(&__buf) {
			}
			basic_serversocketstream(int s) : __stream_type(&__buf) {
				__buf.set_socket(s);
			}
			virtual ~basic_serversocketstream() {
				this->close();
			}

			void close() {
				__stream_type::flush();
				__stream_type::clear();
				if (__buf.get_socket() != 0) {
					::close(__buf.get_socket());
				}
			}

			bool ready() {
				fd_set sockset;
				FD_ZERO(&sockset);
				FD_SET(__buf.get_socket(), &sockset);
				return select(__buf.get_socket() + 1, &sockset, NULL, NULL, NULL) == 1;
			}

			__buf_type& buffer() {
				return this->__buf;
			}

			bool bind(uint16_t _port) {
				this->__sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if (this->__sockfd < 0) {
					__stream_type::setstate(std::ios::failbit);
					throw zapata::ClosedException("Could not create server socket");
				}

				int _opt = 1;
				if (setsockopt(this->__sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &_opt, sizeof(_opt)) == SO_ERROR) {
					::close(this->__sockfd);
					this->__sockfd = -1;
					__stream_type::setstate(std::ios::failbit);
					throw zapata::ClosedException("Could not bind to the provided port");
				}

				struct sockaddr_in _serv_addr;
				bzero((char *) &_serv_addr, sizeof(_serv_addr));
				_serv_addr.sin_family = AF_INET;
				_serv_addr.sin_addr.s_addr = INADDR_ANY;
				_serv_addr.sin_port = htons(_port);
				if (::bind(this->__sockfd, (struct sockaddr *) &_serv_addr, sizeof(_serv_addr)) < 0) {
					::close(this->__sockfd);
					this->__sockfd = -1;
					__stream_type::setstate(std::ios::failbit);
					throw zapata::ClosedException("Could not bind to the provided port");
				}
				::listen(this->__sockfd, 100);
				__buf.set_socket(this->__sockfd);
				return true;
			}

			bool accept(socketstream* _out) {
				if (this->__sockfd != -1) {
					struct sockaddr_in* _cli_addr = new struct sockaddr_in();
					socklen_t _clilen = sizeof(struct sockaddr_in);
					int _newsockfd = ::accept(this->__sockfd, (struct sockaddr *) _cli_addr, &_clilen);

					if (_newsockfd < 0) {
						throw zapata::ClosedException("Could not accept client socket");
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
					socklen_t _clilen = sizeof(struct sockaddr_in);
					int _newsockfd = ::accept(this->__sockfd, (struct sockaddr *) _cli_addr, &_clilen);

					if (_newsockfd < 0) {
						throw zapata::ClosedException("Could not accept client socket");
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

}
