#include <zapata/rest.h>
#include <string>
$[resource.path.h]

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

extern "C" void restify(zpt::ev::emitter _emitter) {
_emitter->connector($[datum.connectors.initialize]);

$[resource.handlers.delegate]
}
