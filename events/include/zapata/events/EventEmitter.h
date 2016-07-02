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

namespace zpt {

	class EventEmitter;

	typedef std::weak_ptr<zpt::EventEmitter> EventEmitterWPtr;
	typedef std::shared_ptr<zpt::EventEmitter> EventEmitterPtr;

	namespace ev {
		typedef std::function<zpt::JSONPtr (zpt::ev::Performative _method, std::string _resource, zpt::JSONPtr, zpt::EventEmitterPtr)> Handler;
		typedef Handler Callback;
		typedef std::map< std::string, pair<regex_t*, vector< zpt::ev::Handler> > > HandlerStack;
		typedef std::map< std::string, zpt::ev::Handler > ReplyHandlerStack;

		zpt::JSONPtr split(std::string _url, zpt::JSONPtr _orphans);
		std::string join(zpt::JSONPtr _info, size_t _orphans);
		std::string to_str(zpt::ev::Performative _performative);
		zpt::ev::Performative from_str(std::string _performative);

		zpt::JSONPtr init_request(std::string _cid = "");
		zpt::JSONPtr init_reply(std::string _cid = "");
	}

	class EventEmitter {
	public:
		EventEmitter();
		EventEmitter(zpt::JSONPtr _options);
		virtual ~EventEmitter();
		
		virtual zpt::JSONPtr options();
		virtual zpt::EventEmitterPtr self();
		
		virtual std::string on(zpt::ev::Performative _method, string _regex,  zpt::ev::Handler _handler) = 0;
		virtual std::string on(std::string _regex,  zpt::ev::Handler _handlers[7]) = 0;
		virtual std::string on(string _regex,  std::map< zpt::ev::Performative, zpt::ev::Handler > _handlers) = 0;
		virtual void off(zpt::ev::Performative _method, std::string _callback_id) = 0;
		virtual void off(std::string _callback_id) = 0;
		
		virtual zpt::JSONPtr trigger(zpt::ev::Performative _method, std::string _resource, zpt::JSONPtr _payload) = 0;
		virtual zpt::JSONPtr route(zpt::ev::Performative _method, std::string _resource, zpt::JSONPtr _payload) = 0;
		
		virtual void add_kb(std::string _name, zpt::KBPtr _kb) final;
		virtual zpt::KBPtr get_kb(std::string _name) final;

	private:
		zpt::JSONPtr __options;
		zpt::EventEmitterPtr __self;
		std::map<std::string, zpt::KBPtr> __kb;

	};

	zpt::JSONPtr split(std::string _to_split, std::string _separator);
	std::string join(zpt::JSONPtr _to_join, std::string _separator);

}

