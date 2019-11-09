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
#include <zapata/http/config.h>
#include <zapata/http/http.h>

zpt::HTTPReq&
zpt::fromhttpstr(string& _in, zpt::HTTPReq& _out) {
    istringstream _ss;
    _ss.str(_in);
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_ss);
    _p.parse();
    return _out;
}

zpt::HTTPRep&
zpt::fromhttpstr(string& _in, zpt::HTTPRep& _out) {
    istringstream _ss;
    _ss.str(_in);
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_ss);
    _p.parse();
    return _out;
}

zpt::HTTPReq&
zpt::fromhttpfile(ifstream& _in, zpt::HTTPReq& _out) {
    if (_in.is_open()) {
        zpt::HTTPParser _p;
        _p.switchRoots(_out);
        _p.switchStreams(_in);
        _p.parse();
    }
    return _out;
}

zpt::HTTPRep&
zpt::fromhttpfile(ifstream& _in, zpt::HTTPRep& _out) {
    if (_in.is_open()) {
        zpt::HTTPParser _p;
        _p.switchRoots(_out);
        _p.switchStreams(_in);
        _p.parse();
    }
    return _out;
}

zpt::HTTPReq&
zpt::fromhttpstream(istream& _in, zpt::HTTPReq& _out) {
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_in);
    _p.parse();
    return _out;
}

zpt::HTTPRep&
zpt::fromhttpstream(istream& _in, zpt::HTTPRep& _out) {
    zpt::HTTPParser _p;
    _p.switchRoots(_out);
    _p.switchStreams(_in);
    _p.parse();
    return _out;
}

void
zpt::tostr(string& _out, HTTPRep& _in) {
    _in->stringify(_out);
}

void
zpt::tostr(string& _out, HTTPReq& _in) {
    _in->stringify(_out);
}

void
zpt::tostr(ostream& _out, HTTPRep& _in) {
    _in->stringify(_out);
    _out << std::flush;
}

void
zpt::tostr(ostream& _out, HTTPReq& _in) {
    _in->stringify(_out);
    _out << std::flush;
}

extern "C" auto
zpt_http() -> int {
    return 1;
}
