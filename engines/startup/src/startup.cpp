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
#include <zapata/startup/startup.h>
#include <zapata/transport.h>

namespace {
zpt::json __builtins = R"({
        "builtin:testing": { "name": "builtin:testing", "source": "libzapata-common-testing-plugin.so",
            "requires" : [ "builtin:lua", "builtin:rest", "builtin:transport" ] },
        "builtin:lua": { "name": "builtin:lua", "source": "libzapata-bridge-lua-plugin.so",
            "requires" : [ "builtin:rest", "builtin:transport" ] },
        "builtin:prolog": { "name": "builtin:prolog", "source": "libzapata-bridge-prolog-plugin.so",
            "requires" : [ "builtin:rest", "builtin:transport" ] },
        "builtin:rest": { "name": "builtin:rest", "source": "libzapata-engine-rest-plugin.so",
            "requires" : [ "builtin:transport", "builtin:identity" ] },
        "builtin:transport": { "name": "builtin:transport",
            "source": "libzapata-engine-transport-plugin.so", "requires" : [] },
        "builtin:http": { "name": "builtin:http", "source": "libzapata-net-http-plugin.so",
            "requires" : [ "builtin:transport" ],
            "needed_for" : [ "builtin:rest", "builtin:identity" ] },
        "builtin:local": { "name": "builtin:local", "source": "libzapata-net-local-plugin.so",
            "requires" : [ "builtin:transport" ],
            "needed_for" : [ "builtin:rest", "builtin:identity" ] },
        "builtin:pipe": { "name": "builtin:pipe", "source": "libzapata-net-pipe-plugin.so",
            "requires" : [ "builtin:transport" ],
            "needed_for" : [ "builtin:rest", "builtin:identity" ] },
        "builtin:tcp": { "name": "builtin:tcp", "source": "libzapata-net-tcp-plugin.so",
            "requires" : [ "builtin:transport" ],
            "needed_for" : [ "builtin:rest", "builtin:identity" ] },
        "builtin:upnp": { "name": "builtin:upnp", "source": "libzapata-net-upnp-plugin.so",
            "requires" : [ "builtin:transport" ],
            "needed_for" : [ "builtin:rest", "builtin:identity" ] },
        "builtin:ws": { "name": "builtin:ws", "source": "libzapata-net-websocket-plugin.so",
            "requires" : [ "builtin:transport" ],
            "needed_for" : [ "builtin:rest", "builtin:identity" ] },
        "builtin:self": { "name": "builtin:self", "source": "libzapata-net-self-plugin.so",
            "requires" : [ "builtin:transport" ],
            "needed_for" : [ "builtin:rest", "builtin:identity" ] },
        "builtin:identity": { "name": "builtin:identity",
            "source": "libzapata-net-identity-plugin.so", "requires" : [ "builtin:transport" ] }
   })"_JSON;
}

zpt::plugin::plugin(zpt::json _options, zpt::json _config)
  : __config{ _config } {
    expect(_options("name")->ok(), "missing name definition in plugin configuration");
    expect(_options("source")->ok(),
           std::string("missing source definition in plugin configuration of ") +
             static_cast<std::string>(_options("name")));
    this->__name.assign(static_cast<std::string>(_options("name")));
    this->__source.assign(static_cast<std::string>(_options("source")));

    this->__lib_handler = dlopen(this->__source.data(), RTLD_NOW);
    if (this->__lib_handler != nullptr) {
        void (*_populate)(zpt::plugin&);
        _populate = (void (*)(zpt::plugin&))dlsym(this->__lib_handler, "_zpt_load_");
        if (_populate != nullptr) {
            _populate((*this));
            return;
        }
        else { zlog(dlerror(), zpt::emergency); }
    }
    else { zlog(dlerror(), zpt::emergency); }
}

zpt::plugin::~plugin() {
    this->__state->store(PLUGIN_STATE_IN_SHUTDOWN);
    if (this->__lib_handler == nullptr) {
        zlog("plugin " << this->__name << " wasn't properly loaded", zpt::warning);
        return;
    }

    void (*_unpopulate)(zpt::plugin&);
    _unpopulate = (void (*)(zpt::plugin&))dlsym(this->__lib_handler, "_zpt_unload_");
    if (_unpopulate != nullptr) { _unpopulate((*this)); }
    else { zlog(dlerror(), zpt::emergency); }

    for (auto& thr : this->__threads) { thr.join(); }

    dlclose(this->__lib_handler);
    this->__lib_handler = nullptr;
    this->__state->store(PLUGIN_STATE_UNLOADED);
}

auto zpt::plugin::name() -> std::string& { return this->__name; }

auto zpt::plugin::source() -> std::string& { return this->__source; }

auto zpt::plugin::config() -> zpt::json& { return this->__config; }

auto zpt::plugin::is_shutdown_ongoing() -> bool {
    return this->__state->load() == PLUGIN_STATE_IN_SHUTDOWN;
}

auto zpt::plugin::is_loaded() -> bool { return this->__state->load() == PLUGIN_STATE_LOADED; }

auto zpt::plugin::is_unloaded() -> bool { return this->__state->load() == PLUGIN_STATE_UNLOADED; }

auto zpt::plugin::plugin::add_thread(std::function<void()> _callback) -> plugin& {
    this->__threads.emplace_back(_callback);
    return (*this);
}

zpt::startup::boot::boot(zpt::json _config)
  : __configuration{ _config } {
    this->__configuration["load"] << __builtins("builtin:self");
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
    this->resolve_builtin_dependencies();

    auto _to_load = zpt::json::object();
    for (auto&& [_idx, __, _lib] : this->__configuration("load")) {
        auto _name = _lib("name")->string();
        _to_load << _name << zpt::json::object();
    }

    bool _no_change{ false };
    while (_to_load->size() != 0 && !_no_change) {
        _no_change = true;

        for (auto&& [_idx, __, _lib] : this->__configuration("load")) {
            auto _name = _lib("name")->string();
            if (this->__plugins.find(_name) != this->__plugins.end()) { continue; }

            expect(!_lib("requires")->ok() || _lib("requires")->is_array(),
                   "Configuration error: library 'requires' field must be an array");

            for (auto&& [___, ____, _required] : _lib("requires")) {
                if (this->__plugins.find(_required->string()) == this->__plugins.end()) {
                    _to_load[_name] << _required->string() << false;
                }
                else { _to_load[_name]->object()->pop(_required->string()); }
            }

            if (_to_load(_name)->size() != 0) { continue; }

            auto _key = zpt::r_replace(_name, "builtin:", "");
            if (!this->__configuration(_key)->ok()) {
                this->__configuration[_key] = zpt::json::object();
            }

            _no_change = false;
            this->load(_lib, this->__configuration(_key));
            this->__load_order.push_back(_name);
            _to_load->object()->pop(_name);
        }
    }

    expect(!_no_change, "Configuration error: unmet dependencies " << _to_load);

    return (*this);
}

auto zpt::startup::boot::unload() -> zpt::startup::boot& {
    for (auto it = this->__load_order.rbegin(); it != this->__load_order.rend(); ++it) {
        this->__plugins.erase(*it);
    }
    this->__plugins.clear();
    return (*this);
}

auto zpt::startup::boot::resolve_builtin_dependencies() -> void {
    zpt::json _already_added = zpt::json::object();

    for (size_t _idx = 0; _idx != this->__configuration("load")->size(); ++_idx) {
        auto _lib = this->__configuration["load"][_idx];
        auto _name = _lib("name")->string();
        if (::__builtins(_name)->ok()) {
            _lib << "source" << ::__builtins(_name)("source") << "requires"
                 << ::__builtins(_name)("requires");
            if (::__builtins(_name)("needed_for")->ok()) {
                for (auto const& [_, __, _needing] : ::__builtins(_name)("needed_for")) {
                    ::__builtins[_needing->string()]["requires"] << _name;
                }
            }
        }
        for (auto const& [_, __, _dependency] : _lib("requires")) {
            if (!_already_added(_dependency->string())->ok()) {
                this->__configuration["load"] << zpt::json{ "name", _dependency };
                _already_added << _dependency->string() << true;
            }
        }
    }
}

auto zpt::startup::boot::load(zpt::json _plugin_options, zpt::json _plugin_config) -> zpt::plugin& {
    expect(_plugin_options("name")->ok(), "missing name definition in plugin configuration");
    expect(this->__plugins.find(_plugin_options("name")->string()) == this->__plugins.end(),
           "duplicate plugin name in plugin configuration list");

    auto [_it, _inserted] = this->__plugins.emplace(
      _plugin_options("name")->string(),
      std::unique_ptr<zpt::plugin>{ new zpt::plugin{ _plugin_options, _plugin_config } });
    auto& _plugin = _it->second;
    return *_plugin;
}

auto zpt::startup::boot::hash(zpt::json& _event) -> std::string {
    return static_cast<std::string>(_event("plugin")) + std::string("/") +
           std::to_string(static_cast<int>(_event("step")));
}

auto zpt::get_default_uri() -> std::string {
    auto _scheme = zpt::IDENTITY()("protocols")("default")->string();
    if (!zpt::IDENTITY()("protocols")("registered")(_scheme)->ok()) { return ""; }

    auto _my_host = zpt::IDENTITY()("protocols")("registered")(_scheme)("address")->string();
    auto _my_port = zpt::IDENTITY()("protocols")("registered")(_scheme)("port")->integer();
    return std::format("{}://{}:{}", _scheme, _my_host, _my_port);
}

auto zpt::BOOT(zpt::json _config) -> zpt::startup::boot& {
    static zpt::startup::boot _global{ _config };
    return _global;
}

auto zpt::GLOBAL_CONFIG() -> zpt::json {
    static zpt::json _global = zpt::json::object();
    return _global;
}

auto zpt::IDENTITY() -> zpt::json const& {
    static zpt::json _self = zpt::GLOBAL_CONFIG()("identity");
    return _self;
}
