#pragma once

#include <json/JSONObj.h>
#include <parsers/JSONLexer.h>

namespace zapata {

	class JSONTokenizerLexer: public JSONLexer {
		public:
			JSONTokenizerLexer(std::istream &_in = std::cin, std::ostream &_out = std::cout, JSONObj* _rootobj = NULL, JSONArr* _rootarr = NULL);
			virtual ~JSONTokenizerLexer();

			void switchRoots(JSONObj* _rootobj, JSONArr* _rootarr);

			void result(zapata::JSONType _in);
			void finish(zapata::JSONType _in);

			void init(zapata::JSONType _in_type, const string _in_str);
			void init(zapata::JSONType _in_type);
			void init(bool _in);
			void init(long long _in);
			void init(double _in);
			void init(string _in);

			void add();

			JSONObj* __root_obj;
			JSONArr* __root_arr;
			JSONType __root_type;
			JSONElement* __value;
			JSONElement* __parent;
			vector<JSONElement*> __context;
	};

}
