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

std::string zpt::ssl_error_print(SSL * _ssl, int _ret) {
	switch(SSL_get_error(_ssl, _ret)) {
		case SSL_ERROR_NONE : {
			return string("SSL_ERROR_NONE: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
		case SSL_ERROR_ZERO_RETURN : {
			return string("SSL_ERROR_ZERO_RETURN: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
		case SSL_ERROR_WANT_READ :{
			return string("SSL_ERROR_WANT_READ: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
		case SSL_ERROR_WANT_WRITE : {
			return string("SSL_ERROR_WANT_WRITE: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
		case SSL_ERROR_WANT_CONNECT :{
			return string("SSL_ERROR_WANT_CONNECT: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
		case SSL_ERROR_WANT_ACCEPT : {
			return string("SSL_ERROR_WANT_ACCEPT: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
		case SSL_ERROR_WANT_X509_LOOKUP : {
			return string("SSL_ERROR_WANT_X509_LOOKUP: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
			/*case SSL_ERROR_WANT_ASYNC : {                                                                                                                                                                                                                          
                         return string("SSL_ERROR_WANT_ASYNC: ") + string(ERR_error_string(ERR_get_error(), nullptr));                                                                                                                                                    
			 }*/
		case SSL_ERROR_SYSCALL : {
			return string("SSL_ERROR_SYSCALL: ") + string(strerror(errno));
		}
		case SSL_ERROR_SSL : {
			return string("SSL_ERROR_SSL: ") + string(ERR_error_string(ERR_get_error(), nullptr));
		}
	}
	return "UNKNOW ERROR";
}
