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
#include <zapata/events.h>
#include <regex.h>
#include <string>
#include <map>
#include <memory>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {

	class Addons;

	typedef std::shared_ptr<zpt::Addons> AddonsPtr;

	class Addons : public zpt::EventEmitter {
		public:
			Addons(zpt::JSONPtr _options);
			virtual ~Addons();

			virtual void on(string _regex,  zpt::ev::Handler _handler);
			virtual void on(zpt::ev::Performative _method, string _regex,  zpt::ev::Handler _handler);
			virtual void on(string _regex,  zpt::ev::Handler _handlers[7]);
			virtual void off(zpt::ev::Performative _method, string _regex);
			virtual void off(string _regex);

			virtual zpt::JSONPtr trigger(std::string _resource, zpt::JSONPtr _payload);
			virtual zpt::JSONPtr trigger(zpt::ev::Performative _method, std::string _resource, zpt::JSONPtr _payload);

		private:
			zpt::ev::HandlerStack __resources;
	};

}

