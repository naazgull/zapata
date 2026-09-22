#include <iostream>
#include <zapata/mongodb.h>
#include <zapata/startup.h>

/**
 * @brief Plugin load callback: configures the MongoDB connector.
 * @param _plugin The plugin instance providing configuration.
 */
extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading MongoDB connector", zpt::info);
    zpt::register_connector("mongodb", zpt::make_connection<zpt::storage::mongodb::connection>);
}

/**
 * @brief Plugin unload callback: cleans up the MongoDB connector state.
 * @param _plugin The plugin instance being unloaded.
 */
extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloaded MongoDB connector", zpt::info);
}
