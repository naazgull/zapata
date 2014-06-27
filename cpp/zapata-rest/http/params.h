#pragma once

#include <http/HTTPObj.h>
#include <json/JSONObj.h>
#include <resource/RESTResource.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void fromparams(HTTPReq& _in, JSONObj& _out, zapata::RESTfulType _resource_type = zapata::RESTfulResource, bool _regexp = false);

}
