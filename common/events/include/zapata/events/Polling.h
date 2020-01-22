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

#include <map>
#include <memory>
#include <mutex>
#include <poll.h>
#include <string>
#include <zapata/base.h>
#include <zapata/events.h>
#include <zapata/http/HTTPObj.h>
#include <zapata/json.h>
#include <zapata/zmq/socket_stream.h>
#include <zmq.h>
#include <zmq.hpp>

#define ZPT_SELF_CERTIFICATE 0
#define ZPT_PEER_CERTIFICATE 1

namespace zpt {
extern zmq::context_t __context;

auto
str2type(std::string _type) -> short;
auto
type2str(short _type) -> std::string;

namespace assync {
typedef std::function<std::string(zpt::json _envelope)> reply_fn;
}

class ChannelPoll : public zpt::Poll {
  public:
    ChannelPoll(zpt::json _options, zpt::ev::emitter_factory _emiter);
    virtual ~ChannelPoll();

    virtual auto options() -> zpt::json;
    virtual auto emitter() -> zpt::ev::emitter_factory;
    virtual auto self() const -> zpt::poll;

    virtual auto get(std::string _uuid) -> zpt::socket_ref;
    virtual auto relay(std::string _key) -> zpt::Channel*;
    virtual auto add(std::string _type, std::string _connection, bool _new_connection = false)
      -> zpt::socket_ref;
    virtual auto add(zpt::Channel* _underlying) -> zpt::socket_ref;
    virtual auto remove(zpt::socket_ref _socket) -> void;
    virtual auto vanished(std::string _connection, zpt::ev::initializer _callback = nullptr)
      -> void;
    virtual auto vanished(zpt::Channel* _underlying, zpt::ev::initializer _callback = nullptr)
      -> void;
    virtual auto pretty() -> std::string;

    virtual auto poll(zpt::socket_ref _socket) -> void;
    virtual auto clean_up(zpt::socket_ref _socket, bool _force = false) -> void;

    virtual auto loop() -> void;

  private:
    zpt::json __options;
    std::map<std::string, zpt::Channel*> __by_refs;
    std::vector<zpt::socket_ref> __by_socket;
    std::vector<zmq::pollitem_t> __items;
    ::pthread_t __id;
    zmq::socket_ptr __sync[2];
    std::mutex __mtx[2];
    zpt::poll __self;
    zpt::ev::emitter_factory __emitter;
    bool __needs_rebuild;
    std::map<zpt::socket_ref, std::string> __to_add;
    std::map<zpt::socket_ref, zpt::ev::initializer> __to_remove;

    auto bind(std::string _type, std::string _connection) -> std::vector<zpt::socket>;
    auto signal(std::string _message) -> void;
    auto notify(std::string _message) -> void;
    auto wait() -> void;
    auto repoll() -> void;
    auto reply(zpt::json _envelope, zpt::socket_ref _socket) -> void;
};

auto
http2internal(zpt::http::req _request) -> zpt::json;
auto
http2internal(zpt::http::rep _reply) -> zpt::json;
auto
internal2http_rep(zpt::json _out) -> zpt::http::rep;
auto
internal2http_req(zpt::json _out, std::string _host) -> zpt::http::req;

namespace http {
zpt::json
deserialize(std::string _body);
}
} // namespace zpt
