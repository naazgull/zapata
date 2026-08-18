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

#include <zapata/startup/configuration.h>
#include <zapata/uuid.h>

auto zpt::startup::configuration::load(zpt::json _parameters, zpt::json& _output) -> void {
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
    if (_output.size() == 0) { _output += zpt::startup::configuration::load_defaults(); }

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

auto zpt::startup::configuration::load_defaults() -> zpt::json {
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
