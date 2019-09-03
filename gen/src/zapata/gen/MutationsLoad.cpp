#include <string>
#include <zapata/rest.h>
$[mutations.api.path.h]

  extern "C" void
  _zpt_load_() {
    zpt::ev::emitter _emitter = zpt::emitter<zpt::rest::emitter>();

    $[mutations.handlers.delegate]
}
