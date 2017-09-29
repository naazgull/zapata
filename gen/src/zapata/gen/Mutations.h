#pragma once

#include <string>
#include <zapata/rest.h>
$[data.path.h]

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

$[namespaces.begin]
auto mutify(zpt::ev::emitter _emitter) -> void;
$[namepsaces.end]
