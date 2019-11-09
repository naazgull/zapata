/*
The MIT License (MIT)

Copyright (c) 2017 n@zgul <n@zgul.me>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
    ZList(zpt::json _options, std::string _conf_path);
    virtual ~ZList();

    virtual auto name() -> std::string;
    virtual auto options() -> zpt::json;
    virtual auto events(zpt::ev::emitter _emitter) -> void;
    virtual auto events() -> zpt::ev::emitter;

    virtual auto connect() -> void;
    virtual auto reconnect() -> void;

    virtual auto set(std::string _key, zpt::timestamp_t _score, zpt::json _data) -> void;
    virtual auto reset(std::string _key, zpt::timestamp_t _increment, zpt::json _data) -> void;
    virtual auto del(std::string _key, zpt::json _data) -> void;
    virtual auto del(std::string _key, std::string _data) -> void;
    virtual auto del(std::string _key, zpt::timestamp_t _min) -> void;
    virtual auto del(std::string _key, zpt::timestamp_t _min, zpt::timestamp_t _max) -> void;
    virtual auto rangebypos(std::string _key, long int _min, long int _max) -> zpt::json;
    virtual auto range(std::string _key,
                       zpt::timestamp_t _min,
                       zpt::timestamp_t _max,
                       int _direction = 1,
                       size_t _offset = 0,
                       size_t _limit = 0) -> zpt::json;
    virtual auto getall(std::string _key) -> zpt::json;
    virtual auto find(std::string _key, std::string _regexp) -> zpt::json;

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
