#include <iostream>
#include <zapata/sqlite.h>
#include <zapata/startup.h>

/**
 * @brief Plugin load callback: configures the SQLite connector.
 * @param _plugin The plugin instance providing configuration.
 */
extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading SQLite connector", zpt::info);
    zpt::register_connector("sqlite", zpt::make_connection<zpt::storage::sqlite::connection>);
}

/**
 * @brief Plugin unload callback: cleans up the SQLite connector state.
 * @param _plugin The plugin instance being unloaded.
 */
extern "C" auto _zpt_unload_(zpt::plugin&) -> void { zlog("Unloaded SQLite connector", zpt::info); }
