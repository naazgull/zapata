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

#include <zapata/base.h>
#include <zapata/json.h>
#include <zapata/events.h>
#include <zapata/startup/configuration.h>

#include <typeinfo>

namespace zpt {

inline size_t BOOT_ENGINE{ 0 };
inline size_t GLOBAL_CONFIG{ 0 };

class globals {
  public:
    template<typename T, typename... Args>
    static auto alloc(size_t& _variable, Args... _args) -> T&;
    template<typename T>
    static auto get(size_t _variable) -> T&;
    template<typename T, typename... Args>
    static auto dealloc(size_t _variable) -> void;

  private:
    static inline std::map<size_t, std::vector<void*>> __variables{};
    static inline zpt::lf::spin_lock __variables_lock;
};

class plugin {
  private:
    class plugin_t {
      public:
        plugin_t() = default;
        plugin_t(zpt::json _config);
        virtual ~plugin_t() = default;

        auto initialize(zpt::json _config) -> zpt::plugin::plugin_t&;
        auto name() -> std::string&;
        auto source() -> std::string&;
        auto requirements() -> zpt::json&;
        auto is_running() -> bool;

        auto set_loader(std::function<bool(zpt::plugin& _plugin)> _loader)
          -> zpt::plugin::plugin_t&;
        auto load_plugin(zpt::plugin& _other) -> zpt::plugin::plugin_t&;

      private:
        std::string __name{ "" };
        std::string __source{ "" };
        zpt::json __requirements{ zpt::json::object() };
        std::function<bool(zpt::plugin& _plugin)> __loader;
        bool __running{ false };
    };

  public:
    using reference = plugin_t*;

    plugin() = default;
    plugin(zpt::json _config);
    plugin(zpt::plugin const& _rhs);
    plugin(zpt::plugin&& _rhs);
    virtual ~plugin() = default;

    auto operator=(zpt::plugin const& _rhs) -> zpt::plugin&;
    auto operator=(zpt::plugin&& _rhs) -> zpt::plugin&;
    auto operator-> () -> reference;

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
enum stages { SEARCH = 0, LOAD = 1, CONFIGURATION = 2, RUN = 3, UNLOAD = 4 };

class engine : public zpt::events::dispatcher<zpt::startup::engine, zpt::json, bool> {
  public:
    engine();
    engine(zpt::json _args);
    virtual ~engine() = default;

    auto initialize(zpt::json _args) -> zpt::startup::engine&;

    auto trapped(zpt::json _event, bool _content) -> void;
    auto listen_to(zpt::json _event, std::function<void(bool)> _callback) -> void;

    auto to_string() -> std::string;

    auto load(zpt::json _plugin_config) -> zpt::plugin&;
    auto start() -> zpt::startup::engine&;

    friend std::ostream& operator<<(std::ostream& _out, zpt::startup::engine& _in) {
        _out << _in.to_string() << std::flush;
        return _out;
    }

  private:
    zpt::json __configuration;
    std::map<std::string, std::vector<std::function<void(bool)>>> __callbacks;
    std::map<std::string, zpt::plugin> __plugins;
    zpt::lf::spin_lock __plugin_list_lock;

    auto load() -> zpt::startup::engine&;
    auto hash(zpt::json& _event) -> std::string;
    auto check_requirements(zpt::plugin& _plugin, std::function<void(bool)> _callback = nullptr)
      -> bool;
    auto load_plugin(zpt::plugin& _plugin) -> zpt::startup::engine&;
    auto add_plugin(zpt::plugin& _plugin, zpt::json& _config) -> zpt::startup::engine&;
};

namespace dynlib {
auto
load_plugin(zpt::plugin& _plugin) -> bool;
} // namespace synlib

} // namespace startup
} // namespace zpt

template<typename T, typename... Args>
auto
zpt::globals::alloc(size_t& _variable, Args... _args) -> T& {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock, false };
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];

    T* _new = new T(_args...);
    _allocated.push_back(static_cast<void*>(_new));
    _variable = _allocated.size() - 1;
    return *_new;
}

template<typename T>
auto
zpt::globals::get(size_t _variable) -> T& {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock, true };
    auto _found = zpt::globals::__variables.find(typeid(T).hash_code());
    expect(_found != zpt::globals::__variables.end() && _found->second.size() > _variable,
           "no such global variable",
           500,
           0);

    return *static_cast<T*>(_found->second[_variable]);
}

template<typename T, typename... Args>
auto
zpt::globals::dealloc(size_t _variable) -> void {
    zpt::lf::spin_lock::guard _sentry{ zpt::globals::__variables_lock, false };
    auto& _allocated = zpt::globals::__variables[typeid(T).hash_code()];
    _allocated.erase(_allocated.begin() + _variable);
}
