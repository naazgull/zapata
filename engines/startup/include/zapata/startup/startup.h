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

auto
BOOT_ENGINE() -> ssize_t&;
auto
GLOBAL_CONFIG() -> ssize_t&;

class plugin {
  private:
    class plugin_t {
      public:
        plugin_t() = default;
        plugin_t(zpt::json _options, zpt::json _config);
        virtual ~plugin_t();

        auto operator->() -> zpt::plugin::plugin_t*;

        auto initialize(zpt::json _config) -> zpt::plugin::plugin_t&;
        auto handler() -> void*;
        auto name() -> std::string&;
        auto source() -> std::string&;
        auto requirements() -> zpt::json&;
        auto config() -> zpt::json&;
        auto is_running() -> bool;

        auto set_handler(void* _handler) -> zpt::plugin::plugin_t&;

        auto set_loader(std::function<bool(zpt::plugin& _plugin)> _loader)
          -> zpt::plugin::plugin_t&;
        auto load_plugin(zpt::plugin& _other) -> zpt::plugin::plugin_t&;
        auto set_unloader(std::function<bool(zpt::plugin& _plugin)> _loader)
          -> zpt::plugin::plugin_t&;
        auto unload_plugin(zpt::plugin& _other) -> zpt::plugin::plugin_t&;

      private:
        void* __lib_handler{ nullptr };
        std::string __name{ "" };
        std::string __source{ "" };
        zpt::json __requirements{ zpt::json::object() };
        std::function<bool(zpt::plugin& _plugin)> __loader;
        std::function<bool(zpt::plugin& _plugin)> __unloader;
        bool __running{ false };
        zpt::json __config;
    };

  public:
    using reference = plugin_t*;

    plugin() = default;
    plugin(zpt::json _options, zpt::json _config);
    plugin(zpt::plugin const& _rhs);
    plugin(zpt::plugin&& _rhs);
    virtual ~plugin() = default;

    auto operator=(zpt::plugin const& _rhs) -> zpt::plugin&;
    auto operator=(zpt::plugin&& _rhs) -> zpt::plugin&;
    auto operator->() -> reference;

    friend std::ostream& operator<<(std::ostream& _out, zpt::plugin& _in) {
        _out << zpt::pretty{ zpt::json{
                  "name", _in->name(), "source", _in->source(), "requires", _in->requirements() } }
             << std::flush;
        return _out;
    }

  private:
    std::shared_ptr<zpt::plugin::plugin_t> __underlying;
};

namespace startup {
enum steps { SEARCH = 0, LOAD = 1, CONFIGURATION = 2, RUN = 3, UNLOAD = 4 };

class engine : public zpt::events::dispatcher<zpt::startup::engine, zpt::json, bool> {
  public:
    using hazard_domain =
      zpt::events::dispatcher<zpt::startup::engine, zpt::json, bool>::hazard_domain;
    
    engine();
    engine(zpt::json _args);
    virtual ~engine();

    auto initialize(zpt::json _args) -> zpt::startup::engine&;

    auto trapped(zpt::json _event, bool _content) -> void;
    auto listen_to(zpt::json _event, std::function<void(bool)> _callback) -> void;
    auto report_error(zpt::json& _event,
                      bool& _content,
                      const char* _what,
                      const char* _description = nullptr,
                      const char* _backtrace = nullptr,
                      int _error = -1,
                      int status = 500) -> bool;

    auto to_string() -> std::string;

    template<typename... Args>
    auto add_thread(std::function<void(Args...)> _callback, Args... _args) -> zpt::startup::engine&;
    auto add_thread(std::function<void()> _callback) -> zpt::startup::engine&;

    auto load(zpt::json _plugin_options, zpt::json _plugin_config) -> zpt::plugin&;
    auto start() -> zpt::startup::engine&;
    auto unload() -> bool;
    auto exit() -> void;

    friend std::ostream& operator<<(std::ostream& _out, zpt::startup::engine& _in) {
        _out << _in.to_string() << std::flush;
        return _out;
    }

  private:
    hazard_domain __hazard_domain{ 2, 2 };
    zpt::json& __configuration;
    std::map<std::string, std::vector<std::function<void(bool)>>> __callbacks;
    std::map<std::string, zpt::plugin> __plugins;
    std::vector<std::string> __load_order;
    zpt::locks::spin_lock __plugin_list_lock;
    std::vector<std::thread> __workers;
    std::atomic<bool> __unloaded{ false };

    auto load() -> zpt::startup::engine&;
    auto hash(zpt::json& _event) -> std::string;
    auto check_requirements(zpt::plugin& _plugin, std::function<void(bool)> _callback = nullptr)
      -> bool;
    auto load_plugin(zpt::plugin& _plugin) -> zpt::startup::engine&;
    auto add_plugin(zpt::plugin& _plugin, zpt::json& _config) -> zpt::startup::engine&;
    auto unload_plugin(zpt::plugin& _plugin) -> zpt::startup::engine&;
};

namespace dynlib {
auto
load_plugin(zpt::plugin& _plugin) -> bool;
auto
unload_plugin(zpt::plugin& _plugin) -> bool;
} // namespace synlib

} // namespace startup
} // namespace zpt

template<typename... Args>
auto
zpt::startup::engine::add_thread(std::function<void(Args...)> _callback, Args... _args)
  -> zpt::startup::engine& {
    this->__workers.emplace_back(_callback, _args...);
    return (*this);
}
