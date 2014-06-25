#pragma once

#include <zapata/net.h>
#include <zapata/http.h>

using namespace std;
using namespace __gnu_cxx;

namespace zapata {

	void send(HTTPReq& _in, HTTPRep& _out, bool _ssl = false);

}
