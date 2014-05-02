#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <streambuf>
#include <istream>
#include <ostream>
#include <strings.h>
#include <unistd.h>
#include <exceptions/ClosedException.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	template<typename Char>
	class basic_sslsocketbuf : public std::basic_streambuf<Char>
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
			basic_sslsocketbuf() : __sock(0) {
				__buf_type::setp(obuf, obuf + (SIZE - 1));
				__buf_type::setg(ibuf, ibuf, ibuf);
			}

			virtual ~basic_sslsocketbuf() {
				sync();
			}

			void set_socket(int sock) {
				this->__sock = sock;
			}
			int get_socket() {
				return this->__sock;
			}

		protected:

			int output_buffer() {
				int num = __buf_type::pptr() - __buf_type::pbase();
				if (send(__sock, reinterpret_cast<char*>(obuf), num * char_size, 0) != num)
				                    return __traits_type::eof();
				__buf_type::pbump(-num);
				return num;
			}

			virtual __int_type overflow(__int_type c) {
				if (c != __traits_type::eof())
				                    {
					*__buf_type::pptr() = c;
					__buf_type::pbump(1);
				}

				if (output_buffer() == __traits_type::eof())
				                    return __traits_type::eof();
				return c;
			}

			virtual int sync() {
				if (output_buffer() == __traits_type::eof())
				                    return __traits_type::eof();
				return 0;
			}

			virtual __int_type underflow() {
				if (__buf_type::gptr() < __buf_type::egptr())
				                    return *__buf_type::gptr();

				int num;
				if ((num = recv(__sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, 0)) <= 0)
				                    return __traits_type::eof();

				__buf_type::setg(ibuf, ibuf, ibuf + num);
				return *__buf_type::gptr();
			}
	};

	typedef basic_sslsocketbuf<char> sslsocketbuf;
	typedef basic_sslsocketbuf<wchar_t> wsslsocketbuf;

	template<typename Char>
	class basic_sslsocketstream : public std::basic_iostream<Char>
	{
		public:
			typedef Char __char_type;
			typedef std::basic_iostream<__char_type> __stream_type;
			typedef basic_sslsocketbuf<__char_type> __buf_type;

		protected:
			__buf_type __buf;

		public:
			basic_sslsocketstream() :
				__stream_type(&__buf) {
			}
			basic_sslsocketstream(int s) : __stream_type(&__buf) {
				__buf.set_socket(s);
			}

			void assign(int _sockfd) {
				__buf.set_socket(_sockfd);
			}

			void close() {
				if (__buf.get_socket() != 0) ::close(__buf.get_socket());
				__stream_type::clear();
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
				return *this;
			}
	};

	typedef basic_sslsocketstream<char> sslsocketstream;
	typedef basic_sslsocketstream<wchar_t> wsslsocketstream;

	template<typename Char>
	class basic_serversslsocketstream : public std::basic_iostream<Char>
	{
		public:
			typedef Char __char_type;
			typedef std::basic_iostream<__char_type> __stream_type;
			typedef basic_sslsocketbuf<__char_type> __buf_type;

		protected:
			__buf_type __buf;
			int __sockfd;

		public:
			basic_serversslsocketstream() :
				__stream_type(&__buf) {
			}
			basic_serversslsocketstream(int s) : __stream_type(&__buf) {
				__buf.set_socket(s);
			}

			void close() {
				if (__buf.get_socket() != 0) ::close(__buf.get_socket());
				__stream_type::clear();
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

			bool accept(sslsocketstream* _out) {
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
	};

	typedef basic_serversslsocketstream<char> serversslsocketstream;
	typedef basic_serversslsocketstream<wchar_t> wserversslsocketstream;

}
