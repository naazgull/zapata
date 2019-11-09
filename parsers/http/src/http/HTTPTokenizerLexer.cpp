/* Author: n@zgul <n@zgul.me> */

#include <zapata/events.h>
#include <zapata/http/http.h>
#include <zapata/http/HTTPTokenizerLexer.h>
#include <zapata/http/config.h>

zpt::HTTPTokenizerLexer::HTTPTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::HTTPLexer(_in, _out) {}

zpt::HTTPTokenizerLexer::~HTTPTokenizerLexer() {}

void
zpt::HTTPTokenizerLexer::switchRoots(HTTPReq& _root) {
    this->__root_req = _root.get();
}

void
zpt::HTTPTokenizerLexer::switchRoots(HTTPRep& _root) {
    this->__root_rep = _root.get();
}

void
zpt::HTTPTokenizerLexer::justLeave() {
    this->leave(-1);
}

void
zpt::HTTPTokenizerLexer::init(zpt::HTTPType _in_type) {
    this->d_chunked_body = false;
    this->d_chunked.clear();
    this->__root_type = _in_type;
    switch (_in_type) {
        case zpt::HTTPRequest: {
            std::string _ms(this->matched());
            zpt::performative _m = zpt::http::from_str(_ms);
            this->__root_req->method(_m);
            break;
        }
        case zpt::HTTPReply: {
            break;
        }
    }
}

void
zpt::HTTPTokenizerLexer::body() {
    switch (this->__root_type) {
        case zpt::HTTPRequest: {
            this->__root_req->body(this->matched());
            break;
        }
        case zpt::HTTPReply: {
            this->__root_rep->body(this->matched());
            break;
        }
    }
}

void
zpt::HTTPTokenizerLexer::url() {
    switch (this->__root_type) {
        case zpt::HTTPRequest: {
            this->__root_req->url(this->matched());
            break;
        }
        case zpt::HTTPReply: {
            break;
        }
    }
}

void
zpt::HTTPTokenizerLexer::status() {
    switch (this->__root_type) {
        case zpt::HTTPRequest: {
            break;
        }
        case zpt::HTTPReply: {
            int _status = 0;
            std::string _statusstr(this->matched());
            zpt::fromstr(_statusstr, &_status);
            this->__root_rep->status((zpt::HTTPStatus)_status);
            break;
        }
    }
}

void
zpt::HTTPTokenizerLexer::add() {
    std::string _s(this->matched());
    zpt::trim(_s);
    if (this->__header_name.length() == 0) {
        this->__header_name.assign(_s);
        return;
    }
    switch (this->__root_type) {
        case zpt::HTTPRequest: {
            this->__root_req->header(this->__header_name, _s);
            break;
        }
        case zpt::HTTPReply: {
            this->__root_rep->header(this->__header_name, _s);
            break;
        }
    }
    this->__header_name.clear();
}

void
zpt::HTTPTokenizerLexer::name() {
    std::string _s(this->matched());
    zpt::trim(_s);
    this->__param_name.assign(_s);
}

void
zpt::HTTPTokenizerLexer::value() {
    std::string _s(this->matched());
    zpt::trim(_s);
    switch (this->__root_type) {
        case zpt::HTTPRequest: {
            this->__root_req->param(this->__param_name, _s);
            break;
        }
        case zpt::HTTPReply: {
            break;
        }
    }
    this->__param_name.clear();
}
