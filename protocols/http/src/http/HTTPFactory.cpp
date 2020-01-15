#include <arpa/inet.h>
#include <chrono>
#include <ifaddrs.h>
#include <iomanip>
#include <netinet/in.h>
#include <ossp/uuid++.hh>
#include <zapata/protocols/http/HTTPFactory.h>

#define HTTP_REP 0
#define HTTP_REQ 1

#define within(num) (int)((float)(num)*random() / (RAND_MAX + 1.0))

zpt::HTTP::HTTP(zpt::socketstream_ptr _underlying, zpt::json _options)
  : zpt::Channel(std::string(_underlying->ssl() ? "https://" : "http://") +
                   std::string(_underlying->host() != "" ? _underlying->host() : "*") +
                   std::string(":") + std::to_string(_underlying->port()),
                 _options)
  , __underlying(_underlying)
  , __state(-1) {}

zpt::HTTP::~HTTP() {}

auto
zpt::HTTP::underlying() -> zpt::socketstream_ptr {
    return this->__underlying;
}

auto
zpt::HTTP::socket() -> zmq::socket_ptr {
    return zmq::socket_ptr(nullptr);
}

auto
zpt::HTTP::in() -> zmq::socket_ptr {
    return zmq::socket_ptr(nullptr);
}

auto
zpt::HTTP::out() -> zmq::socket_ptr {
    return zmq::socket_ptr(nullptr);
}

auto
zpt::HTTP::fd() -> int {
    return this->__underlying->buffer().get_socket();
}

auto
zpt::HTTP::close() -> void {
    this->__underlying->close();
}

auto
zpt::HTTP::available() -> bool {
    char _c;
    int _read = ::recv(this->fd(), &_c, 1, MSG_PEEK);
    return _read > 0;
}

auto
zpt::HTTP::in_mtx() -> std::mutex& {
    return this->__mtx;
}

auto
zpt::HTTP::out_mtx() -> std::mutex& {
    return this->__mtx;
}

auto
zpt::HTTP::type() -> short int {
    return ZMQ_HTTP_RAW;
}

auto
zpt::HTTP::protocol() -> std::string {
    return "HTTP/1.1";
}

auto
zpt::HTTP::send(zpt::json _envelope) -> zpt::json {
    zpt::performative _performative = (zpt::performative)((int)_envelope["performative"]);

    expect((_performative == zpt::ev::Reply && this->__state == HTTP_REQ) ||
             (_performative != zpt::ev::Reply && this->__state != HTTP_REQ),
           "HTTP socket state doesn't allow you to send a reply without having "
           "received a request",
           400,
           1201);
    expect(_envelope["resource"]->ok(), "'resource' attribute is required", 412, 0);

    zpt::json _uri = zpt::uri::parse(_envelope["resource"]);
    _envelope << "resource" << _uri["path"] << "protocol" << this->protocol() << "params"
              << ((_envelope["params"]->is_object() ? _envelope["params"] : zpt::undefined) +
                  _uri["query"]);

    if (_performative == zpt::ev::Reply) {
        expect(_envelope["status"]->ok(), "'status' attribute is required", 412, 0);
        _envelope["headers"] << "X-Status" << _envelope["status"];
    }
    if (_envelope["payload"]["assertion_failed"]->ok() && _envelope["payload"]["code"]->ok()) {
        _envelope["headers"] << "X-Error" << _envelope["payload"]["code"];
    }
    try {
        {
            std::lock_guard<std::mutex> _lock(this->out_mtx());
            std::string _message;
            if (_performative == zpt::ev::Reply) {
                _message.assign(std::string(zpt::internal2http_rep(_envelope)));
                this->__state = HTTP_REP;
            }
            else {
                _message.assign(std::string(zpt::internal2http_req(
                  _envelope,
                  this->__underlying->host() +
                    ((this->__underlying->ssl() && this->__underlying->port() == 443) ||
                         (!this->__underlying->ssl() && this->__underlying->port() == 80)
                       ? std::string("")
                       : std::string(":") + std::to_string(this->__underlying->port())))));
                this->__state = HTTP_REQ;
                this->__cid = std::string(_envelope["channel"]);
                this->__resource = std::string(_envelope["resource"]);
            }
            (*this->__underlying) << _message << std::flush;
            expect(!this->__underlying->is_error(), this->__underlying->error_string(), 503, 0);
        }

        ztrace(std::string("> ") + zpt::ev::to_str(_performative) + std::string(" ") +
               _envelope["resource"]->str() +
               (_performative == zpt::ev::Reply
                  ? std::string(" ") + std::string(_envelope["status"])
                  : std::string("")));
        zverbose(zpt::ev::pretty(_envelope));
    }
    catch (std::ios_base::failure& _e) {
    }
    return zpt::undefined;
}

auto
zpt::HTTP::recv() -> zpt::json {
    zpt::json _in;
    try {
        {
            std::lock_guard<std::mutex> _lock(this->in_mtx());
            if (this->__state != HTTP_REQ) {
                zpt::http::req _request;
                (*this->__underlying) >> _request;
                expect(!this->__underlying->is_error(), this->__underlying->error_string(), 503, 0);
                this->__state = HTTP_REQ;
                try {
                    _in = zpt::http2internal(_request);
                }
                catch (zpt::SyntaxErrorException& _e) {
                    zlog(std::string("error while parsing HTTP message body: syntax "
                                     "error exception"),
                         zpt::error);
                    return { "protocol",
                             this->protocol(),
                             "status",
                             400,
                             "payload",
                             { "text",
                               _e.what(),
                               "assertion_failed",
                               _e.what(),
                               "cod"
                               "e",
                               1062 } };
                }
                this->__cid = std::string(_in["channel"]);
            }
            else {
                zpt::http::rep _reply;
                (*this->__underlying) >> _reply;
                expect(!this->__underlying->is_error(), this->__underlying->error_string(), 503, 0);
                this->__state = HTTP_REP;
                if (_reply->header("X-Cid") == "") {
                    _reply->header("X-Cid", this->__cid);
                }
                if (_reply->header("X-Resource") == "") {
                    _reply->header("X-Resource", this->__resource);
                }
                try {
                    _in = zpt::http2internal(_reply);
                }
                catch (zpt::SyntaxErrorException& _e) {
                    zlog(std::string("error while parsing HTTP message body: syntax "
                                     "error exception"),
                         zpt::error);
                    return { "protocol",
                             this->protocol(),
                             "status",
                             400,
                             "payload",
                             { "text",
                               _e.what(),
                               "assertion_failed",
                               _e.what(),
                               "cod"
                               "e",
                               1062 } };
                }
            }
        }
    }
    catch (zpt::SyntaxErrorException& _e) {
        zlog(std::string("error while parsing HTTP request: syntax error exception"), zpt::error);
        return { "protocol", this->protocol(),
                 "error",    true,
                 "status",   400,
                 "payload",  { "text", _e.what(), "assertion_failed", _e.what(), "code", 1062 } };
    }
    _in << "protocol" << this->protocol();
    ztrace(std::string("< ") + zpt::ev::to_str(zpt::performative(int(_in["performative"]))) +
           std::string(" ") + _in["resource"]->str() +
           (zpt::performative(int(_in["performative"])) == zpt::ev::Reply
              ? std::string(" ") + std::string(_in["status"])
              : std::string("")));
    zverbose(zpt::ev::pretty(_in));
    return _in;
}

zpt::HTTPFactory::HTTPFactory()
  : zpt::ChannelFactory() {}

zpt::HTTPFactory::~HTTPFactory() {}

auto
zpt::HTTPFactory::produce(zpt::json _options) -> zpt::socket {
    zpt::HTTP* _http = new zpt::HTTP(_options);
    return zpt::socket(_http);
}

auto
zpt::HTTPFactory::is_reusable(std::string _type) -> bool {
    return false;
}

auto
zpt::HTTPFactory::clean(zpt::socket _socket) -> bool {
    return true;
}

extern "C" void
_zpt_plugin_load_() {
    zpt::ev::emitter_factory _emitter = zpt::emitter();
    zpt::channel_factory _factory(new zpt::HTTPFactory());
    _emitter->channel({ { "http", _factory }, { "https", _factory } });

    if (_options["http"]->ok() && _options["http"]["bind"]->ok()) {
        std::thread _http([]() -> void {
            zpt::json _uri = zpt::uri::parse(std::string(_options["http"]["bind"]));
            if (!_uri["port"]->ok()) {
                _uri << "port" << 80;
            }

            zpt::serversocketstream_ptr _ss(new zpt::serversocketstream());
            if (!_ss->bind((uint)_uri["port"])) {
                zlog(std::string("couldn't bind HTTP listener to ") +
                       std::string(_options["http"]["bind"]),
                     zpt::alert);
                return;
            }
            std::string _scheme = std::string(_uri["scheme"]);
            std::transform(std::begin(_scheme), std::end(_scheme), std::begin(_scheme), ::toupper);
            zlog(std::string("binding ") + _scheme + std::string("/1.1 listener to ") +
                   std::string(_uri["scheme"]) + std::string("://") + std::string(_uri["domain"]) +
                   std::string(":") + std::string(_uri["port"]),
                 zpt::info);

            bool _is_ssl = _uri["scheme"] == zpt::json::string("https");
            for (; true;) {
                int _fd = -1;
                _ss->accept(&_fd);
                if (_fd >= 0) {
                    zpt::socketstream_ptr _cs(_fd, _is_ssl);
                    this->__poll->poll(this->__poll->add(new HTTP(_cs, _options)));
                }
                else {
                    zlog("please, check your soft and hard limits for allowed number of "
                         "opened file descriptors,",
                         zpt::warning);
                    zlog("unable to accept HTTP sockets, going to disable HTTP server.",
                         zpt::emergency);
                    _ss->close();
                    return;
                }
            }
        });
        _http.detach();
    }
}
