#include <zapata/events/dispatcher.h>
#include <zapata/json.h>

namespace zpt {
namespace events {} // namespace events
} // namespace zpt

extern "C" auto
_zpt_plugin_load_(zpt::json _config) -> void {
}
