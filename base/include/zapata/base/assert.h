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

#include <memory>
#include <ctime>
#include <cstring>
#include <zapata/exceptions/AssertionException.h>

/**
 * Compact form for throwing exceptions when validating logical requirements and input/output validation
 * @param x a boolean expression to be validated
 * @param y the error message
 * @param z the HTTP status code to be replied to the invoking HTTP client
 */
#define assertz(x,y,z,c) if (! (x)) { throw zpt::AssertionException(y, z, c, #x, __LINE__, __FILE__); }

typedef struct epoll_event epoll_event_t;

namespace zpt {
	enum JSONType {
		JSObject, JSArray, JSString, JSInteger, JSDouble, JSBoolean, JSNil, JSDate, JSLambda
	};

	class Connector {
	public:
		virtual auto name() -> std::string = 0;
		
	};
	typedef std::shared_ptr<zpt::Connector> ConnectorPtr;
	typedef ConnectorPtr connector;

	namespace ev {
		enum performative {
			Get = 0,
			Put = 1,
			Post = 2,
			Delete = 3,
			Head = 4,
			Options = 5,
			Patch = 6,
			Reply = 7
		};

		std::string to_str(zpt::ev::performative _performative);
		zpt::ev::performative from_str(std::string _performative);
	}

	extern std::string* tz;
	std::string get_tz();

        typedef std::shared_ptr< std::tm > tm_ptr;

	zpt::tm_ptr get_time(time_t _t);
}
