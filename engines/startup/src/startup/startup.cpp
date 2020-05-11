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

#include <dlfcn.h>
#include <zapata/transport.h>
#include <zapata/startup/startup.h>

auto
zpt::BOOT_ENGINE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}
auto
zpt::GLOBAL_CONFIG() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}
auto
zpt::STREAM_POLLING() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}
auto
zpt::TRANSPORT_LAYER() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::plugin::plugin(zpt::json _options, zpt::json _config)
  : __underlying{ std::make_shared<zpt::plugin::plugin_t>(_options, _config) } {}

zpt::plugin::plugin(zpt::plugin const& _rhs) {
    this->__underlying = _rhs.__underlying;
}

zpt::plugin::plugin(zpt::plugin&& _rhs) {
    this->__underlying = std::move(_rhs.__underlying);
}

auto
zpt::plugin::operator=(zpt::plugin const& _rhs) -> zpt::plugin& {
    this->__underlying = _rhs.__underlying;
    return (*this);
}

auto
zpt::plugin::operator=(zpt::plugin&& _rhs) -> zpt::plugin& {
    this->__underlying = std::move(_rhs.__underlying);
    return (*this);
}

auto zpt::plugin::operator-> () -> reference {
    return this->__underlying.get();
}

zpt::plugin::plugin_t::plugin_t(zpt::json _options, zpt::json _config)
  : __config{ _config } {
    this->initialize(_options);
}

zpt::plugin::plugin_t::~plugin_t() {
    if (this->__lib_handler != nullptr) {
        dlclose(this->__lib_handler);
    }
}

auto zpt::plugin::plugin_t::operator-> () -> zpt::plugin::plugin_t* {
    return this;
}

auto
zpt::plugin::plugin_t::initialize(zpt::json _options) -> zpt::plugin::plugin_t& {
    expect(_options["name"]->ok(), "missing name definition in plugin configuration", 500, 0);
    expect(_options["source"]->ok(),
           std::string("missing source definition in plugin configuration of ") +
             static_cast<std::string>(_options["name"]),
           500,
           0);
    this->__name.assign(static_cast<std::string>(_options["name"]));
    this->__source.assign(static_cast<std::string>(_options["source"]));
    for (auto [_, __, _req] : _options["requires"]) {
        this->__requirements << static_cast<std::string>(_req) << false;
    }
    return (*this);
}

auto
zpt::plugin::plugin_t::handler() -> void* {
    return this->__lib_handler;
}

auto
zpt::plugin::plugin_t::name() -> std::string& {
    return this->__name;
}

auto
zpt::plugin::plugin_t::source() -> std::string& {
    return this->__source;
}

auto
zpt::plugin::plugin_t::requirements() -> zpt::json& {
    return this->__requirements;
}

auto
zpt::plugin::plugin_t::config() -> zpt::json& {
    return this->__config;
}

auto
zpt::plugin::plugin_t::is_running() -> bool {
    return this->__running;
}

auto
zpt::plugin::plugin_t::set_handler(void* _handler) -> zpt::plugin::plugin_t& {
    if (this->__lib_handler != nullptr) {
        dlclose(this->__lib_handler);
    }
    this->__lib_handler = _handler;
    return (*this);
}

auto
zpt::plugin::plugin_t::set_loader(std::function<bool(zpt::plugin& _plugin)> _loader)
  -> zpt::plugin::plugin_t& {
    this->__loader = _loader;
    return (*this);
}

auto
zpt::plugin::plugin_t::load_plugin(zpt::plugin& _other) -> zpt::plugin::plugin_t& {
    if (this->__loader != nullptr) {
        _other->__running = this->__loader(_other);
    }
    return (*this);
}

auto
zpt::plugin::plugin_t::set_unloader(std::function<bool(zpt::plugin& _plugin)> _unloader)
  -> zpt::plugin::plugin_t& {
    this->__unloader = _unloader;
    return (*this);
}

auto
zpt::plugin::plugin_t::unload_plugin(zpt::plugin& _other) -> zpt::plugin::plugin_t& {
    if (this->__unloader != nullptr) {
        _other->__running = this->__unloader(_other);
    }
    return (*this);
}

zpt::startup::engine::engine()
  : zpt::events::dispatcher<zpt::startup::engine, zpt::json, bool>{ __hazard_domain, 50000 }
  , __configuration{ zpt::globals::alloc<zpt::json>(zpt::GLOBAL_CONFIG(), zpt::json::object()) } {}

zpt::startup::engine::engine(zpt::json _args)
  : zpt::startup::engine{} {
    this->initialize(_args);
}

zpt::startup::engine::~engine() {
    if (zpt::GLOBAL_CONFIG() != -1) {
        zpt::globals::dealloc<zpt::json>(zpt::GLOBAL_CONFIG());
    }
}

auto
zpt::startup::engine::initialize(zpt::json _args) -> zpt::startup::engine& {
    zpt::startup::configuration::load(_args, this->__configuration);
    zpt::log_lvl = this->__configuration["log"]["level"]->ok()
                     ? static_cast<int>(this->__configuration["log"]["level"])
                     : 8;
    zpt::log_format = this->__configuration["log"]["format"]->ok()
                        ? static_cast<int>(this->__configuration["log"]["format"])
                        : 0;

    zlog("Starting server with PID " << zpt::log_pid, zpt::info);

    this //
      ->load({ "name", "c++_loader", "source", "*", "handles", { zpt::array, ".so" } },
             zpt::undefined)
      ->set_loader(zpt::startup::dynlib::load_plugin)
      ->set_unloader(zpt::startup::dynlib::unload_plugin);

    return (*this);
}

auto
zpt::startup::engine::trapped(zpt::json _event, bool _content) -> void {
    std::string _key = this->hash(_event);
    auto _it = this->__callbacks.find(_key);
    if (_it != this->__callbacks.end()) {
        for (auto _callback : _it->second) {
            try {
                _callback(_content);
            }
            catch (zpt::events::unregister const& _e) {
            }
        }
    }
}

auto
zpt::startup::engine::listen_to(zpt::json _event, std::function<void(bool)> _callback) -> void {
    std::string _key = this->hash(_event);
    this->__callbacks[_key].push_back(_callback);
}

auto
zpt::startup::engine::error_callback(zpt::json& _event,
                                     bool& _content,
                                     const char* _what,
                                     const char* _description,
                                     int _error) -> bool {
    return false;
}

auto
zpt::startup::engine::to_string() -> std::string {
    zpt::json _callbacks = zpt::json::object();
    for (auto [_key, _callback] : this->__callbacks) {
        _callbacks << _key << _callback.size();
    }
    zpt::json _plugins = zpt::json::object();
    for (auto [_key, _plugin] : this->__plugins) {
        _plugins << _key
                 << zpt::json{ "name",     _plugin->name(),        "source", _plugin->source(),
                               "requires", _plugin->requirements() };
    }
    return *(zpt::pretty{ zpt::json{ "callbacks", _callbacks, "plugins", _plugins } });
}

auto
zpt::startup::engine::add_thread(std::function<void()> _callback) -> zpt::startup::engine& {
    this->__workers.emplace_back(_callback);
    return (*this);
}

auto
zpt::startup::engine::load(zpt::json _plugin_options, zpt::json _plugin_config) -> zpt::plugin& {
    expect(
      _plugin_options["name"]->ok(), "missing name definition in plugin configuration", 500, 0);

    auto [_it, _inserted] = this->__plugins.emplace(_plugin_options["name"],
                                                    zpt::plugin{ _plugin_options, _plugin_config });
    zpt::plugin& _plugin = _it->second;

    if (!_inserted) {
        return _plugin;
    }

    auto _finish = [=](bool _success) mutable -> void {
        if (!_success)
            return;
        if (_plugin->is_running()) {
            throw zpt::events::unregister();
        }
        if (!this->check_requirements(_plugin)) {
            return;
        }
        this //
          ->load_plugin(_plugin)
          .add_plugin(_plugin, _plugin_options);
        throw zpt::events::unregister();
    };

    if (this->check_requirements(_plugin, _finish)) {
        try {
            _finish(true);
        }
        catch (...) {
        }
    }
    return _plugin;
}

auto
zpt::startup::engine::start() -> zpt::startup::engine& {
    this
      ->add_consumer() //
      .load();
    zlog("Going to wait on " << this->__workers.size() << " running threads", zpt::trace);
    for (size_t _idx = 0; _idx != this->__workers.size(); ++_idx) {
        this->__workers[_idx].join();
    }
    zlog("Exiting startup engine", zpt::info);
    this->shutdown();
    return (*this);
}

auto
zpt::startup::engine::unload() -> bool {
    bool _not_unloaded{ false };
    if (this->__unloaded.compare_exchange_strong(_not_unloaded, true)) {
        zlog("Unloading plugins", zpt::info);
        for (auto [_key, _plugin] : this->__plugins) {
            if (_key == "c++_loader" || _key == ".so") {
                continue;
            }
            this->unload_plugin(_plugin);
        }
        return true;
    }
    return false;
}

auto
zpt::startup::engine::exit() -> void {
    zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING()).shutdown();
    this->unload();
}

auto
zpt::startup::engine::load() -> zpt::startup::engine& {
    for (auto [_, __, _lib] : this->__configuration["load"]) {
        this->load(_lib, this->__configuration[_lib["name"]->str()]);
    }
    do {
        std::this_thread::sleep_for(std::chrono::duration<int, std::micro>{ 5 });
    } while (this->__configuration["load"]->size() != this->__plugins.size() - 2);
    zlog("All plugins loaded", zpt::info);
    return (*this);
}

auto
zpt::startup::engine::hash(zpt::json& _event) -> std::string {
    return static_cast<std::string>(_event["plugin"]) + std::string("/") +
           std::to_string(static_cast<int>(_event["stage"]));
}

auto
zpt::startup::engine::check_requirements(zpt::plugin& _plugin, std::function<void(bool)> _callback)
  -> bool {
    bool _fullfilled{ true };
    zpt::json _reqs = _plugin->requirements();
    zpt::lf::spin_lock::guard _sentry{ this->__plugin_list_lock, zpt::lf::spin_lock::shared };
    for (auto [_, _key, _loaded] : _reqs) {
        if (!static_cast<bool>(_loaded)) {
            if (this->__plugins.find(_key) == this->__plugins.end()) {
                _fullfilled = false;
                if (_callback != nullptr) {
                    this->listen({ "plugin", _key, "stage", zpt::startup::stages::RUN }, _callback);
                }
            }
            else {
                _reqs[_key] = true;
            }
        }
    }
    return _fullfilled;
}

auto
zpt::startup::engine::load_plugin(zpt::plugin& _plugin) -> zpt::startup::engine& {
    zlog("Loading plugin " << _plugin->name(), zpt::trace);
    zpt::lf::spin_lock::guard _sentry{ this->__plugin_list_lock, zpt::lf::spin_lock::shared };
    std::string& _lib = _plugin->source();
    size_t _idx{ _lib.find(".") };
    if (_idx != std::string::npos) {
        std::string _extension{ _lib.substr(_idx) };
        if (_extension.length() != 0) {
            auto _provider = this->__plugins.find(_extension);
            if (_provider != this->__plugins.end()) {
                _provider->second->load_plugin(_plugin);
            }
        }
    }
    return (*this);
}

auto
zpt::startup::engine::add_plugin(zpt::plugin& _plugin, zpt::json& _config)
  -> zpt::startup::engine& {
    zpt::lf::spin_lock::guard _sentry{ this->__plugin_list_lock, zpt::lf::spin_lock::exclusive };
    for (auto [_, __, _handled] : _config["handles"]) {
        this->__plugins.insert(std::make_pair(static_cast<std::string>(_handled), _plugin));
    }
    this->__plugins.insert(std::make_pair(_plugin->name(), _plugin));
    return (*this);
}

auto
zpt::startup::engine::unload_plugin(zpt::plugin& _plugin) -> zpt::startup::engine& {
    zlog("Unloading plugin " << _plugin->name(), zpt::trace);
    zpt::lf::spin_lock::guard _sentry{ this->__plugin_list_lock, zpt::lf::spin_lock::shared };
    std::string& _lib = _plugin->source();
    size_t _idx{ _lib.find(".") };
    if (_idx != std::string::npos) {
        std::string _extension{ _lib.substr(_idx) };
        if (_extension.length() != 0) {
            auto _provider = this->__plugins.find(_extension);
            if (_provider != this->__plugins.end()) {
                _provider->second->unload_plugin(_plugin);
            }
        }
    }
    return (*this);
}

auto
zpt::startup::dynlib::load_plugin(zpt::plugin& _plugin) -> bool {
    zpt::startup::engine& _boot = zpt::globals::get<zpt::startup::engine>(zpt::BOOT_ENGINE());
    std::string& _lib = _plugin->name();
    std::string& _lib_file = _plugin->source();

    void* _hndl = dlopen(_lib_file.data(), RTLD_NOW);
    if (_hndl == nullptr) {
        zlog(dlerror(), zpt::emergency);
        return false;
    }
    _plugin->set_handler(_hndl);

    _boot.trigger({ "plugin", _lib, "stage", zpt::startup::stages::SEARCH }, true);
    void (*_populate)(zpt::plugin&);
    _populate = (void (*)(zpt::plugin&))dlsym(_hndl, "_zpt_load_");

    bool _load{ _populate != nullptr };
    _boot.trigger({ "plugin", _lib, "stage", zpt::startup::stages::LOAD }, _load);
    if (!_load) {
        return false;
    }

    bool _config{ true };
    try {
        _populate(_plugin);
    }
    catch (zpt::failed_expectation const& _e) {
        _config = false;
    }
    _boot.trigger({ "plugin", _lib, "stage", zpt::startup::stages::CONFIGURATION }, _config);
    _boot.trigger({ "plugin", _lib, "stage", zpt::startup::stages::RUN }, true);
    return true;
}

auto
zpt::startup::dynlib::unload_plugin(zpt::plugin& _plugin) -> bool {
    if (_plugin->handler() == nullptr) {
        zlog("plugin " << _plugin->name() << " wasn't properly loaded", zpt::emergency);
        return false;
    }
    void (*_unpopulate)(zpt::plugin&);
    _unpopulate = (void (*)(zpt::plugin&))dlsym(_plugin->handler(), "_zpt_unload_");

    bool _unload{ _unpopulate != nullptr };
    if (!_unload) {
        return false;
    }

    try {
        _unpopulate(_plugin);
    }
    catch (zpt::failed_expectation const& _e) {
        return true;
    }
    return false;
}
