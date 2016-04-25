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

#include <zapata/base.h>
#include <zapata/json.h>
#include <regex.h>
#include <string>
#include <map>
#include <memory>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zapata {

	class EventEmitter;

	typedef std::shared_ptr<zapata::EventEmitter> EventEmitterPtr;

	namespace ev {
		typedef std::function<zapata::JSONPtr (zapata::ev::Performative _method, std::string _resource, zapata::JSONPtr, zapata::EventEmitterPtr)> Handler;
		typedef Handler Callback;
	}

	class EventEmitter {
		public:
			EventEmitter();
			explicit EventEmitter(zapata::JSONObj& _options);
			virtual ~EventEmitter();

			virtual zapata::JSONObj& options();
			virtual zapata::EventEmitterPtr self();

			virtual void on(zapata::ev::Performative _method, string _regex,  zapata::ev::Handler _handler) = 0;
			virtual void on(string _regex,  zapata::ev::Handler _handlers[9]) = 0;

			virtual zapata::JSONPtr trigger(zapata::ev::Performative _method, std::string _resource, zapata::JSONPtr _payload) = 0;

			virtual void add_kb(std::string _name, zapata::KBPtr _kb) final;
			virtual zapata::KBPtr get_kb(std::string _name) final;

		private:
			zapata::JSONObj __options;
			zapata::EventEmitterPtr __self;
			std::map<std::string, zapata::KBPtr> __kb;

	};

	void fromstr(string& _in, zapata::ev::Performative * _out);

}

