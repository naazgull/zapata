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

#include <algorithm>
#include <zapata/http/Re2cHTTPLexer.h>

namespace {
constexpr std::size_t INITIAL_BUFFER_SIZE = 4096;
}

zpt::Re2cHTTPLexer::Re2cHTTPLexer(std::istream& _in, std::ostream& _out)
  : __in{ &_in }
  , __out{ &_out } {
    this->resetBuffer();
}

zpt::Re2cHTTPLexer::~Re2cHTTPLexer() {}

auto zpt::Re2cHTTPLexer::matched() const -> std::string const& { return this->__matched; }

auto zpt::Re2cHTTPLexer::setMatched(std::string const& _text) -> void { this->__matched = _text; }

auto zpt::Re2cHTTPLexer::more() -> void { this->__more = true; }

auto zpt::Re2cHTTPLexer::startCondition() const -> zpt::re2c_cond { return this->__condition; }

auto zpt::Re2cHTTPLexer::begin(zpt::re2c_cond _condition) -> void { this->__condition = _condition; }

auto zpt::Re2cHTTPLexer::lineNr() const -> std::size_t { return this->__line_nr; }

auto zpt::Re2cHTTPLexer::resetBuffer() -> void {
    this->__buffer.assign(INITIAL_BUFFER_SIZE, '\0');
    this->__cursor = this->__buffer.data();
    this->__limit = this->__buffer.data();
    this->__marker = this->__buffer.data();
    this->__token_start = this->__buffer.data();
    this->__data_limit = this->__buffer.data();
    this->__eof = false;
}

auto zpt::Re2cHTTPLexer::switchStreams(std::istream& _in, std::ostream& _out) -> void {
    this->__in = &_in;
    this->__out = &_out;
    this->resetBuffer();
    this->__condition = zpt::re2c_cond::INITIAL;
    this->__matched.clear();
    this->__more = false;
    this->__left = false;
    this->__leave_value = 0;
    this->__line_nr = 1;
    this->d_content_length = 0;
    this->d_chunked_length = 0;
    this->d_chunked_body = false;
    this->d_chunked_trailer.clear();
    this->d_chunked.clear();
}

auto zpt::Re2cHTTPLexer::fill(std::size_t _need) -> bool {
    while (static_cast<std::size_t>(this->__limit - this->__cursor) < _need) {
        if (this->__eof) { return false; }

        // Shift buffer contents left, discarding everything before the
        // in-flight token, freeing space at the end for new data.
        std::size_t _shift = static_cast<std::size_t>(this->__token_start - this->__buffer.data());
        std::size_t _used = static_cast<std::size_t>(this->__limit - this->__token_start);
        if (_shift > 0) {
            std::copy(this->__token_start, this->__limit, this->__buffer.data());
            this->__cursor -= _shift;
            this->__marker -= _shift;
            this->__limit -= _shift;
            this->__token_start = this->__buffer.data();
        }

        // Grow the buffer if there still isn't enough room for `_need` more
        // bytes beyond what's already buffered (e.g. an in-flight token
        // longer than the current buffer size). Offsets are captured as
        // integers before resize(), since resize() may reallocate and
        // invalidate the old pointers.
        if (this->__buffer.size() < _used + _need) {
            std::size_t _cursor_off = static_cast<std::size_t>(this->__cursor - this->__token_start);
            std::size_t _marker_off = static_cast<std::size_t>(this->__marker - this->__token_start);

            std::size_t _new_size = this->__buffer.size();
            while (_new_size < _used + _need) { _new_size *= 2; }
            this->__buffer.resize(_new_size, '\0');

            this->__token_start = this->__buffer.data();
            this->__cursor = this->__token_start + _cursor_off;
            this->__marker = this->__token_start + _marker_off;
            this->__limit = this->__token_start + _used;
        }

        // Request only the shortfall actually needed right now, never the
        // whole remaining buffer. std::istream::read() blocks until it gets
        // every byte it was asked for (or EOF) - over a live socket stream,
        // asking for more than the peer has sent hangs forever, since the
        // peer is waiting for our response before sending anything else.
        std::size_t _available = static_cast<std::size_t>(this->__limit - this->__cursor);
        std::size_t _shortfall = _need - _available;
        this->__in->read(this->__limit, static_cast<std::streamsize>(_shortfall));
        std::streamsize _got = this->__in->gcount();
        this->__in->clear(this->__in->rdstate() & ~std::ios_base::failbit);

        if (_got <= 0) {
            this->__eof = true;
            return false;
        }

        this->__limit += _got;
        this->__data_limit = this->__limit;
    }
    return true;
}

auto zpt::Re2cHTTPLexer::yyfill(std::size_t _need) -> void {
    if (this->fill(_need)) { return; }

    // True stream EOF reached with fewer than `_need` real bytes available.
    // Zero-pad the shortfall so the DFA's bounds check succeeds and a message
    // ending exactly at EOF can still match (e.g. a final response with no
    // body on a connection that then closes). __data_limit keeps marking
    // where the real bytes end, so syncBackToStream() never tries to push
    // these synthetic bytes back onto the istream.
    std::size_t _available = static_cast<std::size_t>(this->__limit - this->__cursor);
    std::size_t _shortfall = _need - _available;
    std::size_t _used = static_cast<std::size_t>(this->__limit - this->__token_start);

    if (this->__buffer.size() < _used + _shortfall) {
        std::size_t _cursor_off = static_cast<std::size_t>(this->__cursor - this->__token_start);
        std::size_t _marker_off = static_cast<std::size_t>(this->__marker - this->__token_start);
        std::size_t _data_limit_off = static_cast<std::size_t>(this->__data_limit - this->__token_start);

        std::size_t _new_size = this->__buffer.size();
        while (_new_size < _used + _shortfall) { _new_size *= 2; }
        this->__buffer.resize(_new_size, '\0');

        this->__token_start = this->__buffer.data();
        this->__cursor = this->__token_start + _cursor_off;
        this->__marker = this->__token_start + _marker_off;
        this->__data_limit = this->__token_start + _data_limit_off;
        this->__limit = this->__token_start + _used;
    }
    std::fill(this->__limit, this->__limit + _shortfall, '\0');
    this->__limit += _shortfall;
}

auto zpt::Re2cHTTPLexer::captureMatch() -> void {
    this->__line_nr += static_cast<std::size_t>(std::count(this->__token_start, this->__cursor, '\n'));
    if (this->__more) {
        this->__matched.append(this->__token_start, this->__cursor);
        this->__more = false;
    }
    else { this->__matched.assign(this->__token_start, this->__cursor); }
}

auto zpt::Re2cHTTPLexer::readRaw(std::size_t _n) -> std::string {
    std::string _out;
    _out.reserve(_n);

    while (_out.size() < _n) {
        std::size_t _available = static_cast<std::size_t>(this->__limit - this->__cursor);
        if (_available == 0) {
            this->__token_start = this->__cursor;
            if (!this->fill(_n - _out.size())) {
                // EOF mid-body: append whatever's left and stop, matching the
                // historical behavior of flexc++'s get_() returning AT_EOF
                // (no explicit validation existed there either).
                break;
            }
            _available = static_cast<std::size_t>(this->__limit - this->__cursor);
        }
        std::size_t _take = std::min(_available, _n - _out.size());
        _out.append(this->__cursor, _take);
        this->__cursor += _take;
    }

    this->__token_start = this->__cursor;
    return _out;
}

auto zpt::Re2cHTTPLexer::syncBackToStream() -> void {
    // Only give back real bytes (up to __data_limit) - never the synthetic
    // zero padding that yyfill() may have appended past true stream EOF.
    char* _real_limit = std::min(this->__limit, this->__data_limit);
    if (_real_limit > this->__cursor) {
        // Bytes were fetched into the buffer via fill() but never consumed by
        // the grammar (e.g. the DFA over-read past the message boundary while
        // determining a longest match, or fill() pulled in more than one
        // message's worth at once). Give them back so the istream's read
        // position ends up exactly at the first unconsumed byte.
        for (char* _p = _real_limit; _p > this->__cursor;) {
            --_p;
            this->__in->putback(*_p);
        }
    }
    this->resetBuffer();
}

auto zpt::Re2cHTTPLexer::leave(int _retValue) -> void {
    this->__left = true;
    this->__leave_value = _retValue;
}

auto zpt::Re2cHTTPLexer::lex() -> int {
    while (true) {
        if (this->__left) { return this->__leave_value; }

        this->__token_start = this->__cursor;
        int _token = 0;
        switch (this->__condition) {
            case zpt::re2c_cond::INITIAL: _token = this->lexInitial(); break;
            case zpt::re2c_cond::request: _token = this->lexRequest(); break;
            case zpt::re2c_cond::reply: _token = this->lexReply(); break;
            case zpt::re2c_cond::headers: _token = this->lexHeaders(); break;
            case zpt::re2c_cond::headerval: _token = this->lexHeaderval(); break;
            case zpt::re2c_cond::statustext: _token = this->lexStatustext(); break;
            case zpt::re2c_cond::contentlengthval: _token = this->lexContentLengthVal(); break;
            case zpt::re2c_cond::transferencodingval: _token = this->lexTransferEncodingVal(); break;
            case zpt::re2c_cond::trailerval: _token = this->lexTrailerVal(); break;
            case zpt::re2c_cond::plain_body: _token = this->lexPlainBody(); break;
            case zpt::re2c_cond::chunked_body: _token = this->lexChunkedBody(); break;
        }

        if (this->__left) { return this->__leave_value; }
        if (_token == 0) { continue; } // rule matched but produced no token (e.g. whitespace)
        return _token;
    }
}
