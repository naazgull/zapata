#pragma once

#include <zapata/json.h>

namespace zpt {
namespace runtime {
/**
 * @brief Initializes the application runtime: parses arguments, loads configuration,
 * boots the event dispatcher and transport layer, loads all plugins, and enters
 * the polling loop.
 * @param _argc Argument count from main.
 * @param _argv Argument vector from main.
 * @param _default_config The default configuration, if any (default: zpt::undefined)
 * @return void
 */
auto initialize(int _argc, char** _argv, zpt::json const& _default_config = zpt::undefined) -> void;
/** @brief Shuts down the stream polling loop.
 * @return void
 */
auto shutdown() -> void;
/** @brief Whether or not the system is in shutdown.
 * @return True if the system is in shutdown;
 */
auto is_in_shutdown() -> bool;
} // namespace runtime
} // namespace zpt
