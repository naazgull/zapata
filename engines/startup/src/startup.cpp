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

auto zpt::BOOT() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}
auto zpt::GLOBAL_CONFIG() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::plugin::plugin(zpt::json _options, zpt::json _config)
  : __config{ _config } {
    expect(_options["name"]->ok(), "missing name definition in plugin configuration");
    expect(_options["source"]->ok(),
           std::string("missing source definition in plugin configuration of ") +
             static_cast<std::string>(_options["name"]));
    this->__name.assign(static_cast<std::string>(_options["name"]));
    this->__source.assign(static_cast<std::string>(_options["source"]));

    this->__lib_handler = dlopen(this->__source.data(), RTLD_NOW);
    if (this->__lib_handler != nullptr) {
        void (*_populate)(zpt::plugin&);
        _populate = (void (*)(zpt::plugin&))dlsym(this->__lib_handler, "_zpt_load_");
        if (_populate != nullptr) {
            // try {
            _populate((*this));
            return;
            // }
            // catch (zpt::failed_expectation const& _e) {
            //     zlog(_e, zpt::emergency);
            // }
        }
        else { zlog(dlerror(), zpt::emergency); }
    }
    else { zlog(dlerror(), zpt::emergency); }
}

zpt::plugin::~plugin() {
    this->__shutdown->store(true);

    if (this->__lib_handler == nullptr) {
        zlog("plugin " << this->__name << " wasn't properly loaded", zpt::warning);
        return;
    }

    void (*_unpopulate)(zpt::plugin&);
    _unpopulate = (void (*)(zpt::plugin&))dlsym(this->__lib_handler, "_zpt_unload_");
    if (_unpopulate != nullptr) {
        // try {
        _unpopulate((*this));
        // }
        // catch (zpt::failed_expectation const& _e) {
        //     zlog(_e, zpt::emergency);
        // }
    }
    else { zlog(dlerror(), zpt::emergency); }

    for (auto& thr : this->__threads) { thr.join(); }

    dlclose(this->__lib_handler);
    this->__lib_handler = nullptr;
}

auto zpt::plugin::name() -> std::string& { return this->__name; }

auto zpt::plugin::source() -> std::string& { return this->__source; }

auto zpt::plugin::config() -> zpt::json& { return this->__config; }

auto zpt::plugin::is_shutdown_ongoing() -> bool { return this->__shutdown->load(); }

auto zpt::plugin::plugin::add_thread(std::function<void()> _callback) -> plugin& {
    this->__threads.emplace_back(_callback);
    return (*this);
}

zpt::startup::boot::boot(zpt::json _config)
  : __configuration{ _config } {
    zlog("Starting server with PID " << zpt::log_pid, zpt::info);
}

zpt::startup::boot::~boot() {}

auto zpt::startup::boot::to_string() -> std::string {
    auto _plugins = zpt::json::object();
    for (auto& [_key, _plugin] : this->__plugins) {
        _plugins << _key << zpt::json{ "name", _plugin->name(), "source", _plugin->source() };
    }
    return *(zpt::pretty{ zpt::json{ "plugins", _plugins } });
}

auto zpt::startup::boot::load() -> zpt::startup::boot& {
    auto _to_load = zpt::json::object();
    for (auto [_idx, __, _lib] : this->__configuration["load"]) {
        _to_load << _lib["name"]->string() << true;
    }

    while (_to_load->size() != 0) {
        for (auto [_idx, __, _lib] : this->__configuration["load"]) {
            auto _name = _lib["name"]->string();
            if (_lib("requires")->ok() && _lib["requires"]->is_array()) {
                for (auto [___, ____, _required] : _lib("requires")) {
                    if (this->__plugins.find(_required->string()) == this->__plugins.end()) {
                        continue;
                    }
                }
            }
            this->load(_lib, this->__configuration[_name]);
            _to_load->object()->pop(_name);
        }
    }
    zlog("All plugins loaded", zpt::info);
    return (*this);
}

auto zpt::startup::boot::load(zpt::json _plugin_options, zpt::json _plugin_config) -> zpt::plugin& {
    expect(_plugin_options["name"]->ok(), "missing name definition in plugin configuration");

    auto [_it, _inserted] = this->__plugins.emplace(
      _plugin_options["name"]->string(),
      std::unique_ptr<zpt::plugin>{ new zpt::plugin{ _plugin_options, _plugin_config } });
    auto& _plugin = _it->second;
    return *_plugin;
}

auto zpt::startup::boot::hash(zpt::json& _event) -> std::string {
    return static_cast<std::string>(_event["plugin"]) + std::string("/") +
           std::to_string(static_cast<int>(_event["step"]));
}
