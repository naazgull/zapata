/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file startup.h
 * @brief Plugin management and application bootstrap.
 *
 * Provides dynamic plugin loading and application lifecycle management.
 * Plugins are loaded as shared libraries and can register event handlers,
 * transports, and other components.
 *
 * @par Plugin Entry Point
 * Plugins must export a function with this signature:
 * @code
 * extern "C" void _zpt_load_(zpt::plugin& plugin);
 * @endcode
 */

#pragma once

#include <typeinfo>
#include <zapata/base.h>
#include <zapata/events.h>
#include <zapata/globals.h>
#include <zapata/json.h>
#include <zapata/startup/configuration.h>

namespace zpt {

/** @brief Plugin state: not loaded. */
inline constexpr std::uint64_t PLUGIN_STATE_UNLOADED{ 0 };
/** @brief Plugin state: shutdown in progress. */
inline constexpr std::uint64_t PLUGIN_STATE_IN_SHUTDOWN{ 1 };
/** @brief Plugin state: loaded and running. */
inline constexpr std::uint64_t PLUGIN_STATE_LOADED{ 2 };

/**
 * @brief Dynamic plugin wrapper.
 *
 * Manages the lifecycle of a dynamically loaded shared library plugin.
 * Plugins can register threads, event handlers, and access configuration.
 *
 * @par Example Plugin
 * @code
 * // my_plugin.cpp
 * extern "C" void _zpt_load_(zpt::plugin& plugin) {
 *     auto& config = plugin.config();
 *     // Register handlers, start threads, etc.
 *     plugin.add_thread([&]() {
 *         while (!plugin.is_shutdown_ongoing()) {
 *             // Worker loop
 *         }
 *     });
 * }
 * @endcode
 */
class plugin {
  public:
    /** @brief Plugin entry point function signature. */
    using plugin_fn_type = std::function<bool(zpt::plugin& _plugin)>;

    /**
     * @brief Constructs a plugin with options and configuration.
     * @param _options Plugin options JSON.
     * @param _config Global configuration JSON.
     * @return void (constructors implicitly initialize the object).
     */
    plugin(zpt::json _options, zpt::json _config);
    /**
     * @brief Destructor. Stops plugin threads if running.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~plugin();

    plugin(plugin const& _rhs) = delete;
    plugin(plugin&& _rhs) = delete;
    auto operator=(plugin const& _rhs) -> plugin& = delete;
    auto operator=(plugin&& _rhs) -> plugin& = delete;

    /**
     * @brief Returns the plugin name.
     * @return Reference to plugin name string.
     */
    auto name() -> std::string&;
    /**
     * @brief Returns the shared library path.
     * @return Reference to source path string.
     */
    auto source() -> std::string&;
    /**
     * @brief Returns the plugin configuration.
     * @return Reference to plugin configuration JSON.
     */
    auto config() -> zpt::json&;
    /**
     * @brief Returns true if shutdown is in progress.
     * @return True if shutdown is ongoing.
     */
    auto is_shutdown_ongoing() -> bool;
    /**
     * @brief Returns true if plugin is loaded.
     * @return True if plugin is loaded.
     */
    auto is_loaded() -> bool;
    /**
     * @brief Returns true if plugin is unloaded.
     * @return True if plugin is unloaded.
     */
    auto is_unloaded() -> bool;

    /**
     * @brief Registers a worker thread.
     * @param _callback Worker loop callback function.
     * @return Reference to this plugin.
     */
    auto add_thread(std::function<void()> _callback) -> plugin&;

  private:
    /** @brief Shared library handle from dlopen. */
    void* __lib_handler{ nullptr };
    /** @brief Plugin name. */
    std::string __name{ "" };
    /** @brief Path to the shared library source file. */
    std::string __source{ "" };
    /** @brief Whether the plugin is currently running. */
    bool __running{ false };
    /** @brief Plugin configuration JSON. */
    zpt::json __config;
    /** @brief Worker threads registered via add_thread. */
    std::vector<std::thread> __threads;
    /** @brief Plugin lifecycle state. */
    zpt::padded_atomic<std::uint16_t> __state{ PLUGIN_STATE_UNLOADED };
};

namespace startup {

/**
 * @brief Application boot manager.
 *
 * Manages application startup by loading configuration and plugins
 * in dependency order.
 *
 * @par Example
 * @code
 * auto& boot = zpt::BOOT(config);
 * boot.load();  // Load all plugins
 *
 * // ... application runs ...
 *
 * boot.unload();  // Clean shutdown
 * @endcode
 */
class boot {
  public:
    using plugin_map_element_type = std::unique_ptr<zpt::plugin>;

    /**
     * @brief Constructs boot manager with configuration.
     * @param _config Application configuration JSON.
     * @return void (constructors implicitly initialize the object).
     */
    boot(zpt::json _config);
    /**
     * @brief Destructor.
     * @return void (destructors implicitly clean up the object).
     */
    virtual ~boot();

    boot(boot const& _rhs) = delete;
    boot(boot&& _rhs) = delete;
    auto operator=(boot const& _rhs) -> boot& = delete;
    auto operator=(boot&& _rhs) -> boot& = delete;

    /**
     * @brief Loads all plugins in dependency order.
     * @return Reference to this boot manager.
     */
    auto load() -> zpt::startup::boot&;
    /**
     * @brief Unloads all plugins in reverse order.
     * @return Reference to this boot manager.
     */
    auto unload() -> zpt::startup::boot&;
    /**
     * @brief Returns string representation of loaded plugins.
     * @return String with plugin names and states.
     */
    auto to_string() -> std::string;

    friend std::ostream& operator<<(std::ostream& _out, zpt::startup::boot& _in) {
        _out << _in.to_string() << std::flush;
        return _out;
    }

  private:
    /** @brief Application configuration JSON. */
    zpt::json __configuration;
    /** @brief Map of loaded plugins, keyed by name. */
    std::map<std::string, plugin_map_element_type> __plugins;
    /** @brief Order in which plugins were loaded. */
    std::vector<std::string> __load_order;

    /**
     * @brief Resolves and adds builtin plugin dependencies from __builtins metadata.
     * @return void
     */
    auto resolve_builtin_dependencies() -> void;
    /**
     * @brief Internal helper that loads a single plugin from options and config.
     * @param _plugin_options Plugin-specific options JSON.
     * @param _plugin_config Plugin configuration JSON.
     * @return Reference to the newly created plugin.
     */
    auto load(zpt::json _plugin_options, zpt::json _plugin_config) -> zpt::plugin&;
    /**
     * @brief Computes a string hash for a plugin event (plugin/step).
     * @param _event Event JSON to hash.
     * @return Hash string.
     */
    auto hash(zpt::json& _event) -> std::string;
};

} // namespace startup

/**
 * @brief Returns the default URI for this service instance.
 * @return URI string for the service.
 */
auto get_default_uri() -> std::string;

/**
 * @brief Returns the global boot manager instance.
 * @param _config Configuration (used only on first call).
 * @return Reference to the global boot manager.
 */
auto BOOT(zpt::json _config = nullptr) -> zpt::startup::boot&;
/**
 * @brief Returns the global configuration.
 * @return Global configuration JSON object.
 */
auto GLOBAL_CONFIG() -> zpt::json;
/**
 * @brief Returns the service identity.
 * @return Service identity JSON object.
 */
auto IDENTITY() -> zpt::json const&;
} // namespace zpt
