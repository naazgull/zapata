#include <zapata/config.h>
#include <zapata/uuid.h>

auto zpt::config::load(zpt::json _parameters, zpt::json& _output) -> void {
    for (auto&& [_, __, _conf_file] : _parameters("--config")) {
        try {
            zpt::conf::file(static_cast<std::string>(_conf_file), _output, _output);
        }
        catch (zpt::failed_expectation const& _e) {
            zlog("Found " << _e, zpt::emergency);
        }
    }
    for (auto&& [_, __, _conf_dir] : _parameters("--conf-dir")) {
        try {
            zpt::conf::dirs(static_cast<std::string>(_conf_dir), _output);
        }
        catch (zpt::failed_expectation const& _e) {
            zlog("Found " << _e, zpt::emergency);
        }
    }
    if (_output.size() == 0) { _output += zpt::config::load_defaults(); }

    zpt::conf::env(_output);

    for (auto&& [_, __, _param] : _parameters("--")) {
        auto _pair = _param->string();
        auto _idx = _pair.find(":");
        if (_idx == std::string::npos) { continue; }

        auto _path = _pair.substr(0, _idx);
        zpt::json _to_set;
        auto _value = _pair.substr(_idx + 1);
        try {
            _to_set = zpt::json::parse_json_str(_value);
        }
        catch (...) {
            _to_set = _value;
        }
        auto _already_set = _output->get_path(_path);
        if (_already_set->ok() && _already_set->is_array()) {
            if (_to_set->is_array()) { _already_set = _to_set; }
            else { _already_set << _to_set; }
        }
        else { _output->set_path(_path, _to_set); }
    }
}

auto zpt::config::load_defaults() -> zpt::json {
    auto _defaults = R"(
         {
             "identity": {},
             "log": { "level": 6, "format": 1 },
             "load": [
                 { "name": "builtin:http" },
                 { "name": "builtin:tcp" },
                 { "name": "builtin:upnp" },
                 { "name": "builtin:ws" },
                 { "name": "builtin:rest" }
             ],
             "resources": {
                 "limits": {
                     "max_heap_allocation": 0
                 }
             },
             "dispatcher": {
                 "limits": {
                     "max_workers": 2
                 }
             },
             "upnp": { "bind": "239.192.1.2", "port": 7979 },
             "transport": {
                 "default": "tcp",
                 "limits": {
                     "max_workers": 8
                 }
             }
         })"_JSON;
    _defaults["identity"]["id"] = zpt::uuid{}.to_string();
    _defaults["identity"]["name"] = zpt::generate::r_key(12);
    return _defaults;
}

auto zpt::GLOBAL_CONFIG() -> zpt::json& {
    static zpt::json _global = zpt::json::object();
    return _global;
}
