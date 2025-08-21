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

#pragma once

#include <zapata/base.h>
#include <zapata/globals.h>
#include <zapata/json.h>
#include <zapata/events.h>
#include <zapata/startup/configuration.h>

#include <typeinfo>

namespace zpt {

inline constexpr std::uint64_t PLUGIN_STATE_UNLOADED{ 0 };
inline constexpr std::uint64_t PLUGIN_STATE_IN_SHUTDOWN{ 1 };
inline constexpr std::uint64_t PLUGIN_STATE_LOADED{ 2 };

class plugin {
  public:
    using plugin_fn_type = std::function<bool(zpt::plugin& _plugin)>;

    plugin(zpt::json _options, zpt::json _config);
    virtual ~plugin();

    plugin(plugin const& _rhs) = delete;
    plugin(plugin&& _rhs) = delete;
    auto operator=(plugin const& _rhs) -> plugin& = delete;
    auto operator=(plugin&& _rhs) -> plugin& = delete;

    auto name() -> std::string&;
    auto source() -> std::string&;
    auto config() -> zpt::json&;
    auto is_shutdown_ongoing() -> bool;
    auto is_loaded() -> bool;
    auto is_unloaded() -> bool;

    auto add_thread(std::function<void()> _callback) -> plugin&;

  private:
    void* __lib_handler{ nullptr };
    std::string __name{ "" };
    std::string __source{ "" };
    bool __running{ false };
    zpt::json __config;
    std::vector<std::thread> __threads;
    zpt::padded_atomic<std::uint16_t> __state{ PLUGIN_STATE_UNLOADED };
};

namespace startup {

class boot {
  public:
    using plugin_map_element_type = std::unique_ptr<zpt::plugin>;

    boot(zpt::json _config);
    virtual ~boot();

    boot(boot const& _rhs) = delete;
    boot(boot&& _rhs) = delete;
    auto operator=(boot const& _rhs) -> boot& = delete;
    auto operator=(boot&& _rhs) -> boot& = delete;

    auto load() -> zpt::startup::boot&;
    auto unload() -> zpt::startup::boot&;
    auto to_string() -> std::string;

    friend std::ostream& operator<<(std::ostream& _out, zpt::startup::boot& _in) {
        _out << _in.to_string() << std::flush;
        return _out;
    }

  private:
    zpt::json __configuration;
    std::map<std::string, plugin_map_element_type> __plugins;
    std::vector<std::string> __load_order;

    auto resolve_builtin_dependencies() -> void;
    auto load(zpt::json _plugin_options, zpt::json _plugin_config) -> zpt::plugin&;
    auto hash(zpt::json& _event) -> std::string;
};

} // namespace startup

auto get_default_uri() -> std::string;

auto BOOT(zpt::json _config = nullptr) -> zpt::startup::boot&;
auto GLOBAL_CONFIG() -> zpt::json;
auto SELF() -> zpt::json const&;
} // namespace zpt
