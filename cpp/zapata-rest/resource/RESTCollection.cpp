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

#include <resource/RESTCollection.h>

zapata::RESTCollection::RESTCollection(string _url_pattern) : RESTResource() {
}

zapata::RESTCollection::~RESTCollection(){
}

void zapata::RESTCollection::get(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTCollection::put(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTCollection::post(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP202);
}

void zapata::RESTCollection::remove(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}

void zapata::RESTCollection::head(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP204);
}

void zapata::RESTCollection::patch(HTTPReq& _req, HTTPRep& _rep) {
	_rep->status(zapata::HTTP405);
}
