#pragma once

#include <string>
#include <zapata/rest.h>
$[data.path.h]

    using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

$[namespaces.begin] auto restify(zpt::ev::emitter _emitter) -> void;
$[namespaces.end]
