/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright inteautomaton in the software to the public
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

#include <zapata/automaton/automaton.h>

auto
zpt::AUTOMATON_ENGINE() -> ssize_t& {
    static ssize_t _global{ -1 };
    return _global;
}

zpt::automaton::engine::engine(long _threads, zpt::json _configuration)
  : zpt::fsm::machine<zpt::json, zpt::exchange, zpt::json>{ _threads, _configuration }
  , __configuration{ _configuration } {

    (*this) //
      ->set_error_callback(zpt::automaton::engine::on_error)
      .add_transition(zpt::automaton::engine::receive(),
                      [=](zpt::json _state, zpt::exchange& _channel) -> zpt::json {
                          auto& _layer =
                            zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
                          auto& _transport = _layer.get(_channel->scheme());
                          _transport->receive(_channel);
                          return this->__configuration["begin"];
                      }) //
      .add_transition(
        _configuration["end"], [=](zpt::json _state, zpt::exchange& _channel) -> zpt::json {
            auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
            auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
            auto& _transport = _layer.get(_channel->scheme());
            _transport->send(_channel);
            std::unique_ptr<zpt::stream> _give_back{ &_channel->stream() };
            if (_channel->keep_alive()) { _polling.listen_on(_give_back); }
            return this->__configuration["undefined"];
        });
}

auto
zpt::automaton::engine::on_error(zpt::json const& _state,
                                 zpt::exchange const& _channel,
                                 zpt::json const& _id,
                                 const char* _what,
                                 const char* _description,
                                 const char* _backtrace,
                                 int _error,
                                 int _status) -> bool {
    _channel->stream().state() = zpt::stream_state::PROCESSING;
    _channel->version().assign("1.1");
    _channel->to_send() = {
        "status",
        _status,
        "body",
        { "id", _id, "state", _state, "error", _error, "message", std::string{ _what } }
    };
    if (_description != nullptr) {
        _channel->to_send()["body"] << "description" << std::string{ _description };
    }
    if (_backtrace != nullptr) {
        _channel->to_send()["body"] << "backtrace" << zpt::split(std::string{ _backtrace }, "\n");
    }
    return false;
}

auto
zpt::automaton::engine::receive() -> zpt::json {
    static zpt::json _receive_state{ "AUTOMATON_RECEIVE" };
    return _receive_state;
}
