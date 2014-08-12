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
