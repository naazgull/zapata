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
                           zpt::ontology::to_str(_channel->received()["performative"]->integer()) +
                           _channel->uri());
      });

    zpt::pipeline::engine<zpt::exchange>::add_listener(
      1, "/ROOT/REPLY/{(.*)}", [this](zpt::pipeline::event<zpt::exchange>& _event) mutable -> void {
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
                  if (_channel->keep_alive()) { _polling.listen_on(_give_back); }
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
          try {
              _transport->send(_channel);
              std::unique_ptr<zpt::stream> _give_back{ &_channel->stream() };
              if (_channel->keep_alive()) { _polling.listen_on(_give_back); }
          }
          catch (zpt::failed_expectation const& _e) {
              zlog(_e, zpt::emergency);
          }
          catch (std::exception const& _e) {
              zlog(_e.what(), zpt::emergency);
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
    _channel->stream().state() = zpt::stream_state::PROCESSING;
    _channel->version().assign("1.1");
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
