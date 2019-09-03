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

#include <zapata/base.h>
#include <zmq.h>
#include <zmq.hpp>

namespace zmq {
typedef std::shared_ptr<zmq::socket_t> channel_ptr;
}

namespace zpt {

class drop;
class pipeline;
class pipe;
class abstract_channel_factory;
class abstract_channel;

typedef std::shared_ptr<abstract_channel> channel;
typedef std::shared_ptr<abstract_channel_factory> channel_factory;
typedef std::tuple<std::string, std::function<void(zpt::json, zpt::drop&)>> initializer;
typedef std::tuple<std::string, std::function<void(zpt::channel, zpt::drop&)>> receiver;
typedef std::function<void(std::string, zpt::json, zpt::drop&)> request_transformer;
typedef std::tuple<
  std::string,
  std::map<zpt::performative,
           std::function<void(zpt::performative, std::string, zpt::json, zpt::drop&)>>>
  replier;
typedef std::function<void(zpt::json, zpt::drop&)> reply_transformer;

auto
options(zpt::json _options = zpt::undefined) -> zpt::json&;
auto
options(int argc, char* argv[]) -> zpt::json&;

// class channel_ref : public std::string {
//       public:
// 	channel_ref();
// 	channel_ref(std::string _rhs, zpt::poll _poll);
// 	channel_ref(const zpt::channel_ref& _rhs);

// 	auto poll(zpt::poll _poll) -> void;
// 	auto poll() -> zpt::poll;
// 	auto operator-> () -> zpt::channel*;
// 	auto operator*() -> zpt::channel*;

//       private:
// 	zpt::poll __poll;
// };

class abstract_channel_factory {
  public:
    abstract_channel_factory(std::string _protocol);
    virtual ~abstract_channel_factory();

    virtual auto produce(zpt::json _options) -> zpt::channel&& = 0;
    virtual auto clean(zpt::channel& _socket) -> bool = 0;
    virtual auto is_reusable(std::string _type) -> bool = 0;

    virtual auto protocol() -> std::string final;

  private:
    std::string __protocol;
};

class abstract_channel {
  public:
    abstract_channel(std::string _connection, zpt::json _options);
    virtual ~abstract_channel();

    virtual auto id() -> std::string;
    virtual auto options() -> zpt::json;
    virtual auto connection() -> std::string;
    virtual auto connection(std::string _connection) -> void;
    virtual auto uri(size_t _idx = 0) -> zpt::json;
    virtual auto uri(std::string _connection) -> void;
    virtual auto detach() -> void;
    virtual auto close() -> void;
    virtual auto available() -> bool;

    virtual auto recv() -> zpt::json = 0;
    virtual auto send(zpt::json _envelope) -> zpt::json = 0;
    virtual auto loop_iteration() -> void;

    virtual auto channel_ptr() -> zmq::channel_ptr = 0;
    virtual auto in() -> zmq::channel_ptr = 0;
    virtual auto out() -> zmq::channel_ptr = 0;
    virtual auto fd() -> int = 0;
    virtual auto in_mtx() -> std::mutex& = 0;
    virtual auto out_mtx() -> std::mutex& = 0;
    virtual auto type() -> short int = 0;
    virtual auto protocol() -> std::string = 0;
    virtual auto is_reusable() -> bool = 0;

  private:
    zpt::json __options;
    std::string __connection;
    std::string __id;
    zpt::json __uri;

  protected:
    std::mutex __mtx;
    // zpt::poll __poll;
};

class drop {
  public:
    drop();
    drop(zpt::json _initial, zpt::channel _channel);
    drop(const zpt::drop& _rhs);
    drop(zpt::drop&& _rhs);
    virtual ~drop();

    auto channel(zpt::channel) -> void;
    auto channel() -> zpt::channel;
    auto current() -> zpt::json;

    auto operator=(const zpt::drop& _rhs) -> zpt::drop&;
    auto operator=(zpt::drop&& _rhs) -> zpt::drop&;

    auto forward(zpt::json _message) -> void;
    auto reply(zpt::json _reply) -> void;
    auto route(zpt::json _request) -> void;

  private:
    zpt::json __current;
    zpt::channel __channel;
};

class pipeline {
  public:
    pipeline(std::string _name, zpt::json _options);
    pipeline(const zpt::pipeline& _rhs);
    pipeline(zpt::pipeline&& _rhs);
    virtual ~pipeline();

    virtual void start();

    virtual auto name() -> std::string;
    virtual auto uuid() -> std::string;
    virtual auto options() -> zpt::json;

    virtual auto add(zpt::channel_factory _factory) -> void;
    virtual auto add(zpt::initializer _callback) -> void;
    virtual auto add(zpt::receiver _callback) -> void;
    virtual auto add(zpt::request_transformer _callback) -> void;
    virtual auto add(zpt::replier _callback) -> void;
    virtual auto add(zpt::reply_transformer _callback) -> void;

    auto operator=(const zpt::pipeline& _rhs) -> zpt::pipeline&;
    auto operator=(zpt::pipeline&& _rhs) -> zpt::pipeline&;

    auto feed(zpt::json _request, zpt::channel _channel) -> void;
    auto forward(zpt::json _msg) -> void;
    auto reply(zpt::json _reply) -> void;
    auto cancel() -> void;

    auto static instance() -> zpt::pipeline;

  private:
    static auto process_initalizers(zpt::drop& _drop, zpt::pipeline& _pipeline) -> bool;
    static auto process_receivers(zpt::drop& _drop, zpt::pipeline& _pipeline) -> bool;
    static auto process_req_transformers(zpt::drop& _drop, zpt::pipeline& _pipeline) -> bool;
    static auto process_repliers(zpt::drop& _drop, zpt::pipeline& _pipeline) -> bool;
    static auto process_rep_transformers(zpt::drop& _drop, zpt::pipeline& _pipeline) -> bool;

    std::string __name;
    std::string __uuid;
    zpt::json __options;
    short __drop;
    std::vector<std::function<bool(zpt::drop&, zpt::pipeline&)>> __callbacks;
    std::tuple<std::vector<zpt::initializer>,
               std::vector<zpt::receiver>,
               std::vector<zpt::request_transformer>,
               std::vector<zpt::replier>,
               std::vector<zpt::reply_transformer>>
      __drops;
    std::map<std::string, zpt::channel_factory> __factories;

    static std::atomic<bool> __ready;

    auto init() -> void;
};

class pipe : public zpt::deque<zpt::drop> {
  public:
    pipe();
    pipe(const zpt::pipe& _rhs);
    pipe(zpt::pipe&& _rhs);
    virtual ~pipe();

    auto operator=(const zpt::pipe& _rhs) -> zpt::pipe&;
    auto operator=(zpt::pipe&& _rhs) -> zpt::pipe&;

  private:
};

extern pid_t root;
extern bool interrupted;

// auto terminate(int _signal) -> void;
// auto shutdown(int _signal) -> void;

// namespace authorization {
// auto serialize(zpt::json _info) -> std::string;
// auto deserialize(std::string _token) -> zpt::json;
// auto extract(zpt::json _envelope) -> std::string;
// auto headers(std::string _token) -> zpt::json;
// auto validate(std::string _topic,
// 	      zpt::json _envelope,
// 	      zpt::ev::emitter _emitter,
// 	      zpt::json _roles_needed = zpt::undefined) -> zpt::json;
// auto has_roles(zpt::json _indentity, zpt::json _roles_needed) -> bool;
// }

// namespace scopes {
// std::string serialize(zpt::json _info);
// zpt::json deserialize(std::string _scope);
// bool has_permission(std::string _scope, std::string _ns, std::string
// _permissions);
// bool has_permission(zpt::json _scope, std::string _ns, std::string
// _permissions);
// }
} // namespace zpt
