/*
Copyright (c) 2014, Muzzley

Permission to use, copy, modify, and/or distribute this software for 
any purpose with or without fee is hereby granted, provided that the 
above copyright notice and this permission notice appear in all 
copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE 
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR 
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER 
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
PERFORMANCE OF THIS SOFTWARE.
*/

#pragma once

#include <zapata/http/config.h>

#include <zapata/http/HTTPObj.h>
#include <zapata/http/HTTPLexer.h>

namespace zapata {

	class HTTPTokenizerLexer: public HTTPLexer {
	public:
		HTTPTokenizerLexer(std::istream &_in = std::cin, std::ostream &_out = std::cout);
		virtual ~HTTPTokenizerLexer();

		void switchRoots(HTTPReq& _root);
		void switchRoots(HTTPRep& _root);
		void justLeave();

		void init(zapata::HTTPType _in_type);
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
