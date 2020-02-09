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

#include <zapata/net/transport/websocket.h>
#include <zapata/base.h>

auto
zpt::net::ws::handshake(zpt::stream& _stream) -> void {
    std::string _key;
    std::string _line;
    do {
        std::getline(*_stream, _line);
        zpt::trim(_line);
        if (_line.find("Sec-WebSocket-Key:") != std::string::npos) {
            _key.assign(_line.substr(19));
        }
    } while (_line != "");

    _key.insert(_key.length(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    _key.assign(zpt::hash::SHA1(_key));
    zpt::base64::encode(_key);

    _stream << "HTTP/1.1 101 Switching Protocols" << CRLF << "Upgrade: websocket" << CRLF
            << "Connection: Upgrade" << CRLF << "Sec-WebSocket-Accept: " << _key << CRLF << CRLF
            << std::flush;
}

auto
zpt::net::ws::read(zpt::stream& _stream) -> std::tuple<std::string, int> {
    std::string _out;
    unsigned char _hdr;
    _stream >> std::noskipws >> _hdr;

    int _op_code = _hdr & 0x0F;
    _stream >> std::noskipws >> _hdr;
    bool _mask = _hdr & 0x80;
    std::string _masking;
    std::string _masked;

    int _len = _hdr & 0x7F;
    if (_len == 126) {
        _stream >> std::noskipws >> _hdr;
        _len = (int)_hdr;
        _len <<= 8;
        _stream >> std::noskipws >> _hdr;
        _len += (int)_hdr;
    }
    else if (_len == 127) {
        _stream >> std::noskipws >> _hdr;
        _len = (int)_hdr;
        for (int _i = 0; _i < 7; _i++) {
            _len <<= 8;
            _stream >> std::noskipws >> _hdr;
            _len += (int)_hdr;
        }
    }

    if (_mask) {
        for (int _i = 0; _i < 4; _i++) {
            _stream >> std::noskipws >> _hdr;
            _masking.push_back((char)_hdr);
        }
    }

    for (int _i = 0; _i != _len; _i++) {
        _stream >> std::noskipws >> _hdr;
        _masked.push_back((char)_hdr);
    }

    if (_mask) {
        for (size_t _i = 0; _i < _masked.length(); _i++) {
            _out.push_back(_masked[_i] ^ _masking[_i % 4]);
        }
    }
    else {
        _out.assign(_masked);
    }

    return std::make_tuple(_out, _op_code);
}

auto
zpt::net::ws::write(zpt::stream& _stream, std::string _in) -> void {
    int _len = _in.length();

    _stream << (unsigned char)0x81;
    if (_len > 125) {
        _stream << (unsigned char)0xFE;
        _stream << ((unsigned char)(_len >> 8));
        _stream << ((unsigned char)(_len & 0xFF));
    }
    else {
        _stream << (unsigned char)(0x80 | ((unsigned char)_len));
    }
    for (int _i = 0; _i != 4; _i++) {
        _stream << (unsigned char)0x00;
    }

    _stream << _in << std::flush;
}

auto
zpt::net::transport::websocket::receive(zpt::message& _message) -> void {
    auto [_body, _] = zpt::net::ws::read(_message->stream());
    if (_body.length() != 0) {
        std::istringstream _iss;
        _iss.str(_body);
        zpt::json _content;
        try {
            _iss >> _content;
            _message->received() = _content;
        }
        catch (...) {
        }
        _message->keep_alive() = true;
    }
}

auto
zpt::net::transport::websocket::send(zpt::message& _message) -> void {
    if (_message->to_send()->ok()) {
        zpt::net::ws::write(_message->stream(), _message->to_send());
    }
}
