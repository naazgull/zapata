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

#include <hiredis/hiredis.h>
#include <mutex>
#include <ossp/uuid++.hh>
#include <string>
#include <zapata/events.h>
#include <zapata/json.h>
#include <zapata/redis/convert_redis.h>

namespace zpt {

namespace redis {

class ZList : public zpt::Connector {
  public:
    ZList(zpt::json _options, std::string const& _conf_path);
    virtual ~ZList();

    virtual auto name() -> std::string;
    virtual auto options() -> zpt::json;
    virtual auto events(zpt::ev::emitter _emitter) -> void;
    virtual auto events() -> zpt::ev::emitter;

    virtual auto connect() -> void;
    virtual auto reconnect() -> void;

    virtual auto set(std::string const& _key, zpt::timestamp_t _score, zpt::json _data) -> void;
    virtual auto reset(std::string const& _key, zpt::timestamp_t _increment, zpt::json _data)
      -> void;
    virtual auto del(std::string const& _key, zpt::json _data) -> void;
    virtual auto del(std::string const& _key, std::string const& _data) -> void;
    virtual auto del(std::string const& _key, zpt::timestamp_t _min) -> void;
    virtual auto del(std::string const& _key, zpt::timestamp_t _min, zpt::timestamp_t _max) -> void;
    virtual auto rangebypos(std::string const& _key, long int _min, long int _max) -> zpt::json;
    virtual auto range(std::string const& _key,
                       zpt::timestamp_t _min,
                       zpt::timestamp_t _max,
                       int _direction = 1,
                       size_t _offset = 0,
                       size_t _limit = 0) -> zpt::json;
    virtual auto getall(std::string const& _key) -> zpt::json;
    virtual auto find(std::string const& _key, std::string const& _regexp) -> zpt::json;

  private:
    zpt::json __options;
    std::mutex __mtx;
    redisContext* __conn;
    std::string __host;
    uint __port;
    zpt::ev::emitter __events;
};
} // namespace redis
} // namespace zpt
