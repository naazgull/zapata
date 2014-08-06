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

#pragma once

#include <json/JSONObj.h>
#include <mongo/client/dbclient.h>
#include <mongo/bson/bsonelement.h>
#include <mongo/bson/bsonobjbuilder.h>
#include <stddef.h>
#include <string>

namespace mongo {
	class ScopedDbConnection;
}

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void frommongo(bson::bo& _in, zapata::JSONObj& _out);
	void frommongo(bson::be& _in, zapata::JSONArr& _out);

	void tomongo(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _out);
	void tomongo(zapata::JSONArr& _in, mongo::BSONArrayBuilder&  _out);

	void tomongoquery(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _query, mongo::BSONObjBuilder& _order, size_t& _page_size, size_t& _page_start_index);

	void torestcollection(mongo::ScopedDbConnection* _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out);
	void toreststore(mongo::ScopedDbConnection* _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out);
	void torestdocument(mongo::ScopedDbConnection* _conn, string _mongo_collection, zapata::JSONObj& _params, zapata::JSONObj& _out);
}
