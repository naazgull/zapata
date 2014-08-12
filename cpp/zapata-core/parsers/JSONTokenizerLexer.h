/*
    Author: Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>
    Copyright (c) 2014 Pedro (n@zgul)Figueiredo
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with Zapata.  If not, see <http://www.gnu.org/licenses/>.
*/

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
