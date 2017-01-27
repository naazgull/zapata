#include <zapata/rest.h>
#include <string>
$[resource.path.h]

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

extern "C" void restify(zpt::ev::emitter _emitter) {
_emitter->connector($[datum.connectors.initialize]);

$[resource.registry.begin]
_emitter->on("$[resource.topic.regex]",
{
$[resource.handler.get]
$[resource.handler.post]
$[resource.handler.put]
$[resource.handler.patch]
$[resource.handler.delete]
$[resource.handler.head]
},
$[resource.opts]
);
$[resource.registry.end]
}
