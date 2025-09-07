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
#include <zapata/transport.h>

extern "C" auto _zpt_load_(zpt::plugin& _plugin) -> void {
    auto _global_config = zpt::GLOBAL_CONFIG();
    auto& _config = _plugin.config();
    expect(_config->type() == zpt::JSObject, "Configuration 'identity' must be defined");

    std::string _id = _config("id")->ok() ? _config("id")->string() : zpt::generate::r_uuid();
    _config["_id"] = _id;
    _config["name"] = _config("name")->ok() ? _config("name") : _config["_id"];
    _config->object()->pop("id");

    for (auto const& [_protocol, _] : zpt::TRANSPORT_LAYER()) {
        _config["protocols"]["registered"][_protocol] = _global_config(_protocol);
    }
    _config["protocols"]["default"] = _global_config("transport")("default");
    zlog("Minion identified by " << _config("_id")->string(), zpt::info);
}

extern "C" auto _zpt_unload_(zpt::plugin&) {}
