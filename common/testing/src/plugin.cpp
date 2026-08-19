/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright inteautomaton in the software to the public
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

#include <iostream>
#include <zapata/lua.h>
#include <zapata/runtime.h>
#include <zapata/startup.h>

namespace {
auto trim_module_name(std::string const& _to_trim) -> std::string {
    std::string _result = _to_trim;
    while (_result[0] == '.') { _result = _result.substr(1); }
    return _result;
}
} // namespace

class plugin_testing_execute_after_boot : public zpt::system_event {
  public:
    using zpt::system_event::system_event;
    ~plugin_testing_execute_after_boot() = default;

    auto operator()(zpt::events::dispatcher::ptr _dispatcher) -> zpt::events::state override {
        auto& _bridge = zpt::LUA_BRIDGE().thread_instance();
        auto _config = zpt::GLOBAL_CONFIG();
        auto _dummy_args = zpt::json::array();
        auto _failed = zpt::json::array();

        for (auto&& [_, __, _target] : _config("testing")("target")) {
            try {
                zlog(_target->string() << ": exec...", zpt::debug);
                _bridge.call(zpt::json{ "module", _target, "function", "run" }, _dummy_args);
                zlog(_target->string() << ": ok", zpt::notice);
            }
            catch (std::exception const& _e) {
                zlog(_target->string() << ": fail - " << _e.what(), zpt::warning);
                _failed << _target;
            }
            if (_dispatcher->is_in_shutdown()) { break; }
        }

        zpt::SYSTEM_EVENTS_RESOLVER() //
          ->remove<plugin_testing_execute_after_boot>(zpt::system_event_type::FINISHED_BOOT);

        if (_failed->size() != 0) {
            zlog("Failed tests: " << zpt::pretty{ _failed }, zpt::warning);
            abort();
        }

        zlog("Shutting down", zpt::notice);
        zpt::runtime::shutdown();

        return zpt::events::finish;
    }
};

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _config = _plugin.config();

    if (!_config("target")->ok()) {
        auto _root = _config("root")->ok() ? _config("root")->string() : ".";

        std::vector<std::string> _files;
        zpt::glob(_root, _files, "(.*)\\.lua", 100);

        _config["target"] = zpt::json::array();
        for (auto&& _target : _files) { _config["target"] << _target; }
    }
    else if (!_config("target")->is_array()) {
        auto _target = static_cast<std::string>(_config("target"));
        _config["target"] = { zpt::array, _target };
    }

    auto _targets = zpt::json::array();
    for (auto&& [_, __, _target] : _config("target")) {
        if (_target->string().find(".lua") != std::string::npos) { _targets << _target; }
        else {
            std::vector<std::string> _files;
            zpt::glob(_target->string(), _files, "(.*)\\.lua", 100);
            for (auto&& _t : _files) { _targets << _t; }
        }
    }

    _config["target"] = zpt::json::array();
    auto& _bridge = zpt::LUA_BRIDGE();
    for (auto&& [_, __, _target] : _targets) {
        auto _name = zpt::r_replace_multiple(_target->string(), { "/", ".lua" }, { ".", "" });
        _name = ::trim_module_name(_name);

        zpt::json _module = { "file", _target, "module", _name };
        _bridge.add_module(_target->string(), _module);
        _config["target"] << _name;
    }

    zpt::SYSTEM_EVENTS_RESOLVER() //
      ->add<plugin_testing_execute_after_boot>(zpt::system_event_type::FINISHED_BOOT);

    zlog("Loaded testing plugin", zpt::info);
}

extern "C" auto _zpt_unload_(zpt::plugin&) -> void { zlog("Unloaded testing plugin", zpt::info); }
