/*
The MIT License (MIT)

Copyright (c) 2014 n@zgul <naazgull@dfz.pt>

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

#include <zapata/rest/WebSocket.h>
#include <zapata/text/convert.h>
#include <zapata/text/manip.h>

bool zapata::ws::handshake(zapata::socketstream& _s) {
	string _key;
	string _line;
	do {
		getline(_s, _line);
		zapata::trim(_line);
		if (_line.find("Sec-WebSocket-Key:") != string::npos) {
			_key.assign(_line.substr(19));
		}
	}
	while (_line != "");

	_key.insert(_key.length(), "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	_key.assign(zapata::hash::SHA1(_key));
	zapata::base64::encode(_key);

	_s << 
		"HTTP/1.1 101 Switching Protocols" << CRLF <<
		"Upgrade: websocket" << CRLF << 
		"Connection: Upgrade" << CRLF <<
		"Sec-WebSocket-Accept: " << _key << CRLF <<
		CRLF << flush;
	return true;
}

bool zapata::ws::read(zapata::socketstream& _s, string& _out, int* _op_code) {
	unsigned char _hdr;
	_s >> noskipws >> _hdr;

	bool _fin = _hdr & 0x80;
	*_op_code = _hdr & 0x0F;
	_s >> noskipws >> _hdr;
	bool _mask = _hdr & 0x80;
	string _masking;
	string _masked;

	int _len = _hdr & 0x7F;
	if (_len == 126) {
		_s >> noskipws >> _hdr;
		_len = (int) _hdr;
		_len <<= 8;
		_s >> noskipws >> _hdr;
		_len += (int) _hdr;
	}
	else if (_len == 127) {
		_s >> noskipws >> _hdr;
		_len = (int) _hdr;
		for (int _i = 0; _i < 7; _i++) {
			_len <<= 8;
			_s >> noskipws >> _hdr;
			_len += (int) _hdr;
		}
	}

	if (_mask) {
		for (int _i = 0; _i < 4; _i++) {
			_s >> noskipws >> _hdr;
			_masking.push_back((char) _hdr);
		}
	}


	for (int _i = 0; _i != _len; _i++) {
		_s >> noskipws >> _hdr;
		_masked.push_back((char) _hdr);
	}

	if (_mask) {
		for (size_t _i = 0; _i < _masked.length(); _i++) {
			_out.push_back(_masked[_i] ^ _masking[_i % 4]);
		}
	}
	else {
		_out.assign(_masked);
	}

	return _fin;
}

bool zapata::ws::write(zapata::socketstream& _s, string _in){
	int _len = _in.length();

	_s << (unsigned char) 0x81;
	if (_len > 125) {
		_s << (unsigned char) 0xFE;
		_s << ((unsigned char) (_len >> 8));
		_s << ((unsigned char) (_len & 0xFF));
	}
	else {
		_s << (unsigned char) (0x80 | ((unsigned char) _len));
	}
	for (int _i = 0; _i != 4; _i++) {
		_s << (unsigned char) 0x00;
	}

	_s << _in << flush;

	return true;

}
