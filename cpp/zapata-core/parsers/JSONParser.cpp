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

#include <parsers/JSONParser.h>

zapata::JSONParser::JSONParser(std::istream &_in, std::ostream &_out, zapata::JSONObj* _rootobj, zapata::JSONArr* _rootarr) {
	this->d_scanner.switchStreams(_in, _out);
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

zapata::JSONParser::~JSONParser() {
}

void zapata::JSONParser::switchRoots(JSONObj* _rootobj, JSONArr* _rootarr) {
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

void zapata::JSONParser::switchStreams(std::istream &_in, std::ostream &_out) {
	this->d_scanner.switchStreams(_in, _out);
}

