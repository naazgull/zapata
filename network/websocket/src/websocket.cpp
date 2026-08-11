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

/**
 * @file websocket.cpp
 * @brief WebSocket transport implementation.
 *
 * Provides WebSocket message framing, frame read/write utilities,
 * and the zpt::net::transport::websocket transport class.
 */

#include <zapata/base.h>
#include <zapata/globals/globals.h>
#include <zapata/http.h>
#include <zapata/net/socket/socket_stream.h>
#include <zapata/net/transport/websocket.h>

using request_type = zpt::ws_message;
using reply_type = zpt::ws_message;

namespace {
/** @brief Exception thrown when a WebSocket message is not valid JSON.
 *
 * Wraps the original message content for debugging purposes.
 */
class non_json_message : public std::exception {
  public:
    zpt::json __original; ///< Original non-JSON message content.
    non_json_message(zpt::json _original)
      : __original{ _original } {}
};
} // namespace

auto zpt::ws_message::to_stream(std::ostream& _out) const -> zpt::basic_message const& {
    std::ostringstream _oss;
    zpt::json_message::to_stream(_oss);
    zpt::net::ws::write(_out, _oss.str(), false); // No masking for server messages
    return (*this);
}

auto zpt::ws_message::from_stream(std::istream& _in) -> zpt::basic_message& {
    auto [_message, _op] = zpt::net::ws::read(_in);
    try {
        auto _content = zpt::json::parse_json_str(_message);
        if (_content("body")->ok() && _content("uri")->ok()) { this->__underlying = _content; }
        else { throw ::non_json_message{ _content }; }
    }
    catch (...) {
        throw ::non_json_message{ _message };
    }
    return (*this);
}

auto zpt::net::ws::read(std::istream& _stream) -> std::tuple<std::string, int> {
    // Read frame header (2 bytes)
    std::uint8_t _header[2] = { 0, 0 };
    _stream.read(reinterpret_cast<char*>(&_header), 2);

    std::uint8_t _op_code = _header[0] & 0x0F;
    bool _mask = (_header[1] & 0x80) != 0;
    std::uint64_t _len = _header[1] & 0x7F;

    // Extended payload length
    if (_len == 126) {
        std::uint8_t _ext[2] = { 0, 0 };
        _stream.read(reinterpret_cast<char*>(&_ext), 2);
        _len = (static_cast<std::uint64_t>(_ext[0]) << 8) | _ext[1];
    }
    else if (_len == 127) {
        std::uint8_t _ext[8];
        _stream.read(reinterpret_cast<char*>(&_ext), 8);
        for (int _i = 0; _i < 8; _i++) { _len = (_len << 8) | _ext[_i]; }
    }

    // Read masking key (4 raw bytes)
    std::uint8_t _mkey[4] = { 0, 0, 0, 0 };
    if (_mask) { _stream.read(reinterpret_cast<char*>(&_mkey), 4); }

    // Read payload data
    std::string _raw(static_cast<size_t>(_len), '\0');
    if (_len > 0) { _stream.read(&_raw[0], static_cast<std::streamsize>(_len)); }

    // Apply masking if needed
    if (_mask && _len > 0) {
        for (std::uint64_t _i = 0; _i < _len; _i++) {
            _raw[static_cast<size_t>(_i)] ^= _mkey[_i % 4];
        }
    }

    return std::make_tuple(std::move(_raw), _op_code);
}

auto zpt::net::ws::write(std::ostream& _stream, std::string const& _in, bool _mask) -> void {
    std::uint64_t _len = static_cast<std::uint64_t>(_in.length());
    // First byte: FIN=1 + TEXT opcode=1
    std::uint8_t _header[2];
    _header[0] = 0x80 | 0x01; // FIN + TEXT

    if (_len <= 125) {
        _header[1] = static_cast<std::uint8_t>(_len | (_mask ? 0x80 : 0));
        _stream.write(reinterpret_cast<char*>(&_header), 2);
    }
    else if (_len <= 65535) {
        _header[1] = static_cast<std::uint8_t>(126 | (_mask ? 0x80 : 0));
        _stream.write(reinterpret_cast<char*>(&_header), 2);
        std::uint16_t _ext = htons(static_cast<std::uint16_t>(_len));
        _stream.write(reinterpret_cast<char*>(&_ext), 2);
    }
    else {
        _header[1] = static_cast<std::uint8_t>(127 | (_mask ? 0x80 : 0));
        _stream.write(reinterpret_cast<char*>(&_header), 2);
        std::uint8_t _ext[8];
        for (int _i = 7; _i >= 0; _i--) {
            _ext[7 - _i] = static_cast<std::uint8_t>(_len >> (_i * 8));
        }
        _stream.write(reinterpret_cast<char*>(&_ext), 8);
    }

    // Write masking key and payload
    if (_mask) {
        std::uint8_t _mkey[4] = { 0, 0, 0, 0 };
        _stream.write(reinterpret_cast<char*>(&_mkey), 4);

        // Write masked payload
        std::string _masked;
        _masked.resize(_len);
        for (std::uint64_t _i = 0; _i < _len; _i++) {
            _masked[static_cast<size_t>(_i)] =
              _in[static_cast<size_t>(_i)] ^ static_cast<char>(_mkey[_i % 4]);
        }
        _stream.write(_masked.data(), static_cast<std::streamsize>(_masked.length()));
    }
    else { _stream.write(_in.data(), static_cast<std::streamsize>(_in.length())); }

    _stream.flush();
}

auto zpt::net::transport::websocket::upgraded_from() const -> std::string const& {
    static std::string _return{ "http" };
    return _return;
}

auto zpt::net::transport::websocket::has_capability(std::uint64_t _capability) const -> bool {
    static constexpr std::uint64_t _capabilities =
      zpt::transport_capability::PERSISTENT | zpt::transport_capability::UPGRADED;
    return (_capabilities & _capability) == _capability;
}

auto zpt::net::transport::websocket::make_request() const -> zpt::message {
    auto _to_return = zpt::allocate_message<request_type>();
    return _to_return;
}

auto zpt::net::transport::websocket::make_reply(bool _with_allocator) const -> zpt::message {
    auto _to_return =
      _with_allocator ? zpt::allocate_message<reply_type>() : zpt::make_message<reply_type>();
    return _to_return;
}

auto zpt::net::transport::websocket::make_reply(zpt::message _request) const -> zpt::message {
    auto _to_return = zpt::make_message<reply_type>(_request, true);
    return _to_return;
}

auto zpt::net::transport::websocket::process_incoming_request(zpt::stream _stream) const
  -> zpt::message {
    expect(_stream->transport() == "ws", "Stream underlying transport isn't 'websocket'");
    try {
        auto _message = zpt::allocate_message<request_type>();
        (*_stream) >> std::noskipws >> _message;
        return _message;
    }
    catch (::non_json_message const& _e) {
        auto _message = std::any_cast<zpt::message>(_stream->metadata())->clone();
        _message->body() = _e.__original;
        return _message;
    }
}

auto zpt::net::transport::websocket::process_incoming_reply(zpt::stream _stream) const
  -> zpt::message {
    return this->process_incoming_request(_stream);
}
