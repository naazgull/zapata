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

#include <zapata/rest/rest.h>

auto
zpt::REST_ENGINE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::rest::engine::engine(size_t _pipeline_size, zpt::json _configuration)
  : zpt::pipeline::engine<zpt::exchange>{ _pipeline_size + 2, _configuration } {

    this->set_error_callback(zpt::rest::engine::on_error);

    zpt::pipeline::engine<zpt::exchange>::add_listener(
      0, "{(.*)}", [](zpt::pipeline::event<zpt::exchange>& _event) -> void {
          auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
          auto& _channel = _event->content();
          auto& _transport = _layer.get(_channel->scheme());

          _transport->receive(_channel);
          _event->set_path(std::string("/ROOT/") +
                           zpt::rest::to_str(_channel->received()["performative"]->integer()) +
                           _channel->uri());
      });

    zpt::pipeline::engine<zpt::exchange>::add_listener(
      1, "/ROOT/REPLY/{(.*)}", [=](zpt::pipeline::event<zpt::exchange>& _event) mutable -> void {
          auto& _channel = _event->content();
          std::string _key{ "/REPLY" };
          _key.append(_channel->uri());
          {
              zpt::lf::spin_lock::guard _shared_sentry{ this->__pending_lock,
                                                        zpt::lf::spin_lock::shared };
              auto _found = this->__pending.find(_key);
              if (_found != this->__pending.end()) {
                  _found->second(_event);
                  auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
                  std::unique_ptr<zpt::stream> _give_back{ &_channel->stream() };
                  if (_channel->keep_alive()) {
                      _polling.listen_on(_give_back);
                  }
                  _event.cancel();
              }
          }
          {
              zpt::lf::spin_lock::guard _shared_sentry{ this->__pending_lock,
                                                        zpt::lf::spin_lock::exclusive };
              this->__pending.erase(_key);
          }
      });

    zpt::pipeline::engine<zpt::exchange>::add_listener(
      _pipeline_size + 1, "{(.*)}", [](zpt::pipeline::event<zpt::exchange>& _event) -> void {
          auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
          auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
          auto& _channel = _event->content();
          auto& _transport = _layer.get(_channel->scheme());
          _transport->send(_channel);
          std::unique_ptr<zpt::stream> _give_back{ &_channel->stream() };
          if (_channel->keep_alive()) {
              _polling.listen_on(_give_back);
          }
      });
}

auto
zpt::rest::engine::add_listener(std::string _pattern,
                                std::function<void(zpt::pipeline::event<zpt::exchange>&)> _callback)
  -> zpt::rest::engine& {
    return this->add_listener(0, _pattern, _callback);
}

auto
zpt::rest::engine::add_listener(size_t _stage,
                                std::string _pattern,
                                std::function<void(zpt::pipeline::event<zpt::exchange>&)> _callback)
  -> zpt::rest::engine& {
    zlog("Registering " << _pattern, zpt::trace);
    if (_pattern.find("/REPLY/") == 0) {
        zpt::lf::spin_lock::guard _shared_sentry{ this->__pending_lock,
                                                  zpt::lf::spin_lock::exclusive };
        this->__pending.insert(std::make_pair(_pattern, _callback));
    }
    else {
        _pattern.insert(0, "/ROOT");
        zpt::pipeline::engine<zpt::exchange>::add_listener(_stage + 1, _pattern, _callback);
    }
    return (*this);
}

auto
zpt::rest::engine::request(std::string _uri,
                           std::function<void(zpt::pipeline::event<zpt::exchange>&)> _callback)
  -> zpt::rest::engine& {
    auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
    auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
    auto _channel = _layer.resolve(_uri);
    auto& _transport = _layer.get(_channel->scheme());

    this->add_listener("/REPLY/" + std::to_string(static_cast<int>(_channel->stream())), _callback);
    _transport->send(_channel);

    std::unique_ptr<zpt::stream> _give_back{ &_channel->stream() };
    _polling.listen_on(_give_back);
    return (*this);
}

auto
zpt::rest::engine::on_error(zpt::json& _path,
                            zpt::pipeline::event<zpt::exchange>& _event,
                            const char* _what,
                            const char* _description,
                            const char* _backtrace,
                            int _error,
                            int _status) -> bool {
    auto& _channel = _event->content();
    _channel->to_send() = {
        "status", _status, "body", { "error", _error, "message", std::string{ _what } }
    };
    if (_description != nullptr) {
        _channel->to_send()["body"] << "description" << std::string{ _description };
    }
    if (_backtrace != nullptr) {
        _channel->to_send()["body"] << "backtrace" << zpt::split(std::string{ _backtrace }, "\n");
    }
    return true;
}

auto
zpt::rest::to_str(zpt::performative _performative) -> std::string {
    switch (_performative) {
        case zpt::rest::Get: {
            return "GET";
        }
        case zpt::rest::Put: {
            return "PUT";
        }
        case zpt::rest::Post: {
            return "POST";
        }
        case zpt::rest::Delete: {
            return "DELETE";
        }
        case zpt::rest::Head: {
            return "HEAD";
        }
        case zpt::rest::Options: {
            return "OPTIONS";
        }
        case zpt::rest::Patch: {
            return "PATCH";
        }
        case zpt::rest::Reply: {
            return "REPLY";
        }
        case zpt::rest::Msearch: {
            return "M-SEARCH";
        }
        case zpt::rest::Notify: {
            return "NOTIFY";
        }
        case zpt::rest::Trace: {
            return "TRACE";
        }
        case zpt::rest::Connect: {
            return "CONNECT";
        }
    }
    return "HEAD";
}

auto
zpt::rest::from_str(std::string const& _performative) -> zpt::performative {
    if (_performative == "GET" || _performative == "get") {
        return zpt::rest::Get;
    }
    if (_performative == "PUT" || _performative == "put") {
        return zpt::rest::Put;
    }
    if (_performative == "POST" || _performative == "post") {
        return zpt::rest::Post;
    }
    if (_performative == "DELETE" || _performative == "delete") {
        return zpt::rest::Delete;
    }
    if (_performative == "HEAD" || _performative == "head") {
        return zpt::rest::Head;
    }
    if (_performative == "OPTIONS" || _performative == "options") {
        return zpt::rest::Options;
    }
    if (_performative == "PATCH" || _performative == "patch") {
        return zpt::rest::Patch;
    }
    if (_performative == "REPLY" || _performative == "reply") {
        return zpt::rest::Reply;
    }
    if (_performative == "M-SEARCH" || _performative == "m-search") {
        return zpt::rest::Msearch;
    }
    if (_performative == "NOTIFY" || _performative == "notify") {
        return zpt::rest::Notify;
    }
    if (_performative == "TRACE" || _performative == "trace") {
        return zpt::rest::Msearch;
    }
    if (_performative == "CONNECT" || _performative == "connect") {
        return zpt::rest::Connect;
    }
    return 0;
}
