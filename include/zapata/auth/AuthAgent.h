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

#include <unistd.h>
#include <pthread.h>
#include <string>
#include <functional>
#include <memory>
#include <zapata/base/assert.h>
#include <zapata/core.h>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zapata {
	class AuthAgent;
	class AuthAgentPtr;
	typedef std::function< void (zapata::JSONPtr& _auth_data, zapata::AuthAgentPtr&) > AuthAgentCallback;
	
	class AuthAgentPtr : public std::shared_ptr<zapata::AuthAgent> {
	public:
		AuthAgentPtr(zapata::AuthAgent * _target);
		virtual ~AuthAgentPtr();
	};

	class AuthAgent {
	public:
		AuthAgent(zapata::AuthAgentCallback _callback, zapata::JSONPtr& _options);
		virtual ~AuthAgent();

		virtual zapata::AuthAgentCallback& callback() final;
		virtual zapata::JSONPtr& options() final;

		virtual std::string code_url() = 0;
		virtual std::string token_url() = 0;
		virtual std::string refresh_token_url() = 0;

		virtual zapata::JSONPtr authenticate(zapata::JSONPtr _credentials) = 0;

	private:
		zapata::AuthAgentCallback __callback;
		zapata::JSONPtr __options;
	};

}
