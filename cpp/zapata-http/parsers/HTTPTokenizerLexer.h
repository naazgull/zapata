#pragma once

#include <http/HTTPObj.h>
#include <parsers/HTTPLexer.h>

namespace zapata {

	class HTTPTokenizerLexer: public HTTPLexer {
		public:
			HTTPTokenizerLexer(std::istream &_in = std::cin, std::ostream &_out = std::cout, HTTPReq* _rootreq = NULL, HTTPRep* _rootrep = NULL);
			virtual ~HTTPTokenizerLexer();

			void switchRoots(HTTPReq* _rootreq, HTTPRep* _rootrep);
			void justLeave();

			void init(zapata::HTTPType _in_type);
			void body();
			void url();
			void status();

			void add();
			void name();
			void value();

			HTTPReq* __root_req;
			HTTPRep* __root_rep;
			HTTPType __root_type;

	};

}
