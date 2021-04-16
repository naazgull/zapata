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

zpt::automaton::engine::engine(long _processor_threads, zpt::json _configuration)
  : zpt::fsm::machine<engine, zpt::json, zpt::exchange, zpt::json>{ _processor_threads,
                                                                    __hazard_domain,
                                                                    _configuration }
  , __configuration{ _configuration } {
    this->__hazard_domain.set_limits(_processor_threads + 2, 4);

    zpt::json _extra{ "begin", zpt::automaton::engine::receive(),
                      "end",   zpt::automaton::engine::send(),
                      "pause", zpt::automaton::engine::pause() };
    this                                                      //
      ->set_states((this->__configuration - _extra) | _extra) //
      .set_error_callback(zpt::automaton::engine::on_error)   //
      .add_allowed_transitions({ zpt::array,
                                 { zpt::array,
                                   zpt::automaton::engine::receive(),
                                   { zpt::array, this->__configuration["begin"] } },
                                 { zpt::array,
                                   this->__configuration["end"],
                                   { zpt::array, zpt::automaton::engine::send() } } }) //
      .add_transition(
        zpt::automaton::engine::receive(),
        [this](
          zpt::json _state, zpt::exchange& _channel, zpt::json const& _id) mutable -> zpt::json {
            auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
            auto& _transport = _layer.get(_channel->scheme());
            _transport->receive(_channel);
            if (_channel->received()["performative"] == zpt::Patch) {
                expect(_channel->received()["state"]->ok(),
                       "a `state` must be provided to the CONTINUE directive",
                       412,
                       0);
                expect(_channel->received()["id"]->ok(),
                       "an `id` must be provided to the CONTINUE directive",
                       412,
                       0);
                this->resume(_channel->received()["id"], _channel->received()["state"]);
                return zpt::automaton::engine::pause();
            }
            return this->__configuration["begin"];
        }) //
      .add_transition(
        this->__configuration["end"],
        [](zpt::json _state, zpt::exchange& _channel, zpt::json const& _id) -> zpt::json {
            return zpt::automaton::engine::send();
        }) //
      .add_transition(
        zpt::automaton::engine::send(),
        [this](
          zpt::json _state, zpt::exchange& _channel, zpt::json const& _id) mutable -> zpt::json {
            _channel->to_send() = {
                "status",
                200,
                "body",
                { "id", _id, "state", this->__configuration["end"], "data", _channel->to_send() }
            };
            zpt::automaton::engine::transmit(_channel);
            return this->__configuration["undefined"];
        }) //
      .add_transition(
        zpt::automaton::engine::pause(),
        [](zpt::json _state, zpt::exchange& _channel, zpt::json const& _id) -> zpt::json {
            _channel->to_send() = { "status", 202, "body", { "id", _id, "state", "PAUSED" } };
            zpt::automaton::engine::transmit(_channel);
            return zpt::automaton::engine::pause();
            ;
        });
    this->__initialized = true;
}

auto
zpt::automaton::engine::to_string() -> std::string {
    std::ostringstream _oss;
    _oss << static_cast<zpt::fsm::machine<engine, zpt::json, zpt::exchange, zpt::json>*>(this)
              ->to_string()
         << std::flush;
    return _oss.str();
}

auto
zpt::automaton::engine::verify_allowed_transition(zpt::json _from, zpt::json _to) -> void {
    if (!this->__initialized) { return; }
    expect(_from != zpt::automaton::engine::receive() && _to != zpt::automaton::engine::send() &&
             _from != this->__configuration["undefined"] &&
             _to != this->__configuration["undefined"] &&
             _from != zpt::automaton::engine::pause() && _to != zpt::automaton::engine::pause(),
           "invalid transition nodes",
           500,
           0);
}

auto
zpt::automaton::engine::verify_transition(zpt::json _current) -> void {
    if (!this->__initialized) { return; }
    expect(_current != zpt::automaton::engine::receive() &&
             _current != zpt::automaton::engine::send() &&
             _current != this->__configuration["undefined"] &&
             _current != zpt::automaton::engine::pause(),
           "invalid transition node",
           500,
           0);
}

auto
zpt::automaton::engine::on_error(zpt::json const& _state,
                                 zpt::exchange& _channel,
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
    zpt::automaton::engine::transmit(_channel);
    return true;
}

auto
zpt::automaton::engine::to_string() -> std::string {
    std::ostringstream _oss;
    _oss << static_cast<zpt::fsm::machine<zpt::json, zpt::exchange, zpt::json>&>(*this)->to_string()
         << std::flush;
    return _oss.str();
}

auto
zpt::automaton::engine::receive() -> zpt::json {
    static zpt::json _receive_state{ "__receive__" };
    return _receive_state;
}

auto
zpt::automaton::engine::send() -> zpt::json {
    static zpt::json _send_state{ "__send__" };
    return _send_state;
}

auto
zpt::automaton::engine::pause() -> zpt::json {
    static zpt::json _pause_state{ "__pause__" };
    return _pause_state;
}

auto
zpt::automaton::engine::transmit(zpt::exchange& _channel) -> void {
    auto& _polling = zpt::globals::get<zpt::stream::polling>(zpt::STREAM_POLLING());
    auto& _layer = zpt::globals::get<zpt::transport::layer>(zpt::TRANSPORT_LAYER());
    auto& _transport = _layer.get(_channel->scheme());
    _transport->send(_channel);
    std::unique_ptr<zpt::stream> _give_back{ &_channel->stream() };
    if (_channel->keep_alive()) { _polling.listen_on(_give_back); }
}
