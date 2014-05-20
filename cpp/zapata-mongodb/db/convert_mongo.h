#pragma once

#include <mongo/client/dbclient.h>
#include <json/JSONObj.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void frommongo(bson::bo& _in, zapata::JSONObj& _out);
	void frommongo(bson::be& _in, zapata::JSONArr& _out);

	void tomongo(zapata::JSONObj& _in, mongo::BSONObjBuilder&  _out);
	void tomongo(zapata::JSONArr& _in, mongo::BSONArrayBuilder&  _out);
}
