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

static zpt::json __bridges = { "py",   "zapata-bridge-python", "pl",   "zapata-bridge-prolog",
                               "lisp", "zapata-bridge-lisp",   "fasb", "zapata-bridge-lisp",
                               "lua",  "zapata-bridge-lua" };

zpt::startup::engine::engine()
  : zpt::events::dispatcher<zpt::startup::engine, zpt::json, bool>{ 2, 8, 1 } {}

zpt::startup::engine::engine(zpt::json _args)
  : zpt::events::dispatcher<zpt::startup::engine, zpt::json, bool>{ 2, 8, 1 } {
    this->initialize(_args);
}

auto
zpt::startup::engine::initialize(zpt::json _args) -> zpt::startup::engine& {
    this->__configuration = zpt::startup::configuration::load(_args);
    return (*this);
}

auto
zpt::startup::engine::trapped(zpt::json _event, bool _content) -> void {
    auto _it = this->__callbacks.find(this->hash(_event));
    if (_it != this->__callbacks.end()) {
        auto [_, _callback] = *_it;
        _callback(_content);
    }
}

auto
zpt::startup::engine::listen_to(zpt::json _event, std::function<void(bool)> _callback) -> void {
    this->__callbacks.insert(std::make_pair(this->hash(_event), _callback));
}

auto
zpt::startup::engine::start() -> zpt::startup::engine& {
    this
      ->add_consumer() //
      .load();
    return (*this);
}

auto
zpt::startup::engine::load(std::string const& _lib) -> zpt::startup::engine& {
    std::string _lib_file{ "lib" };
    _lib_file.append(_lib);
    _lib_file.append(".so");

    if (_lib_file.length() > 6) {
        void* _hndl = dlopen(_lib_file.data(), RTLD_NOW);
        if (_hndl == nullptr) {
            return this->load_script(_lib);
        }
        else {
            return this->load_lib(_lib, _hndl);
        }
    }
    return (*this);
}

auto
zpt::startup::engine::unload(std::string const& _lib) -> zpt::startup::engine& {
    return (*this);
}

auto
zpt::startup::engine::config() -> zpt::json& {
    return this->__configuration;
}

auto
zpt::startup::engine::hash(zpt::json& _event) -> std::string {
    return static_cast<std::string>(_event["lib"]) + std::string("/") +
           std::to_string(static_cast<int>(_event["stage"]));
}

auto
zpt::startup::engine::load() -> zpt::startup::engine& {
    for (auto [_, __, _lib] : this->__configuration["load"]) {
        this->load(_lib);
    }
    return (*this);
}

auto
zpt::startup::engine::load_script(std::string const& _lib) -> zpt::startup::engine& {
    size_t _idx{ _lib.find(".") };
    std::string _extension{ _lib.substr(_idx + 1) };

    bool _search{ _idx != std::string::npos && __bridges[_extension]->ok() };
    this->trigger({ "lib", _lib, "stage", stages::SEARCH }, _search);
    if (!_search) {
        return (*this);
    }

    this->listen({ "lib", __bridges[_extension], "stage", stages::LOAD },
                 [=](bool _result) -> void {
                     if (_result) {
                     }
                     this->trigger({ "lib", _lib, "stage", stages::LOAD }, _result);
                 });
    return (*this);
}

auto
zpt::startup::engine::load_lib(std::string const& _lib, void* _hndl) -> zpt::startup::engine& {
    this->trigger({ "lib", _lib, "stage", stages::SEARCH }, true);
    void (*_populate)();
    _populate = (void (*)())dlsym(_hndl, "_zpt_load_");

    bool _load{ _populate != nullptr };
    this->trigger({ "lib", _lib, "stage", stages::LOAD }, _load);
    if (!_load) {
        return (*this);
    }

    bool _config{ true };
    try {
        _populate();
    }
    catch (zpt::failed_expectation& _e) {
        _config = false;
    }
    this->trigger({ "lib", _lib, "stage", stages::CONFIGURATION }, _config);

    return (*this);
}
