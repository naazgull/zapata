/*
The MIT License (MIT)

Copyright (c) 2014 Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <zapata/json/JSONObj.h>
#include <mongo/client/dbclient.h>
#include <mongo/bson/bsonelement.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <stddef.h>
#include <string>

namespace mongo {
	class ScopedDbConnection;
}

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zapata {

	void frommongo(mongo::BSONObj& _in, zapata::JSONObj& _out);
	void frommongo(mongo::BSONElement& _in, zapata::JSONArr& _out);

	void tomongo(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _out);
	void tomongo(zapata::JSONArr& _in, mongo::BSONArrayBuilder&  _out);

	void tomongoquery(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _query, mongo::BSONObjBuilder& _order, size_t& _page_size, size_t& _page_start_index);

	void torestcollection(mongo::ScopedDbConnection& _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out);
	void toreststore(mongo::ScopedDbConnection& _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out);
	void torestdocument(mongo::ScopedDbConnection& _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out);

	float valid_mongo_version();
}

extern "C" int zapata_mongodb();
