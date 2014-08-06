/*
    This file is part of Zapata.

    Zapata is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Zapata is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <parsers/HTTPParser.h>

zapata::HTTPParser::HTTPParser(std::istream &_in, std::ostream &_out, zapata::HTTPReq* _rootobj, zapata::HTTPRep* _rootarr) {
	this->d_scanner.switchStreams(_in, _out);
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

zapata::HTTPParser::~HTTPParser() {
}

void zapata::HTTPParser::switchRoots(HTTPReq* _rootobj, HTTPRep* _rootarr) {
	this->d_scanner.switchRoots(_rootobj, _rootarr);
}

void zapata::HTTPParser::switchStreams(std::istream &_in, std::ostream &_out) {
	this->d_scanner.switchStreams(_in, _out);
}

