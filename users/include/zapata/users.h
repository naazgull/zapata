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
#include <zmq.hpp>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {
	namespace authenticator {
		
		class OAuth2 : public zpt::KnowledgeBase {
		public:
			OAuth2(zpt::json _options);
			virtual ~OAuth2();

			virtual zpt::json options();
			virtual std::string name();

			virtual zpt::json authorize(zpt::ev::performative _performative, zpt::json _envelope, zpt::ev::emitter _emitter);
			virtual zpt::json token(zpt::ev::performative _performative, zpt::json _envelope, zpt::ev::emitter _emitter);
			virtual zpt::json refresh(zpt::ev::performative _performative, zpt::json _envelope, zpt::ev::emitter _emitter);
			virtual zpt::json validate(std::string _access_token, zpt::ev::emitter _emitter);
			
		private:
			zpt::json __options;

			zpt::json generate_token(zpt::json _owner, zpt::json _application, std::string _client_id, std::string _client_secret, std::string _scope, std::string _grant_type, zpt::ev::emitter _emitter);
			zpt::json generate_token(zpt::json _owner, std::string _application_url, std::string _client_id, std::string _client_secret, std::string _scope, std::string _grant_type, zpt::ev::emitter _emitter);
			zpt::json generate_token(std::string _owner_url, std::string _application_url, std::string _client_id, std::string _client_secret, std::string _scope, std::string _grant_type, zpt::ev::emitter _emitter);
		};
		
	}
}
