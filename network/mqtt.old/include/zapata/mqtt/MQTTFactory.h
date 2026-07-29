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

#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unistd.h>
#include <zapata/mqtt/MQTT.h>

namespace zpt {

class MQTTFactory : public zpt::ChannelFactory {
  public:
    MQTTFactory();
    virtual ~MQTTFactory();
    virtual auto produce(zpt::json _options) -> zpt::socket;
    virtual auto is_reusable(std::string const& _type) -> bool;
    virtual auto clean(zpt::socket _socket) -> bool;

    static auto on_connect(zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void;
    static auto on_disconnect(zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void;
    static auto on_message(zpt::mqtt::data _data, zpt::mqtt::broker _mqtt) -> void;

  private:
    std::map<std::string, zpt::socket> __channels;
};
} // namespace zpt
