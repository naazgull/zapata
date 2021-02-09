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

#define ZMQ_PUB_SUB -1
#define ZMQ_XPUB_XSUB -2
#define ZMQ_ROUTER_DEALER -3
#define ZMQ_ASSYNC_REQ -4
#define ZMQ_HTTP_RAW -5
#define ZMQ_MQTT_RAW -6
#define ZMQ_UPNP_RAW -7

#define ZPT_SELF_CERTIFICATE 0
#define ZPT_PEER_CERTIFICATE 1

namespace zpt {
extern zmq::context_t __context;

auto
str2type(std::string const& _type) -> short;
auto
type2str(short _type) -> std::string;

namespace assync {
typedef std::function<std::string(zpt::json _envelope)> reply_fn;
}

class ZMQPoll : public zpt::Poll {
  public:
    ZMQPoll(zpt::json _options, zpt::ev::emitter _emiter = nullptr);
    virtual ~ZMQPoll();

    virtual auto options() -> zpt::json;
    virtual auto emitter() -> zpt::ev::emitter_factory;
    virtual auto self() const -> zpt::poll;

    virtual auto get(std::string const& _uuid) -> zpt::socket_ref;
    virtual auto relay(std::string const& _key) -> zpt::Channel*;
    virtual auto add(short _type, std::string const& _connection, bool _new_connection = false)
      -> zpt::socket_ref;
    virtual auto add(zpt::Channel* _underlying) -> zpt::socket_ref;
    virtual auto remove(zpt::socket_ref _socket) -> void;
    virtual auto vanished(std::string const& _connection, zpt::ev::initializer _callback = nullptr)
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
    zpt::ev::emitter __emitter;
    bool __needs_rebuild;
    std::map<zpt::socket_ref, std::string> __to_add;
    std::map<zpt::socket_ref, zpt::ev::initializer> __to_remove;

    auto bind(short _type, std::string const& _connection) -> zpt::Channel*;
    auto signal(std::string const& _message) -> void;
    auto notify(std::string const& _message) -> void;
    auto wait() -> void;
    auto repoll() -> void;
    auto reply(zpt::json _envelope, zpt::socket_ref _socket) -> void;
};

class ZMQChannel : public zpt::Channel {
  public:
    ZMQChannel(std::string const& _connection, zpt::json _options);
    virtual ~ZMQChannel();

    auto recv() -> zpt::json;
    auto send(zpt::json _envelope) -> zpt::json;
};

class ZMQReq : public zpt::ZMQChannel {
  public:
    ZMQReq(std::string const& _connection, zpt::json _options);
    virtual ~ZMQReq();

    // virtual zpt::json send(zpt::json _envelope);

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket;
};

class ZMQRep : public zpt::ZMQChannel {
  public:
    ZMQRep(std::string const& _connection, zpt::json _options);
    virtual ~ZMQRep();

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket;
};

class ZMQXPubXSub : public zpt::ZMQChannel {
  public:
    ZMQXPubXSub(std::string const& _connection, zpt::json _options);
    virtual ~ZMQXPubXSub();

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket_sub;
    zmq::socket_ptr __socket_pub;
};

class ZMQPubSub : public zpt::ZMQChannel {
  public:
    ZMQPubSub(std::string const& _connection, zpt::json _options);
    virtual ~ZMQPubSub();

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

    virtual void subscribe(std::string const& _prefix);

  private:
    zmq::socket_ptr __socket_sub;
    zmq::socket_ptr __socket_pub;
    std::mutex __out_mtx;
};

class ZMQPub : public zpt::ZMQChannel {
  public:
    ZMQPub(std::string const& _connection, zpt::json _options);
    virtual ~ZMQPub();

    virtual zpt::json recv();

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket;
};

class ZMQSub : public zpt::ZMQChannel {
  public:
    ZMQSub(std::string const& _connection, zpt::json _options);
    virtual ~ZMQSub();

    virtual zpt::json send(zpt::json _envelope);

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

    virtual void subscribe(std::string const& _prefix);

  private:
    zmq::socket_ptr __socket;
};

class ZMQPush : public zpt::ZMQChannel {
  public:
    ZMQPush(std::string const& _connection, zpt::json _options);
    virtual ~ZMQPush();

    virtual zpt::json recv();

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket;
};

class ZMQPull : public zpt::ZMQChannel {
  public:
    ZMQPull(std::string const& _connection, zpt::json _options);
    virtual ~ZMQPull();

    virtual zpt::json send(zpt::json _envelope);

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket;
};

class ZMQRouterDealer : public zpt::ZMQChannel {
  public:
    ZMQRouterDealer(std::string const& _connection, zpt::json _options);
    virtual ~ZMQRouterDealer();

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket_router;
    zmq::socket_ptr __socket_dealer;
};

class ZMQRouter : public zpt::ZMQChannel {
  public:
    ZMQRouter(std::string const& _connection, zpt::json _options);
    virtual ~ZMQRouter();

    virtual auto send(zpt::json _envelope) -> zpt::json;
    virtual auto recv() -> zpt::json;

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket;
    std::map<std::string, zmq::message_t*> __sock_id;
    std::mutex __sock_mtx;
    std::mutex __in_mtx;
    std::mutex __out_mtx;
};

class ZMQDealer : public zpt::ZMQChannel {
  public:
    ZMQDealer(std::string const& _connection, zpt::json _options);
    virtual ~ZMQDealer();

    virtual auto send(zpt::json _envelope) -> zpt::json;
    virtual auto recv() -> zpt::json;

    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;

  private:
    zmq::socket_ptr __socket;
};

class ZMQFactory : public zpt::ChannelFactory {
  public:
    ZMQFactory();
    virtual ~ZMQFactory();
    virtual auto produce(zpt::json _options) -> zpt::socket;
    virtual auto is_reusable(std::string const& _type) -> bool;
    virtual auto clean(zpt::socket _socket) -> bool;

  private:
    std::map<std::string, zpt::socket> __channels;
};
} // namespace zpt
