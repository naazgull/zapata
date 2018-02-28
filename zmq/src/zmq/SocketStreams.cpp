/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

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

#include <zapata/zmq/SocketStreams.h>

std::string zpt::ssl_error_print(SSL* _ssl, int _ret) {
	switch (SSL_get_error(_ssl, _ret)) {
	case SSL_ERROR_NONE: {
		return std::string("SSL_ERROR_NONE: ") + std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	case SSL_ERROR_ZERO_RETURN: {
		return std::string("SSL_ERROR_ZERO_RETURN: ") + std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	case SSL_ERROR_WANT_READ: {
		return std::string("SSL_ERROR_WANT_READ: ") + std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	case SSL_ERROR_WANT_WRITE: {
		return std::string("SSL_ERROR_WANT_WRITE: ") + std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	case SSL_ERROR_WANT_CONNECT: {
		return std::string("SSL_ERROR_WANT_CONNECT: ") +
		       std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	case SSL_ERROR_WANT_ACCEPT: {
		return std::string("SSL_ERROR_WANT_ACCEPT: ") + std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	case SSL_ERROR_WANT_X509_LOOKUP: {
		return std::string("SSL_ERROR_WANT_X509_LOOKUP: ") +
		       std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	/*case SSL_ERROR_WANT_ASYNC : {
	 return std::string("SSL_ERROR_WANT_ASYNC: ") + std::string(ERR_error_string(ERR_get_error(), nullptr));
	 }*/
	case SSL_ERROR_SYSCALL: {
		return std::string("SSL_ERROR_SYSCALL: ") + std::string(strerror(errno));
	}
	case SSL_ERROR_SSL: {
		return std::string("SSL_ERROR_SSL: ") + std::string(ERR_error_string(ERR_get_error(), nullptr));
	}
	}
	return "UNKNOW ERROR";
}

std::string zpt::ssl_error_print(unsigned long _error) {
	if (_error == 0) {
		_error = ERR_get_error();
	}
	return std::string("SSL_ERROR_") + std::to_string(_error) + std::string(": ") +
	       std::string(ERR_error_string(_error, nullptr));
}
