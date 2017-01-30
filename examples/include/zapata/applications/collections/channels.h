#pragma once

#include <string>
#include <zapata/rest.h>
#include <zapata/applications/datums/ResourceOwners.h>
#include <zapata/applications/datums/Applications.h>
#include <zapata/applications/datums/MyApplications.h>
#include <zapata/applications/datums/MyUsers.h>


using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {
namespace apps {
namespace collections {
namespace channels {

auto restify(zpt::ev::emitter _emitter) -> void;
}
}
}
}


