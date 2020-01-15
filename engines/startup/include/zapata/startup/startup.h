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

namespace zpt {

constexpr int BOOT_ENGINE{ 0 };

class globals {
  public:
    template<typename T, int N, typename... Args>
    static auto get(Args... _args) -> T&;

  private:
    static inline std::map<int, void*> __variables{};
};

namespace startup {
enum stages { SEARCH = 0, LOAD = 1, CONFIGURATION = 2, UNLOAD = 3 };

class engine : public zpt::events::dispatcher<zpt::startup::engine, zpt::json, bool> {
  public:
    engine();
    engine(zpt::json _args);
    virtual ~engine() = default;

    auto initialize(zpt::json _args) -> zpt::startup::engine&;

    auto trapped(zpt::json _event, bool _content) -> void;
    auto listen_to(zpt::json _event, std::function<void(bool)> _callback) -> void;

    auto start() -> zpt::startup::engine&;
    auto load(std::string const& _lib) -> zpt::startup::engine&;
    auto unload(std::string const& _lib) -> zpt::startup::engine&;

    auto config() -> zpt::json&;

  private:
    zpt::json __configuration;
    std::map<std::string, std::function<void(zpt::json)>> __callbacks;

    auto hash(zpt::json& _event) -> std::string;
    auto load() -> zpt::startup::engine&;
    auto load_script(std::string const& _lib) -> zpt::startup::engine&;
    auto load_lib(std::string const& _lib, void* _hndl) -> zpt::startup::engine&;
};
} // namespace startup
} // namespace zpt

template<typename T, int N, typename... Args>
auto
zpt::globals::get(Args... _args) -> T& {
    auto _found = zpt::globals::__variables.find(N);
    if (_found != zpt::globals::__variables.end()) {
        return *static_cast<T*>(_found->second);
    }

    T* _new = new T(_args...);
    zpt::globals::__variables.insert(std::make_pair(N, static_cast<void*>(_new)));
    return *_new;
    ;
}
