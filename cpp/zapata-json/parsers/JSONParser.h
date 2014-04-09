#pragma once

#include <json/JSONObj.h>
#include <parsers/JSONTokenizer.h>

namespace zapata {

	class JSONParser: public JSONTokenizer {
		public:
			JSONParser(std::istream &_in = std::cin, std::ostream &_out = std::cout, JSONObj* _rootobj = NULL, JSONArr* _rootarr = NULL);
			virtual ~JSONParser();

			void switchRoots(JSONObj* _rootobj = NULL, JSONArr* _rootarr = NULL);
			void switchStreams(std::istream &_in = std::cin, std::ostream &_out = std::cout);
	};

}
