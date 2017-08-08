#include <zapata/rest.h>
#include <string>
$[mutations.api.path.h]

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

extern "C" void _zpt_load_() {
zpt::ev::emitter _emitter = zpt::emitter< zpt::rest::emitter >();

$[mutations.handlers.delegate]
}
