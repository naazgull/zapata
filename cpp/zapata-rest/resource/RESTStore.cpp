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

#include <resource/RESTStore.h>

zapata::RESTStore::RESTStore(string _url_pattern) : RESTResource(_url_pattern) {
}

zapata::RESTStore::~RESTStore(){
}

void zapata::RESTStore::get(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTStore::put(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP202);
}

void zapata::RESTStore::post(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTStore::remove(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTStore::head(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTStore::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}
