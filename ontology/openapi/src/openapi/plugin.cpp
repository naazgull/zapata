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

#include <iostream>
#include <zapata/startup.h>
#include <zapata/openapi.h>

extern "C" auto
_zpt_load_(zpt::plugin& _plugin) -> void {
    auto& _boot = zpt::globals::get<zpt::startup::engine>(zpt::BOOT_ENGINE());
    auto& _config = zpt::globals::get<zpt::json>(zpt::GLOBAL_CONFIG());
    zpt::globals::alloc<zpt::openapi::engine>(zpt::OPENAPI_ENGINE(), _config["limits"]);

    _boot.add_thread([]() -> void {
        zlog("Starting OpenAPI processor", zpt::info);
        auto& _boot = zpt::globals::get<zpt::startup::engine>(zpt::BOOT_ENGINE());
        auto& _config = zpt::globals::get<zpt::json>(zpt::GLOBAL_CONFIG());
        auto& _openapi = zpt::globals::get<zpt::openapi::engine>(zpt::OPENAPI_ENGINE());
        _openapi.start_threads();

        for (auto [_idx, _key, _source] : _config["openapi"]) {
            _openapi.add_source(_source);
        }
        _openapi.traverse();
        _boot.exit();
    });
}

extern "C" auto
_zpt_unload_(zpt::plugin& _plugin) -> void {
    zlog("Finishing up OpenAPI processor", zpt::info);
    zpt::globals::get<zpt::openapi::engine>(zpt::OPENAPI_ENGINE()).shutdown();
    zpt::globals::dealloc<zpt::openapi::engine>(zpt::OPENAPI_ENGINE());
}
