#pragma once

#include <zapata/base.h>
#include <zapata/json.h>

namespace zpt {
namespace config {
/**
 * @brief Loads configuration from parameters into the output JSON object.
 * @param _parameters Command-line parameters as JSON.
 * @param _output JSON object to populate with merged configuration.
 * @return void
 */
auto load(zpt::json _parameters, zpt::json& _output) -> void;
/**
 * @brief Returns the default configuration JSON with identity, logging, transport, and plugin
 * settings.
 * @return Default configuration JSON object.
 */
auto load_defaults() -> zpt::json;
} // namespace config
/**
 * @brief Returns the global configuration.
 * @return Global configuration JSON object.
 */
auto GLOBAL_CONFIG() -> zpt::json&;
} // namespace zpt
