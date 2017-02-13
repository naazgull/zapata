/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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
#include <zapata/exceptions/ClosedException.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

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
		static const int SIZE = 128;
		__char_type obuf[SIZE];
		__char_type ibuf[SIZE];

		int __sock;
		struct sockaddr_in __server;
		string __host;
		short __port;
		short __protocol;

	public:
		basic_socketbuf() : __sock(0), __port(-1), __protocol(0) {
			__buf_type::setp(obuf, obuf + (SIZE - 1));
			__buf_type::setg(ibuf, ibuf, ibuf);
		};

		virtual ~basic_socketbuf() {
			sync();
		}

		void set_socket(int _sock) {
			this->__sock = _sock;
			if (_sock != 0) {
				int iOption = 1; 
				setsockopt(this->__sock, SOL_SOCKET, SO_KEEPALIVE, (const char *) &iOption,  sizeof(int));
			}
		}

		int get_socket() {
			return this->__sock;
		}

		struct sockaddr_in& server() {
			return this->__server;
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
			return this->__sock != 0;
		}

	protected:

		int output_buffer() {
			if (!__good()) {
				return __traits_type::eof();
			}

			switch(protocol()) {
				case IPPROTO_IP: {
					int _num = __buf_type::pptr() - __buf_type::pbase();
					int _actually_written = -1;
					if ((_actually_written = ::send(__sock, reinterpret_cast<char*>(obuf), _num * char_size, MSG_NOSIGNAL)) < 0) {
						if (_actually_written < 0) {
							::shutdown(this->__sock, SHUT_RDWR);
							::close(this->__sock);
							this->__sock = 0;
						}
						return __traits_type::eof();
					}
					__buf_type::pbump(-_actually_written);
					return _actually_written;
				}
				case IPPROTO_UDP: {
					int _num = __buf_type::pptr() - __buf_type::pbase();
					int _actually_written = -1;
					if ((_actually_written = ::sendto(__sock, reinterpret_cast<char*>(obuf), _num * char_size, 0, (struct sockaddr *) & __server, sizeof(__server))) < 0) {
						if (_actually_written < 0) {
							::shutdown(this->__sock, SHUT_RDWR);
							::close(this->__sock);
							this->__sock = 0;
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

			int num = -1;
			if ((num = ::recv(__sock, reinterpret_cast<char*>(ibuf), SIZE * char_size, MSG_NOSIGNAL)) <= 0) {
				if (num < 0) {
					::shutdown(this->__sock, SHUT_RDWR);
					::close(this->__sock);
					this->__sock = 0;
				}
				return __traits_type::eof();
			}
			__buf_type::setg(ibuf, ibuf, ibuf + num);
			return *__buf_type::gptr();
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

	public:
		basic_socketstream() :
		__stream_type(&__buf) {
		};

		basic_socketstream(int s) : __stream_type(&__buf) {
			__buf.set_socket(s);
		}
		virtual ~basic_socketstream() {
			this->close();
		}

		void assign(int _sockfd) {
			__buf.set_socket(_sockfd);
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
			bool _return = (__buf.get_socket() != 0 && __buf.__good());
			return _return;
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

		bool open(const std::string& _host, uint16_t _port, short _protocol = IPPROTO_IP) {
			this->close();
			__buf.host() = _host;
			__buf.port() = _port;
			__buf.protocol() = _protocol;
			
			::hostent *_he = gethostbyname(_host.c_str());
			if (_he == nullptr) {
				return false;
			}

			std::copy(reinterpret_cast<char*>(_he->h_addr), reinterpret_cast<char*>(_he->h_addr) + _he->h_length, reinterpret_cast<char*>(& __buf.server().sin_addr.s_addr));
			__buf.server().sin_family = AF_INET;
			__buf.server().sin_port = htons(_port);

			switch(_protocol) {
				case IPPROTO_IP: {
					int _sd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
					if (::connect(_sd, reinterpret_cast<sockaddr*>(& __buf.server()), sizeof(__buf.server())) < 0) {
						__stream_type::setstate(std::ios::failbit);
						__buf.set_socket(0);
						return false;
					}
					else {
						__buf.set_socket(_sd);
					}
					return true;
				}
				case IPPROTO_UDP: {
					int _sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
					int _ret = inet_aton(__buf.host().c_str(), & __buf.server().sin_addr);
					if (_ret == 0) {
						struct addrinfo _hints, * _result = NULL;
						std::memset(& _hints, 0, sizeof(_hints));
						_hints.ai_family = AF_INET;
						_hints.ai_socktype = SOCK_DGRAM;

						_ret = getaddrinfo(__buf.host().c_str(), NULL, & _hints, & _result);
						if (_ret) {
							__stream_type::setstate(std::ios::failbit);
							__buf.set_socket(0);
							return false;
						}
						struct sockaddr_in* _host_addr = (struct sockaddr_in *) _result->ai_addr;
						memcpy(& __buf.server().sin_addr, & _host_addr->sin_addr, sizeof(struct in_addr));
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
	};

	typedef basic_socketstream<char> socketstream;
	typedef basic_socketstream<wchar_t> wsocketstream;

	typedef std::shared_ptr< zpt::socketstream > socketstream_ptr;
	typedef std::shared_ptr< zpt::wsocketstream > wsocketstream_ptr;

	template<typename Char>
	class basic_serversocketstream : public std::basic_iostream<Char> {
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
		};

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
				::shutdown(__buf.get_socket(), SHUT_RDWR);
				::close(__buf.get_socket());
			}
			__buf.set_socket(0);
		}

		bool is_open() {
			return __buf.get_socket() != 0 && __buf.__good();
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

		bool bind(uint16_t _port) {
			this->__sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (this->__sockfd < 0) {
				__stream_type::setstate(std::ios::failbit);
				return false;
			}

			int _opt = 1;
			if (setsockopt(this->__sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &_opt, sizeof(_opt)) == SO_ERROR) {
				::shutdown(this->__sockfd, SHUT_RDWR);
				::close(this->__sockfd);
				this->__sockfd = 0;
				__stream_type::setstate(std::ios::failbit);
				return false;
			}

			struct sockaddr_in _serv_addr;
			bzero((char *) &_serv_addr, sizeof(_serv_addr));
			_serv_addr.sin_family = AF_INET;
			_serv_addr.sin_addr.s_addr = INADDR_ANY;
			_serv_addr.sin_port = htons(_port);
			if (::bind(this->__sockfd, (struct sockaddr *) &_serv_addr, sizeof(_serv_addr)) < 0) {
				::shutdown(this->__sockfd, SHUT_RDWR);
				::close(this->__sockfd);
				this->__sockfd = 0;
				__buf.set_socket(0);
				__stream_type::setstate(std::ios::failbit);
				return false;
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
				socklen_t _clilen = sizeof(struct sockaddr_in);
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

	typedef std::shared_ptr< zpt::serversocketstream > serversocketstream_ptr;
	typedef std::shared_ptr< zpt::wserversocketstream > wserversocketstream_ptr;

	#define CRLF "\r\n"

	class websocketserverstream : public basic_serversocketstream<char> {
	public:
		websocketserverstream() {
		};
		virtual ~websocketserverstream() {
		}

		bool handshake() {
			string _key;
			string _line;
			do {
				std::getline((* this), _line);
				zpt::trim(_line);
				string _header(_line);
				std::transform(_header.begin(), _header.end(), _header.begin(), ::tolower);
				if (_header.find("sec-websocket-key:") != std::string::npos) {
					_key.assign(_line.substr(19));
				}
			}
			while (_line != "");

			_key.insert(_key.length(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
			string _sha1 = zpt::hash::SHA1(_key);
			zpt::base64::encode(_sha1);
			_key.assign(_sha1);

			(* this) << 
			"HTTP/1.1 101 Switching Protocols" << "\r\n" <<
			"Upgrade: websocket" << CRLF << 
			"Connection: Upgrade" << CRLF <<
			"Sec-WebSocket-Accept: " << _key << CRLF <<
			CRLF << std::flush;
			return true;
		}

		bool read(string& _out, int* _op_code) {
			unsigned char _hdr;
			(* this) >> noskipws >> _hdr;

			bool _fin = _hdr & 0x80;
			*_op_code = _hdr & 0x0F;
			(* this) >> noskipws >> _hdr;
			bool _mask = _hdr & 0x80;
			string _masking;
			string _masked;

			int _len = _hdr & 0x7F;
			if (_len == 126) {
				(* this) >> noskipws >> _hdr;
				_len = (int) _hdr;
				_len <<= 8;
				(* this) >> noskipws >> _hdr;
				_len += (int) _hdr;
			}
			else if (_len == 127) {
				(* this) >> noskipws >> _hdr;
				_len = (int) _hdr;
				for (int _i = 0; _i < 7; _i++) {
					_len <<= 8;
					(* this) >> noskipws >> _hdr;
					_len += (int) _hdr;
				}
			}

			if (_mask) {
				for (int _i = 0; _i < 4; _i++) {
					(* this) >> noskipws >> _hdr;
					_masking.push_back((char) _hdr);
				}
			}


			for (int _i = 0; _i != _len; _i++) {
				(* this) >> noskipws >> _hdr;
				_masked.push_back((char) _hdr);
			}

			if (_mask) {
				for (size_t _i = 0; _i < _masked.length(); _i++) {
					_out.push_back(_masked[_i] ^ _masking[_i % 4]);
				}
			}
			else {
				_out.assign(_masked);
			}

			return _fin;
		}

		bool write(string _in){
			int _len = _in.length();

			if (!this->is_open()) {
				return false;
			}
			(* this) << (unsigned char) 0x81;
			if (_len > 65535) {
				(* this) << (unsigned char) 0x7F;
				(* this) << (unsigned char) 0x00;
				(* this) << (unsigned char) 0x00;
				(* this) << (unsigned char) 0x00;
				(* this) << (unsigned char) 0x00;
				(* this) << ((unsigned char) ((_len >> 24) & 0xFF));
				(* this) << ((unsigned char) ((_len >> 16) & 0xFF));
				(* this) << ((unsigned char) ((_len >> 8) & 0xFF));
				(* this) << ((unsigned char) (_len & 0xFF));
			}
			else if (_len > 125) {
				(* this) << (unsigned char) 0x7E;
				(* this) << ((unsigned char) (_len >> 8));
				(* this) << ((unsigned char) (_len & 0xFF));
			}
			else {
				(* this) << (unsigned char) (0x80 | ((unsigned char) _len));
			}

			(* this) << _in << std::flush;
			return true;
		}
	};

	class websocketstream : public basic_socketstream<char> {
	public:
		websocketstream(int _socket) : basic_socketstream<char>(_socket) {
		};
		virtual ~websocketstream() {
		}

		bool handshake() {
			string _key;
			string _line;
			do {
				std::getline((* this), _line);
				zpt::trim(_line);
				string _header(_line);
				std::transform(_header.begin(), _header.end(), _header.begin(), ::tolower);
				if (_header.find("sec-websocket-key:") != std::string::npos) {
					_key.assign(_line.substr(19));
				}
			}
			while (_line != "");

			_key.insert(_key.length(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
			string _sha1 = zpt::hash::SHA1(_key);
			zpt::base64::encode(_sha1);
			_key.assign(_sha1);

			(* this) << 
			"HTTP/1.1 101 Switching Protocols" << "\r\n" <<
			"Upgrade: websocket" << CRLF << 
			"Connection: Upgrade" << CRLF <<
			"Sec-WebSocket-Accept: " << _key << CRLF <<
			CRLF << std::flush;
			return true;
		}

		bool read(string& _out, int* _op_code) {
			unsigned char _hdr;
			(* this) >> noskipws >> _hdr;

			bool _fin = _hdr & 0x80;
			*_op_code = _hdr & 0x0F;
			(* this) >> noskipws >> _hdr;
			bool _mask = _hdr & 0x80;
			string _masking;
			string _masked;

			int _len = _hdr & 0x7F;
			if (_len == 126) {
				(* this) >> noskipws >> _hdr;
				_len = (int) _hdr;
				_len <<= 8;
				(* this) >> noskipws >> _hdr;
				_len += (int) _hdr;
			}
			else if (_len == 127) {
				(* this) >> noskipws >> _hdr;
				_len = (int) _hdr;
				for (int _i = 0; _i < 7; _i++) {
					_len <<= 8;
					(* this) >> noskipws >> _hdr;
					_len += (int) _hdr;
				}
			}

			if (_mask) {
				for (int _i = 0; _i < 4; _i++) {
					(* this) >> noskipws >> _hdr;
					_masking.push_back((char) _hdr);
				}
			}


			for (int _i = 0; _i != _len; _i++) {
				(* this) >> noskipws >> _hdr;
				_masked.push_back((char) _hdr);
			}

			if (_mask) {
				for (size_t _i = 0; _i < _masked.length(); _i++) {
					_out.push_back(_masked[_i] ^ _masking[_i % 4]);
				}
			}
			else {
				_out.assign(_masked);
			}

			return _fin;
		}

		bool write(string _in, bool _masked = false){
			int _len = _in.length();

			if (!this->is_open()) {
				return false;
			}
			(* this) << (unsigned char) 0x81;
			if (_len > 65535) {
				(* this) << (unsigned char) 0x7F;
				(* this) << (unsigned char) 0x00;
				(* this) << (unsigned char) 0x00;
				(* this) << (unsigned char) 0x00;
				(* this) << (unsigned char) 0x00;
				(* this) << ((unsigned char) ((_len >> 24) & 0xFF));
				(* this) << ((unsigned char) ((_len >> 16) & 0xFF));
				(* this) << ((unsigned char) ((_len >> 8) & 0xFF));
				(* this) << ((unsigned char) (_len & 0xFF));
			}
			else if (_len > 125) {
				(* this) << (unsigned char) 0x7E;
				(* this) << ((unsigned char) (_len >> 8));
				(* this) << ((unsigned char) (_len & 0xFF));
			}
			else {
				(* this) << (unsigned char) (0x80 | ((unsigned char) _len));
			}

			if (_masked) {
				for (int _i = 0; _i != 4; _i++) {
					(* this) << (unsigned char) 0x00;
				}		
			}

			(* this) << _in << std::flush;
			return true;
		}
	};	

	typedef std::shared_ptr< zpt::websocketstream > websocketstream_ptr;
	typedef std::shared_ptr< zpt::websocketserverstream > websocketserverstream_ptr;

}
