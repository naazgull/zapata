#pragma once

namespace zpt {
namespace runtime {
/**
 * @brief Initializes the application runtime: parses arguments, loads configuration,
 * boots the event dispatcher and transport layer, loads all plugins, and enters
 * the polling loop.
 * @param _argc Argument count from main.
 * @param _argv Argument vector from main.
 */
auto initialize(int _argc, char** _argv) -> void;
/** @brief Shuts down the stream polling loop. */
auto shutdown() -> void;
} // namespace runtime
} // namespace zpt
