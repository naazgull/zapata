/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <streambuf>
#include <istream>
#include <ostream>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <memory>
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
	class basic_sslsocketbuf : public std::basic_streambuf<Char> {
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
		SSL* __sslstream;
		SSL_CTX* __context;

	public:
		basic_sslsocketbuf() : __sock(0), __sslstream(nullptr), __context(nullptr) {
			__buf_type::setp(obuf, obuf + (SIZE - 1));
			__buf_type::setg(ibuf, ibuf, ibuf);
		};
		virtual ~basic_sslsocketbuf() {
			sync();
			if (this->__sslstream != nullptr) {
				SSL_free(this->__sslstream);
				SSL_CTX_free(this->__context);
			}
		}

		void set_socket(int sock) {
			this->__sock = sock;
		}
		int get_socket() {
			return this->__sock;
		}
		void set_context(SSL_CTX* _ctx) {
			this->__context = _ctx;
			this->__sslstream = SSL_new(_ctx);
			SSL_set_fd(this->__sslstream, this->__sock);
			SSL_connect(this->__sslstream);
		}

		virtual bool __good() {
			return this->__sock != 0 && this->__sslstream != nullptr && this->__context != nullptr;
		}

	protected:

		int output_buffer() {
			if (!__good()) {
				return __traits_type::eof();
			}
			int _num = __buf_type::pptr() - __buf_type::pbase();
			int _actually_written = 0;
			if ((_actually_written = SSL_write(this->__sslstream, reinterpret_cast<char*>(obuf), _num * char_size)) < 0) {
				cout << zpt::ssl_error_print(this->__sslstream, _actually_written) << endl << flush;
				SSL_free(this->__sslstream);
				SSL_CTX_free(this->__context);
				::shutdown(this->__sock, SHUT_RDWR);
				::close(this->__sock);
				this->__sock = 0;
				this->__sslstream = nullptr;
				this->__context = nullptr;
				assertz(_actually_written > 0, "write operation failed", 503, 0);
			}
			__buf_type::pbump(-_actually_written);
			return _actually_written;
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
			if (output_buffer() == __traits_type::eof()){
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

			int _actually_read;
			if ((_actually_read = SSL_read(this->__sslstream, reinterpret_cast<char*>(ibuf), SIZE * char_size)) < 0) {
				SSL_free(this->__sslstream);
				SSL_CTX_free(this->__context);
				::shutdown(this->__sock, SHUT_RDWR);
				::close(this->__sock);
				this->__sock = 0;
				this->__sslstream = nullptr;
				this->__context = nullptr;
				assertz(_actually_read > 0, "read operation failed", 503, 0);
			}
			if (_actually_read == 0) {
				return __traits_type::eof();
			}
			__buf_type::setg(ibuf, ibuf, ibuf + _actually_read);
			return *__buf_type::gptr();
		}
	};

	typedef basic_sslsocketbuf<char> sslsocketbuf;
	typedef basic_sslsocketbuf<wchar_t> wsslsocketbuf;

	template<typename Char>
	class basic_sslsocketstream : public std::basic_iostream<Char> {
	public:
		typedef Char __char_type;
		typedef std::basic_iostream<__char_type> __stream_type;
		typedef basic_sslsocketbuf<__char_type> __buf_type;

	protected:
		__buf_type __buf;
		bool __is_error;

	public:
		basic_sslsocketstream() :
			__stream_type(&__buf), __is_error(false) {
		};
		basic_sslsocketstream(int s, SSL_CTX* _ctx) : __stream_type(&__buf), __is_error(false) {
			__buf.set_socket(s);
			__buf.set_context(_ctx);
		}
		virtual ~basic_sslsocketstream() {
			this->close();
		}

		void assign(int _sockfd, SSL_CTX* _ctx) {
			__buf.set_socket(_sockfd);
			__buf.set_context(_ctx);
		}

		void close() {
			if (__buf.get_socket() != 0) {
				::shutdown(__buf.get_socket(), SHUT_RDWR);
				::close(__buf.get_socket());
			}
			__stream_type::clear();
		}

		bool is_open() {
			return !__is_error && __buf.get_socket() != 0 && __buf.__good();
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

		bool open(const std::string& _host, uint16_t _port) {
			this->close();
			int _sd = socket(AF_INET, SOCK_STREAM, 0);
			sockaddr_in _sin;
			hostent *_he = gethostbyname(_host.c_str());
			if (_he == nullptr) {
				return false;
			}
			
			std::copy(reinterpret_cast<char*>(_he->h_addr), reinterpret_cast<char*>(_he->h_addr) + _he->h_length, reinterpret_cast<char*>(&_sin.sin_addr.s_addr));
			_sin.sin_family = AF_INET;
			_sin.sin_port = htons(_port);

			// fcntl(_sd, F_SETFL, O_NONBLOCK);
			if (::connect(_sd, reinterpret_cast<sockaddr*>(&_sin), sizeof(_sin)) < 0) {
				__stream_type::setstate(std::ios::failbit);
				__buf.set_socket(0);
				__is_error = true;
				return false;
			}
			else {
				// fcntl(_sd, F_SETFL, fcntl(_sd, F_GETFL, 0) & ~O_NONBLOCK);
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
			struct timeval _timeout;
			_timeout.tv_sec = 10;
			_timeout.tv_usec = 0;

			if (setsockopt (_sd, SOL_SOCKET, SO_RCVTIMEO, (char *) &_timeout, sizeof(_timeout)) < 0) {
				this->close();
				__is_error = true;
				return false;
			}
			if (setsockopt (_sd, SOL_SOCKET, SO_SNDTIMEO, (char *) &_timeout, sizeof(_timeout)) < 0) {
				this->close();
				__is_error = true;
				return false;
			}
			return true;
		}
	};

	typedef basic_sslsocketstream<char> sslsocketstream;
	typedef basic_sslsocketstream<wchar_t> wsslsocketstream;

	typedef std::shared_ptr< zpt::sslsocketstream > sslsocketstream_ptr;
	typedef std::shared_ptr< zpt::wsslsocketstream > wsslscoketstream_ptr;

	template<typename Char>
	class basic_serversslsocketstream : public std::basic_iostream<Char> {
	public:
		typedef Char __char_type;
		typedef std::basic_iostream<__char_type> __stream_type;
		typedef basic_sslsocketbuf<__char_type> __buf_type;

	protected:
		__buf_type __buf;
		int __sockfd;
		SSL_CTX* __context;
		bool __chained;
	public:
		basic_serversslsocketstream() : __stream_type(&__buf), __sockfd(-1), __context(nullptr), __chained(false)  {
			SSL_library_init();
			OpenSSL_add_all_algorithms();
			SSL_load_error_strings();
			this->__context = SSL_CTX_new(SSLv3_server_method());
			if (this->__context == nullptr) {
				abort();
			};
		}
		basic_serversslsocketstream(int s) : __stream_type(&__buf), __sockfd(-1), __context(nullptr), __chained(false) {
			__buf.set_socket(s);

			SSL_library_init();
			OpenSSL_add_all_algorithms();
			SSL_load_error_strings();
			this->__context = SSL_CTX_new(SSLv3_server_method());
			if (this->__context == nullptr) {
				abort();
			}
		}
		virtual ~basic_serversslsocketstream() {
			this->close();
		}

		void close() {
			EVP_cleanup();
			SSL_CTX_free(this->__context);
			if (__buf.get_socket() != 0) {
				::shutdown(__buf.get_socket(), SHUT_RDWR);
				::close(__buf.get_socket());
			}
			__stream_type::clear();
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

		void certificates(SSL_CTX* _ctx, string _cert, string _key) {
			if (this->__chained) {
				if (SSL_CTX_use_certificate_chain_file(_ctx, _cert.data()) <= 0) {
					fprintf(stderr, "Chain file is not available or not valid\n");
					abort();
				}
			}
			else {
				if (SSL_CTX_use_certificate_file(_ctx, _cert.data(), SSL_FILETYPE_PEM) <= 0) {
					fprintf(stderr, "PEM file is not available or not valid\n");
					abort();
				}
			}
			if (SSL_CTX_use_PrivateKey_file(_ctx, _key.data(), SSL_FILETYPE_PEM) <= 0) {
				fprintf(stderr, "Private key file is not available or not valid\n");
				abort();
			}
			if (!SSL_CTX_check_private_key(_ctx)) {
				fprintf(stderr, "Private key does not match the public certificate\n");
				abort();
			}
		}

		bool bind(uint16_t _port, string _cert, string _key) {
			this->certificates(this->__context, _cert, _key);

			this->__sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (this->__sockfd < 0) {
				__stream_type::setstate(std::ios::failbit);
				return false;
			}

			int _opt = 1;
			if (setsockopt(this->__sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &_opt, sizeof(_opt)) == SO_ERROR) {
				::shutdown(this->__sockfd, SHUT_RDWR);
				::close(this->__sockfd);
				this->__sockfd = -1;
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
				this->__sockfd = -1;
				__buf.set_context(0);
				__stream_type::setstate(std::ios::failbit);
				return false;
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
					return false;
				}

				struct linger _so_linger;
				_so_linger.l_onoff = 1;
				_so_linger.l_linger = 30;
				::setsockopt(_newsockfd,SOL_SOCKET, SO_LINGER, &_so_linger, sizeof _so_linger);
				_out->assign(_newsockfd, this->__context);
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

	typedef basic_serversslsocketstream<char> serversslsocketstream;
	typedef basic_serversslsocketstream<wchar_t> wserversslsocketstream;

	typedef std::shared_ptr< zpt::serversslsocketstream > serversslsocketstream_ptr;
	typedef std::shared_ptr< zpt::wserversslsocketstream > wserversslsocketstream_ptr;

	#define CRLF "\r\n"

	class websocketsslserverstream : public basic_serversslsocketstream<char> {
	public:
		websocketsslserverstream() {
		};
		virtual ~websocketsslserverstream() {
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

	class websocketsslstream : public basic_sslsocketstream<char> {
	public:
		websocketsslstream(int _socket, SSL_CTX* _ctx) : basic_sslsocketstream<char>(_socket, _ctx) {
		};
		virtual ~websocketsslstream() {
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

	typedef std::shared_ptr< zpt::websocketsslstream > websocketsslstream_ptr;
	typedef std::shared_ptr< zpt::websocketsslserverstream > websocketsslserverstream_ptr;
	
}
