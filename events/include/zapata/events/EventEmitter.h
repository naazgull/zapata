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
#include <ossp/uuid++.hh>

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
		typedef std::vector<pair<regex_t*, vector< zapata::ev::Handler> > > HandlerStack;
		typedef std::map< std::string, zapata::ev::Handler > ReplyHandlerStack;

		zapata::JSONPtr split(std::string _url, zapata::JSONPtr _orphans);
		std::string join(zapata::JSONPtr _info, size_t _orphans);
		std::string to_str(zapata::ev::Performative _performative);
		zapata::ev::Performative from_str(std::string _performative);

		zapata::JSONPtr init_request();
		zapata::JSONPtr init_reply(std::string _cid);
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
			virtual void off(zapata::ev::Performative _method, string _regex) = 0;
			virtual void off(string _regex) = 0;

			virtual zapata::JSONPtr trigger(zapata::ev::Performative _method, std::string _resource, zapata::JSONPtr _payload) = 0;
			virtual zapata::JSONPtr trigger(std::string _resource, zapata::JSONPtr _payload) = 0;

			virtual void add_kb(std::string _name, zapata::KBPtr _kb) final;
			virtual zapata::KBPtr get_kb(std::string _name) final;

		private:
			zapata::JSONObj __options;
			zapata::EventEmitterPtr __self;
			std::map<std::string, zapata::KBPtr> __kb;

	};

	zapata::JSONPtr split(std::string _to_split, std::string _separator);
	std::string join(zapata::JSONPtr _to_join, std::string _separator);

}

