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

namespace zpt {
	// start at 1000
	enum ErrorCodeUsers {
		ERRNameMandatory = 1000,
		ERREmailMandatory = 1001,
		ERRPasswordMandatory = 1002,
		ERRConfirmationMandatory = 1003,
		ERRPasswordConfirmationDontMatch = 1004,
		ERRUserNotFound = 1005,
		ERRIDMandatory = 1006
	};

	class Users : public zpt::KnowledgeBase {
	public:
		Users(zpt::ev::emitter _emitter);
		virtual ~Users();

		virtual std::string name();
		
		virtual std::tuple< std::string, std::string > salt_hash(std::string _password);
		virtual bool validate(std::string _username, std::string _password);
		
		virtual zpt::json list(std::string _resource, zpt::json _envelope);
		virtual zpt::json get(std::string _resource, zpt::json _envelope);
		virtual zpt::json add(std::string _resource, zpt::json _envelope);
		virtual zpt::json replace(std::string _resource, zpt::json _envelope);
		virtual zpt::json patch(std::string _resource, zpt::json _envelope);
		virtual zpt::json remove(std::string _resource, zpt::json _envelope);

	private:
		zpt::ev::emitter __emitter;
	};

	namespace users {
		typedef zpt::Users broker;
	}
	
	class UsersPtr : public std::shared_ptr< zpt::Users > {
	public:
		UsersPtr(zpt::ev::emitter _emitter);
		virtual ~UsersPtr();
	};

}
