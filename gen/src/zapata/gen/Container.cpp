#include <string>
#include <zapata/rest.h>
$[resource.path.h]

  extern "C" void
  _zpt_plugin_load_() {
    zpt::ev::emitter _emitter = zpt::emitter<zpt::rest::emitter>();
    _emitter->connector($[datum.connectors.initialize]);

    $[resource.handlers.delegate]
}
