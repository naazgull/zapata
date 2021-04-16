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

#pragma once

#include <zapata/startup.h>
#include <zapata/fsm.h>
#include <zapata/transport.h>

namespace zpt {
auto
AUTOMATON_ENGINE() -> ssize_t&;

namespace automaton {
class engine : public zpt::fsm::machine<engine, zpt::json, zpt::exchange, zpt::json> {
  public:
    using hazard_domain =
      zpt::fsm::machine<engine, zpt::json, zpt::exchange, zpt::json>::hazard_domain;

    engine(long _processor_threads, zpt::json _configuration);
    virtual ~engine() = default;

    auto to_string() -> std::string;

    friend auto operator<<(std::ostream& _out, engine& _in) -> std::ostream& {
        _out << _in.to_string() << std::flush;
        return _out;
    }

    auto verify_allowed_transition(zpt::json _from, zpt::json _to) -> void;
    auto verify_transition(zpt::json _current) -> void;

    static auto on_error(zpt::json const& _state,
                         zpt::exchange& _channel,
                         zpt::json const& _id,
                         const char* _what,
                         const char* _description = nullptr,
                         const char* _backtrace = nullptr,
                         int _error = -1,
                         int _status = 500) -> bool;

  private:
    zpt::json __configuration;
    hazard_domain __hazard_domain;
    bool __initialized{ false };

    static auto receive() -> zpt::json;
    static auto send() -> zpt::json;
    static auto pause() -> zpt::json;
    static auto transmit(zpt::exchange& _channel) -> void;
};

} // namespace automaton
} // nanespace zpt
