#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <poll.h>
#include <string>
#include <zapata/base.h>
#include <zapata/net/socket.h>
#include <zapata/http.h>
#include <zapata/json.h>
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

class HTTP : public zpt::Channel {
  public:
    HTTP(zpt::socketstream_ptr _underlying, zpt::json _options);
    virtual ~HTTP();

    virtual auto recv() -> zpt::json;
    virtual auto send(zpt::json _envelope) -> zpt::json;

    virtual auto underlying() -> zpt::socketstream_ptr;
    virtual auto socket() -> zmq::socket_ptr;
    virtual auto in() -> zmq::socket_ptr;
    virtual auto out() -> zmq::socket_ptr;
    virtual auto fd() -> int;
    virtual auto in_mtx() -> std::mutex&;
    virtual auto out_mtx() -> std::mutex&;
    virtual auto type() -> short int;
    virtual auto protocol() -> std::string;
    virtual auto close() -> void;
    virtual auto available() -> bool;

  private:
    zpt::socketstream_ptr __underlying;
    short __state;
    std::string __cid;
    std::string __resource;
};

class HTTPFactory : public zpt::ChannelFactory {
  public:
    HTTPFactory();
    virtual ~HTTPFactory();
    virtual auto produce(zpt::json _options) -> zpt::socket;
    virtual auto is_reusable(std::string _type) -> bool;
    virtual auto clean(zpt::socket _socket) -> bool;
};
} // namespace zpt
