#include <iostream>
#include <zapata/pgsql.h>
#include <zapata/startup.h>

/**
 * @brief Plugin load callback: configures the PostgreSQL connector.
 * @param _plugin The plugin instance providing configuration.
 */
extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading PostgreSQL connector", zpt::info);
    zpt::register_connector("pgsql", zpt::make_connection<zpt::storage::pgsql::connection>);
}

/**
 * @brief Plugin unload callback: cleans up the PostgreSQL connector state.
 * @param _plugin The plugin instance being unloaded.
 */
extern "C" auto _zpt_unload_(zpt::plugin&) -> void {
    zlog("Unloaded PostgreSQL connector", zpt::info);
}
