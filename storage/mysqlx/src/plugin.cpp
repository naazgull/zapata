#include <iostream>
#include <zapata/mysqlx.h>
#include <zapata/startup.h>

/**
 * @brief Plugin load callback: configures the MySQL connector.
 * @param _plugin The plugin instance providing configuration.
 */
extern "C" auto _zpt_load_(zpt::plugin&) -> void {
    zlog("Loading MySQL connector", zpt::info);
    zpt::storage::mysqlx::init();
    zpt::register_connector("mysqlx", zpt::make_connection<zpt::storage::mysqlx::connection>);
}

/**
 * @brief Plugin unload callback: cleans up the MySQL connector state.
 * @param _plugin The plugin instance being unloaded.
 */
extern "C" auto _zpt_unload_(zpt::plugin&) -> void { zlog("Unloaded MySQL connector", zpt::info); }
