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

#include <zapata/redis/ZList.h>

zpt::redis::ZList::ZList(zpt::json _options, std::string const& _conf_path)
  : __options(_options)
  , __conn(nullptr) {
    try {
        std::string _bind((std::string)_options->get_path(_conf_path)["bind"]);
        std::string _address(_bind.substr(0, _bind.find(":")));
        uint _port = std::stoi(_bind.substr(_bind.find(":") + 1));
        this->connection(_options->get_path(_conf_path) +
                         zpt::json{ "host", _address, "port", _port });
    }
    catch (std::exceptionconst& _e) {
        expect(false, std::string("could not connect to Redis server: ") + _e.what(), 500, 0);
    }
}

zpt::redis::ZList::~ZList() {
    if (this->__conn != nullptr) { redisFree(this->__conn); }
}

auto
zpt::redis::ZList::name() -> std::string {
    return std::string("redis://") + ((std::string)this->connection()["bind"]) + std::string("/") +
           ((std::string)this->connection()["db"]);
}

auto
zpt::redis::ZList::options() -> zpt::json {
    return this->__options;
}

auto
zpt::redis::ZList::events(zpt::ev::emitter _emitter) -> void {
    this->__events = _emitter;
}

auto
zpt::redis::ZList::events() -> zpt::ev::emitter {
    return this->__events;
}

auto
zpt::redis::ZList::connect() -> void {
    this->__host.assign(std::string(this->connection()["host"]).data());
    this->__port = int(this->connection()["port"]);
    expect((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr,
           std::string("connection to Redis at ") + this->name() +
             std::string(" has not been established."),
           500,
           0);
    zpt::Connector::connect();
};

auto
zpt::redis::ZList::reconnect() -> void {
    std::lock_guard<std::mutex> _lock(this->__mtx);
    expect(this->__conn != nullptr,
           std::string("connection to Redis at ") + this->name() +
             std::string(" has not been established."),
           500,
           0);
    redisFree(this->__conn);
    expect((this->__conn = redisConnect(this->__host.data(), this->__port)) != nullptr,
           std::string("connection to Redis at ") + this->name() +
             std::string(" has not been established."),
           500,
           0);
    zpt::Connector::reconnect();
}

auto
zpt::redis::ZList::set(std::string const& _key, zpt::timestamp_t _score, zpt::json _data) -> void {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    expect(_data->ok(), "'_data' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    bool _success = true;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        redisAppendCommand(
          this->__conn, "ZADD %s %lld %s", _key.data(), _score, ((std::string)_data).data());
        _success = (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK);
    }
    freeReplyObject(_reply);
    expect(_success, "something whent wrong while accessing Redis", 500, 0);
}

auto
zpt::redis::ZList::reset(std::string const& _key, zpt::timestamp_t _increment, zpt::json _data)
  -> void {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    expect(_data->ok(), "'_data' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    bool _success = true;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        redisAppendCommand(
          this->__conn, "ZINCRBY %s %lld %s", _key.data(), _increment, ((std::string)_data).data());
        _success = (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK);
    }
    freeReplyObject(_reply);
    expect(_success, "something whent wrong while accessing Redis", 500, 0);
}

auto
zpt::redis::ZList::del(std::string const& _key, zpt::json _data) -> void {
    this->del(_key, (std::string)_data);
}

auto
zpt::redis::ZList::del(std::string const& _key, std::string const& _data) -> void {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    expect(_data.length() != 0, "'_data' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    bool _success = true;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        redisAppendCommand(this->__conn, "ZREM %s %s", _key.data(), _data.data());
        _success = (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK);
    }
    freeReplyObject(_reply);
    expect(_success, "something whent wrong while accessing Redis", 500, 0);
}

auto
zpt::redis::ZList::del(std::string const& _key, zpt::timestamp_t _min) -> void {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    bool _success = true;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        redisAppendCommand(this->__conn, "ZREMRANGEBYSCORE %s %lld +inf", _key.data(), _min);
        _success = (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK);
    }
    freeReplyObject(_reply);
    expect(_success, "something whent wrong while accessing Redis", 500, 0);
}

auto
zpt::redis::ZList::del(std::string const& _key, zpt::timestamp_t _min, zpt::timestamp_t _max)
  -> void {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    bool _success = true;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        redisAppendCommand(this->__conn, "ZREMRANGEBYSCORE %s %lld %lld", _key.data(), _min, _max);
        _success = (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK);
    }
    freeReplyObject(_reply);
    expect(_success, "something whent wrong while accessing Redis", 500, 0);
}

auto
zpt::redis::ZList::rangebypos(std::string const& _key, long int _min, long int _max) -> zpt::json {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    bool _success = true;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        redisAppendCommand(this->__conn, "ZRANGE %s %d %d WITHSCORES", _key.data(), _min, _max);
        _success =
          (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK) && (_reply != nullptr);
    }
    if (!_success && _reply != nullptr) { freeReplyObject(_reply); }
    expect(_success, "something whent wrong while accessing Redis", 500, 0);

    zpt::json _return = zpt::json::object();
    int _type = _reply->type;
    switch (_type) {
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_ERROR: {
            std::string _status_text(_reply->str, _reply->len);
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied with an error/status: ") + _status_text,
                   0,
                   0);
        }
        case REDIS_REPLY_INTEGER: {
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied something else: REDIS_REPLY_INTEGER"),
                   0,
                   0);
        }
        case REDIS_REPLY_STRING: {
            std::string _string_text(_reply->str, _reply->len);
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied something else: REDIS_REPLY_STRING > ") +
                     _string_text,
                   0,
                   0);
        }
        case REDIS_REPLY_NIL: {
            freeReplyObject(_reply);
            break;
        }
        case REDIS_REPLY_ARRAY: {
            for (size_t _i = 0; _i < _reply->elements; _i += 2) {
                char* _end;
                zpt::timestamp_t _ts = std::strtoull(
                  std::string(_reply->element[_i + 1]->str, _reply->element[_i + 1]->len).data(),
                  &_end,
                  10);
                std::string _data(_reply->element[_i]->str, _reply->element[_i]->len);
                std::string _key = std::string(zpt::json::date(_ts));
                zpt::json _payload;
                try {
                    _payload = zpt::json(_data);
                }
                catch (zpt::SyntaxErrorException const& _e) {
                    _payload = zpt::json::string(_data);
                }
                if (_return[_key]->ok()) { _return[_key] << _payload; }
                else {
                    _return << _key << zpt::json{ zpt::array, _payload };
                }
            }
            freeReplyObject(_reply);
            break;
        }
        default: {
            freeReplyObject(_reply);
            break;
        }
    }

    return { "size", _return->object()->size(), "elements", _return };
}

auto
zpt::redis::ZList::range(std::string const& _key,
                         zpt::timestamp_t _min,
                         zpt::timestamp_t _max,
                         int _direction,
                         size_t _offset,
                         size_t _limit) -> zpt::json {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    std::string _minimum("-inf");
    std::string _maximum("+inf");
    if (_min != 0) { _minimum = std::to_string(_min); }
    if (_max != 0) { _maximum = std::to_string(_max); }
    bool _success = false;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        if (_direction > 0) {
            if (_limit == 0 && _offset == 0) {
                redisAppendCommand(this->__conn,
                                   "ZRANGEBYSCORE %s %s %s WITHSCORES",
                                   _key.data(),
                                   _minimum.data(),
                                   _maximum.data());
            }
            else {
                redisAppendCommand(this->__conn,
                                   "ZRANGEBYSCORE %s %s %s WITHSCORES LIMIT %lld %lld",
                                   _key.data(),
                                   _minimum.data(),
                                   _maximum.data(),
                                   _offset,
                                   (_limit == 0 ? 999999999L : _limit));
            }
        }
        else {
            if (_limit == 0 && _offset == 0) {
                redisAppendCommand(this->__conn,
                                   "ZREVRANGEBYSCORE %s %s %s WITHSCORES",
                                   _key.data(),
                                   _maximum.data(),
                                   _minimum.data());
            }
            else {
                redisAppendCommand(this->__conn,
                                   "ZREVRANGEBYSCORE %s %s %s WITHSCORES LIMIT %lld %lld",
                                   _key.data(),
                                   _maximum.data(),
                                   _minimum.data(),
                                   _offset,
                                   (_limit == 0 ? 999999999L : _limit));
            }
        }
        _success =
          (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK) && (_reply != nullptr);
    }
    if (!_success && _reply != nullptr) { freeReplyObject(_reply); }
    expect(_success, "something whent wrong while accessing Redis", 500, 0);

    zpt::json _return = zpt::json::object();
    int _type = _reply->type;
    switch (_type) {
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_ERROR: {
            std::string _status_text(_reply->str, _reply->len);
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied with an error/status: ") + _status_text,
                   0,
                   0);
        }
        case REDIS_REPLY_INTEGER: {
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied something else: REDIS_REPLY_INTEGER"),
                   0,
                   0);
        }
        case REDIS_REPLY_STRING: {
            std::string _string_text(_reply->str, _reply->len);
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied something else: REDIS_REPLY_STRING > ") +
                     _string_text,
                   0,
                   0);
        }
        case REDIS_REPLY_NIL: {
            freeReplyObject(_reply);
            break;
        }
        case REDIS_REPLY_ARRAY: {
            for (size_t _i = 0; _i < _reply->elements; _i += 2) {
                char* _end;
                zpt::timestamp_t _ts = std::strtoull(
                  std::string(_reply->element[_i + 1]->str, _reply->element[_i + 1]->len).data(),
                  &_end,
                  10);
                std::string _data(_reply->element[_i]->str, _reply->element[_i]->len);
                std::string _key = std::string(zpt::json::date(_ts));
                zpt::json _payload;
                try {
                    _payload = zpt::json(_data);
                }
                catch (zpt::SyntaxErrorException const& _e) {
                    _payload = zpt::json::string(_data);
                }
                if (_return[_key]->ok()) { _return[_key] << _payload; }
                else {
                    _return << _key << zpt::json{ zpt::array, _payload };
                }
            }
            freeReplyObject(_reply);
            break;
        }
        default: {
            freeReplyObject(_reply);
            break;
        }
    }

    return { "size", _return->object()->size(), "elements", _return };
}

auto
zpt::redis::ZList::getall(std::string const& _key) -> zpt::json {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    redisReply* _reply = nullptr;
    bool _success = true;
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        redisAppendCommand(this->__conn, "ZRANGEBYSCORE %s -inf +inf WITHSCORES", _key.data());
        _success =
          (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK) && (_reply != nullptr);
    }
    if (!_success && _reply != nullptr) { freeReplyObject(_reply); }
    expect(_success, "something whent wrong while accessing Redis", 500, 0);

    zpt::json _return = zpt::json::object();
    int _type = _reply->type;
    switch (_type) {
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_ERROR: {
            std::string _status_text(_reply->str, _reply->len);
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied with an error/status: ") + _status_text,
                   0,
                   0);
        }
        case REDIS_REPLY_INTEGER: {
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied something else: REDIS_REPLY_INTEGER"),
                   0,
                   0);
        }
        case REDIS_REPLY_STRING: {
            std::string _string_text(_reply->str, _reply->len);
            freeReplyObject(_reply);
            expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                   std::string("Redis server replied something else: REDIS_REPLY_STRING > ") +
                     _string_text,
                   0,
                   0);
        }
        case REDIS_REPLY_NIL: {
            freeReplyObject(_reply);
            break;
        }
        case REDIS_REPLY_ARRAY: {
            for (size_t _i = 0; _i != _reply->elements; _i += 2) {
                char* _end;
                zpt::timestamp_t _ts = std::strtoull(
                  std::string(_reply->element[_i + 1]->str, _reply->element[_i + 1]->len).data(),
                  &_end,
                  10);
                std::string _data(_reply->element[_i]->str, _reply->element[_i]->len);
                std::string _key = std::string(zpt::json::date(_ts));
                zpt::json _payload;
                try {
                    _payload = zpt::json(_data);
                }
                catch (zpt::SyntaxErrorException const& _e) {
                    _payload = zpt::json::string(_data);
                }
                if (_return[_key]->ok()) { _return[_key] << _payload; }
                else {
                    _return << _key << zpt::json{ zpt::array, _payload };
                }
            }
            freeReplyObject(_reply);
            break;
        }
        default: {
            freeReplyObject(_reply);
            break;
        }
    }

    return { "size", _return->object()->size(), "elements", _return };
}

auto
zpt::redis::ZList::find(std::string const& _key, std::string const& _regexp) -> zpt::json {
    {
        std::lock_guard<std::mutex> _lock(this->__mtx);
        expect(this->__conn != nullptr,
               std::string("connection to Redis at ") + this->name() +
                 std::string(" has not been established."),
               500,
               0);
    }
    expect(_key.length() != 0, "'_key' parameter must not be empty", 0, 0);
    expect(_regexp.length() != 0, "'_regexp' parameter must not be empty", 0, 0);
    std::string _command("ZSCAN %s %i MATCH %s");
    int _cursor = 0;
    redisReply* _reply = nullptr;
    zpt::json _return = zpt::json::object();
    ;

    do {
        bool _success = true;
        {
            std::lock_guard<std::mutex> _lock(this->__mtx);
            redisAppendCommand(this->__conn, _command.data(), _key.data(), _cursor, _regexp.data());
            _success =
              (redisGetReply(this->__conn, (void**)&_reply) == REDIS_OK) && (_reply != nullptr);
        }
        if (!_success && _reply != nullptr) { freeReplyObject(_reply); }
        expect(_success, "something whent wrong while accessing Redis", 500, 0);

        int _type = _reply->type;
        switch (_type) {
            case REDIS_REPLY_STATUS:
            case REDIS_REPLY_ERROR: {
                _cursor = 0;
                std::string _status_text(_reply->str, _reply->len);
                freeReplyObject(_reply);
                expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                       std::string("Redis server replied with an error/status: ") + _status_text,
                       0,
                       0);
            }
            case REDIS_REPLY_INTEGER: {
                _cursor = 0;
                freeReplyObject(_reply);
                expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                       std::string("Redis server replied something else: REDIS_REPLY_INTEGER"),
                       0,
                       0);
            }
            case REDIS_REPLY_STRING: {
                _cursor = 0;
                std::string _string_text(_reply->str, _reply->len);
                freeReplyObject(_reply);
                expect(_type == REDIS_REPLY_NIL || _type == REDIS_REPLY_ARRAY,
                       std::string("Redis server replied something else: REDIS_REPLY_STRING > ") +
                         _string_text,
                       0,
                       0);
            }
            case REDIS_REPLY_NIL: {
                _cursor = 0;
                freeReplyObject(_reply);
                break;
            }
            case REDIS_REPLY_ARRAY: {
                _cursor = std::stoi(std::string(_reply->element[0]->str, _reply->element[0]->len));
                for (size_t _i = 0; _i < _reply->element[1]->elements; _i += 2) {
                    char* _end;
                    zpt::timestamp_t _ts = std::strtoull(
                      std::string(_reply->element[_i + 1]->str, _reply->element[_i + 1]->len)
                        .data(),
                      &_end,
                      10);
                    std::string _data(_reply->element[_i]->str, _reply->element[_i]->len);
                    std::string _key = std::string(zpt::json::date(_ts));
                    zpt::json _payload;
                    try {
                        _payload = zpt::json(_data);
                    }
                    catch (zpt::SyntaxErrorException const& _e) {
                        _payload = zpt::json::string(_data);
                    }
                    if (_return[_key]->ok()) { _return[_key] << _payload; }
                    else {
                        _return << _key << zpt::json{ zpt::array, _payload };
                    }
                }
                freeReplyObject(_reply);
                break;
            }
            default: {
                _cursor = 0;
                freeReplyObject(_reply);
                break;
            }
        }
    } while (_cursor != 0);

    return { "size", _return->object()->size(), "elements", _return };
}
