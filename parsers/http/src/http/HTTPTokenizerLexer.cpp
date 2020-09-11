#include <zapata/http/http.h>
#include <zapata/http/HTTPTokenizerLexer.h>
#include <zapata/http/config.h>

zpt::HTTPTokenizerLexer::HTTPTokenizerLexer(std::istream& _in, std::ostream& _out)
  : zpt::HTTPLexer(_in, _out) {}

zpt::HTTPTokenizerLexer::~HTTPTokenizerLexer() {}

auto
zpt::HTTPTokenizerLexer::switchRoots(HTTPReq& _root) -> void {
    this->__root_req = &(*_root);
    this->begin(zpt::HTTPLexerBase::StartCondition_::INITIAL);
}

auto
zpt::HTTPTokenizerLexer::switchRoots(HTTPRep& _root) -> void {
    this->__root_rep = &(*_root);
    this->begin(zpt::HTTPLexerBase::StartCondition_::INITIAL);
}

auto
zpt::HTTPTokenizerLexer::justLeave() -> void {
    this->leave(-1);
}

auto
zpt::HTTPTokenizerLexer::init(zpt::http::message_type _in_type) -> void {
    this->d_chunked_body = false;
    this->d_chunked.clear();
    this->__root_type = _in_type;
    switch (_in_type) {
        case zpt::http::message_type::request: {
            std::string _ms(this->matched());
            zpt::performative _m = zpt::http::from_str(_ms);
            this->__root_req->method(_m);
            break;
        }
        case zpt::http::message_type::reply: {
            break;
        }
    }
}

auto
zpt::HTTPTokenizerLexer::version() -> void {
    std::string _s(this->matched());
    zpt::replace(_s, "HTTP/", "");
    switch (this->__root_type) {
        case zpt::http::message_type::request: {
            this->__root_req->version(_s);
            break;
        }
        case zpt::http::message_type::reply: {
            this->__root_rep->version(_s);
            break;
        }
    }
}

auto
zpt::HTTPTokenizerLexer::body() -> void {
    switch (this->__root_type) {
        case zpt::http::message_type::request: {
            this->__root_req->body(this->matched());
            break;
        }
        case zpt::http::message_type::reply: {
            this->__root_rep->body(this->matched());
            break;
        }
    }
    this->setMatched("");
}

auto
zpt::HTTPTokenizerLexer::url() -> void {
    switch (this->__root_type) {
        case zpt::http::message_type::request: {
            this->__root_req->url(this->matched());
            break;
        }
        case zpt::http::message_type::reply: {
            break;
        }
    }
}

auto
zpt::HTTPTokenizerLexer::status() -> void {
    switch (this->__root_type) {
        case zpt::http::message_type::request: {
            break;
        }
        case zpt::http::message_type::reply: {
            int _status = 0;
            std::string _statusstr(this->matched());
            zpt::fromstr(_statusstr, &_status);
            this->__root_rep->status((zpt::http::status)_status);
            break;
        }
    }
}

auto
zpt::HTTPTokenizerLexer::add() -> void {
    std::string _s(this->matched());
    zpt::trim(_s);
    if (this->__header_name.length() == 0) {
        this->__header_name.assign(_s);
        return;
    }
    switch (this->__root_type) {
        case zpt::http::message_type::request: {
            this->__root_req->header(this->__header_name, _s);
            break;
        }
        case zpt::http::message_type::reply: {
            this->__root_rep->header(this->__header_name, _s);
            break;
        }
    }
    this->__header_name.clear();
}

auto
zpt::HTTPTokenizerLexer::name() -> void {
    if (this->__param_name.length() != 0) {
        switch (this->__root_type) {
            case zpt::http::message_type::request: {
                this->__root_req->param(this->__param_name, "");
                break;
            }
            case zpt::http::message_type::reply: {
                break;
            }
        }
    }
    std::string _s(this->matched());
    zpt::trim(_s);
    this->__param_name.assign(_s);
}

auto
zpt::HTTPTokenizerLexer::value() -> void {
    std::string _s(this->matched());
    zpt::trim(_s);
    switch (this->__root_type) {
        case zpt::http::message_type::request: {
            this->__root_req->param(this->__param_name, _s);
            break;
        }
        case zpt::http::message_type::reply: {
            break;
        }
    }
    this->__param_name.clear();
}
