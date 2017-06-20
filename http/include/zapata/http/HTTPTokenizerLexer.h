/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <n@zgul.me>

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

#include <zapata/http/config.h>

#include <zapata/http/HTTPObj.h>
#include <zapata/http/HTTPLexer.h>

namespace zpt {

	class HTTPTokenizerLexer: public HTTPLexer {
	public:
		HTTPTokenizerLexer(std::istream &_in = std::cin, std::ostream &_out = std::cout);
		virtual ~HTTPTokenizerLexer();

		void switchRoots(HTTPReq& _root);
		void switchRoots(HTTPRep& _root);
		void justLeave();

		void init(zpt::HTTPType _in_type);
		void body();
		void url();
		void status();

		void add();
		void name();
		void value();

		string __header_name;
		string __param_name;
		HTTPReqT* __root_req;
		HTTPRepT* __root_rep;
		HTTPType __root_type;

	};

}
