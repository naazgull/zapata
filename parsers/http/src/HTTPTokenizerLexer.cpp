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

#include <zapata/http/HTTPTokenizerLexer.h>

zpt::HTTPTokenizerLexer::HTTPTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::HTTPLexer(_in, _out) {}

zpt::HTTPTokenizerLexer::~HTTPTokenizerLexer() {}

auto zpt::HTTPTokenizerLexer::switchRoots(zpt::http::basic_request& _root) -> void {
    this->__root_req = &_root;
    this->begin(zpt::HTTPLexerBase::StartCondition_::INITIAL);
}

auto zpt::HTTPTokenizerLexer::switchRoots(zpt::http::basic_reply& _root) -> void {
    this->__root_rep = &_root;
    this->begin(zpt::HTTPLexerBase::StartCondition_::INITIAL);
}

auto zpt::HTTPTokenizerLexer::justLeave() -> void { this->leave(-1); }

auto zpt::HTTPTokenizerLexer::init(int _in_type) -> void {
    this->d_chunked_body = false;
    this->d_chunked.clear();
    this->__root_type = _in_type;
    switch (_in_type) {
        case 0: {
            std::string _ms(this->matched());
            zpt::performative _m = zpt::ontology::from_str(_ms);
            this->__root_req->performative(_m);
            break;
        }
        case 1: {
            break;
        }
    }
}

auto zpt::HTTPTokenizerLexer::version() -> void {
    std::string _s(this->matched());
    zpt::replace(_s, "HTTP/", "");
    switch (this->__root_type) {
        case 0: {
            this->__root_req->version(_s);
            break;
        }
        case 1: {
            this->__root_rep->version(_s);
            break;
        }
    }
}

auto zpt::HTTPTokenizerLexer::body() -> void {
    switch (this->__root_type) {
        case 0: {
            this->__root_req->body(this->matched());
            break;
        }
        case 1: {
            this->__root_rep->body(this->matched());
            break;
        }
    }
    this->setMatched("");
}

auto zpt::HTTPTokenizerLexer::url() -> void {
    switch (this->__root_type) {
        case 0: {
            this->__root_req->uri()["path"] = zpt::split(this->matched(), "/");
            this->__root_req->uri()["scheme"] = "http";
            break;
        }
        case 1: {
            break;
        }
    }
}

auto zpt::HTTPTokenizerLexer::status() -> void {
    switch (this->__root_type) {
        case 0: {
            break;
        }
        case 1: {
            int _status = 0;
            std::string _statusstr(this->matched());
            zpt::fromstr(_statusstr, &_status);
            this->__root_rep->status((zpt::http::status)_status);
            break;
        }
    }
}

auto zpt::HTTPTokenizerLexer::add() -> void {
    std::string _s(this->matched());
    zpt::trim(_s);
    if (this->__header_name.length() == 0) {
        this->__header_name.assign(_s);
        return;
    }
    switch (this->__root_type) {
        case 0: {
            this->__root_req->header(this->__header_name, _s);
            break;
        }
        case 1: {
            this->__root_rep->header(this->__header_name, _s);
            break;
        }
    }
    this->__header_name.assign("");
}
