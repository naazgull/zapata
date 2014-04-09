#pragma once

#include <http/HTTPObj.h>
#include <parsers/HTTPTokenizer.h>

namespace zapata {

	class HTTPParser: public HTTPTokenizer {
		public:
			HTTPParser(std::istream &_in = std::cin, std::ostream &_out = std::cout, HTTPReq* _rootreq = NULL, HTTPRep* _rootrep = NULL);
			virtual ~HTTPParser();

			void switchRoots(HTTPReq* _rootreq = NULL, HTTPRep* _rootrep = NULL);
			void switchStreams(std::istream &_in = std::cin, std::ostream &_out = std::cout);
	};

}
