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

#pragma once

#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/oauth2/config.h>
#include <string>
#include <map>
#include <memory>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {
	namespace authenticator {

		auto extract(zpt::json _envelope) -> std::string;
		
		class OAuth2 {
		public:
			OAuth2(zpt::json _options);
			virtual ~OAuth2();

			virtual auto options() -> zpt::json;
			virtual auto name() -> std::string;

			virtual auto authorize(zpt::ev::performative _performative, zpt::json _envelope, zpt::json _opts) -> zpt::json;
			virtual auto authorize(std::string _topic, zpt::json _envelope, zpt::json _roles_needed) -> zpt::json;
			virtual auto token(zpt::ev::performative _performative, zpt::json _envelope, zpt::json _opts) -> zpt::json;
			virtual auto refresh(zpt::ev::performative _performative, zpt::json _envelope, zpt::json _opts) -> zpt::json;
			virtual auto validate(std::string _access_token, zpt::json _opts) -> zpt::json;

			virtual auto retrieve_owner(zpt::json _envelope) -> zpt::json = 0;
			virtual auto retrieve_owner(std::string _owner, std::string _password, std::string _client_id) -> zpt::json = 0;
			virtual auto retrieve_client(zpt::json _envelope) -> zpt::json = 0;
			virtual auto retrieve_client(std::string _client_id, std::string _client_secret) -> zpt::json = 0;
			virtual auto store_token(zpt::json _token) -> std::string = 0;
			virtual auto get_code(std::string _code) -> zpt::json = 0;
			virtual auto get_token(std::string _access_token) -> zpt::json = 0;
			virtual auto get_refresh_token(std::string _refresh_token) -> zpt::json = 0;
			virtual auto get_roles_permissions(zpt::json _token) -> zpt::json = 0;
			virtual auto validate_roles_permissions(zpt::json _envelope, std::string _topic, zpt::json _permissions) -> bool = 0;
			virtual auto remove_token(zpt::json _token) -> void = 0;
			
		private:
			zpt::json __options;

			auto generate_token(zpt::json _data) ->zpt::json;
		};
		
	}
}
