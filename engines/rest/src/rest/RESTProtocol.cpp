#include <zapata/rest/RESTProtocol.h>

zpt::RESTOntology::RESTOntology() {}
zpt::RESTOntology::~RESTOntology() {}

auto
zpt::RESTOntology::pretty(zpt::json _envelope) -> std::string {}

auto
zpt::RESTOntology::compose_reply(zpt::performative _method,
                                 std::string _resource,
                                 zpt::json _payload,
                                 zpt::json _headers) -> zpt::json {}

auto
zpt::RESTOntology::compose_request(zpt::performative _method,
                                   std::string _resource,
                                   zpt::json _payload,
                                   zpt::json _headers) -> zpt::json {}

auto
zpt::RESTOntology::extract_from_reply(zpt::json _envelope)
  -> std::tuple<zpt::performative, std::string, zpt::json, zpt::json> {}

auto
zpt::RESTOntology::extract_from_request(zpt::json _envelope)
  -> std::tuple<zpt::performative, std::string, zpt::json, zpt::json> {}

zpt::RESTListener::RESTListener(std::string _regex)
  : zpt::EventListener(_regex) {}

zpt::RESTListener::~RESTListener() {}

auto
zpt::RESTListener::get(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter)
  -> void {
    expect(false, "Performative is not accepted for the given resource", 405, 0);
}

auto
zpt::RESTListener::put(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter)
  -> void {
    expect(false, "Performative is not accepted for the given resource", 405, 0);
}

auto
zpt::RESTListener::post(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter)
  -> void {
    expect(false, "Performative is not accepted for the given resource", 405, 0);
}

auto
zpt::RESTListener::del(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter)
  -> void {
    expect(false, "Performative is not accepted for the given resource", 405, 0);
}

auto
zpt::RESTListener::head(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter)
  -> void {
    expect(false, "Performative is not accepted for the given resource", 405, 0);
}

auto
zpt::RESTListener::options(std::string _resource,
                           zpt::json _envelope,
                           zpt::EventEmitterPtr _emitter) -> void {
    if (_envelope["headers"]["Origin"]->ok()) {
        _emitter->reply(_envelope,
                        { "status",
                          413,
                          "headers",
                          zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) });
    }
    string _origin = _envelope["headers"]["Origin"];
    _emitter->reply(_envelope,
                    { "status",
                      200,
                      "headers",
                      (zpt::ev::init_reply(std::string(_envelope["headers"]["X-Cid"])) +
                       zpt::json({ "Access-Control-Allow-Origin",
                                   _envelope["headers"]["Origin"],
                                   "Access-Control-Allow-Methods",
                                   "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY",
                                   "Access-Control-Allow-Headers",
                                   ACCESS_CONTROL_HEADERS,
                                   "Access-Control-Expose-Headers",
                                   ACCESS_CONTROL_HEADERS,
                                   "Access-Control-Max-Age",
                                   "1728000" })) });
}

auto
zpt::RESTListener::patch(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter)
  -> void {
    expect(false, "Performative is not accepted for the given resource", 405, 0);
}

auto
zpt::RESTListener::reply(std::string _resource, zpt::json _envelope, zpt::EventEmitterPtr _emitter)
  -> void {}

auto
zpt::rest::init_request(std::string _cid) -> zpt::json {
    time_t _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    char _buffer_expires[80];
    strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

    zpt::json _return(zpt::json{ "Accept",
                                 "application/json",
                                 "Accept-Charset",
                                 "utf-8",
                                 "Date",
                                 string(_buffer_date),
                                 "User-Agent",
                                 "zapata RESTful server" });
    if (_cid != "") {
        _return << "X-Cid" << _cid;
    }
    else {
        _return << "X-Cid" << zpt::generate::r_uuid();
    }
    if (zpt::ev::__default_authorization != nullptr) {
        _return << "Authorization" << std::string(zpt::ev::__default_authorization->data());
    }
    return _return;
}

auto
zpt::rest::init_reply(std::string _uuid, zpt::json _request) -> zpt::json {
    time_t _rawtime = time(nullptr);
    struct tm _ptm;
    char _buffer_date[80];
    localtime_r(&_rawtime, &_ptm);
    strftime(_buffer_date, 80, "%a, %d %b %Y %X %Z", &_ptm);

    char _buffer_expires[80];
    _ptm.tm_hour += 1;
    strftime(_buffer_expires, 80, "%a, %d %b %Y %X %Z", &_ptm);

    zpt::json _return(
      zpt::json{ "Server",
                 "zapata RESTful server",
                 "Vary",
                 "Accept-Language,Accept-Encoding,X-Access-Token,Authorization,E-Tag",
                 "Date",
                 std::string(_buffer_date) });
    if (_uuid != "") {
        _return << "X-Cid" << _uuid;
    }
    if (_request["headers"]["Connection"]->ok()) {
        _return << "Connection" << _request["headers"]["Connection"];
    }

    if (_request->ok()) {
        if (_request["headers"]["X-No-Redirection"]->ok() &&
            std::string(_request["headers"]["X-No-Redirection"]) == "true") {
            if (((int)_return["status"]) > 299 && ((int)_return["status"]) < 400) {
                _return << "status" << 200;
                _return["headers"] << "X-Redirect-To" << _return["headers"]["Location"];
            }
        }
        if (_request["headers"]["Origin"]->ok()) {
            _return << "Access-Control-Allow-Origin" << _request["headers"]["Origin"]
                    << "Access-Control-Allow-Credentials"
                    << "true"
                    << "Access-Control-Allow-Methods"
                    << "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY"
                    << "Access-Control-Allow-Headers" << REST_ACCESS_CONTROL_HEADERS
                    << "Access-Control-Expose-Headers" << REST_ACCESS_CONTROL_HEADERS
                    << "Access-Control-Max-Age"
                    << "1728000";
        }
    }

    return _return;
}

auto
zpt::rest::pretty(zpt::json _envelope) -> std::string {
    std::string _protocol =
      (_envelope["protocol"]->ok() ? std::string(_envelope["protocol"]) : "ZPT/1.0");
    std::string _ret("\n");
    if (_envelope["status"]->ok()) {
        std::string _color;
        if (int(_envelope["status"]) < 300) {
            _color.assign("32");
        }
        else if (int(_envelope["status"]) < 400) {
            _color.assign("33");
        }
        else {
            _color.assign("31");
        }
        _ret += _protocol + std::string(" \033[1;") + _color + std::string("m") +
                std::string(zpt::status_names[int(_envelope["status"])]) + std::string("\033[0m\n");
    }
    else {
        _ret += zpt::ev::to_str(zpt::performative(int(_envelope["performative"]))) +
                std::string(" \033[1;35m") + std::string(_envelope["resource"]) +
                std::string("\033[0m");
        if (_envelope["params"]->is_object()) {
            _ret += std::string("?");
            bool _first = true;
            for (auto _param : _envelope["params"]->obj()) {
                if (!_first) {
                    _ret += std::string("&");
                }
                _first = false;
                _ret += _param.first + std::string("=") + std::string(_param.second);
            }
        }
        _ret += std::string(" ") + _protocol + std::string("\n");
    }
    if (_envelope["headers"]->ok()) {
        for (auto _header : _envelope["headers"]->obj()) {
            _ret +=
              _header.first + std::string(": ") + std::string(_header.second) + std::string("\n");
        }
    }
    if (!_envelope["headers"]["X-Resource"]->ok()) {
        _ret +=
          std::string("X-Resource: ") + std::string(_envelope["resource"]) + std::string("\n");
    }
    _ret += std::string("\n");
    if (_envelope["payload"]->ok() && !_envelope["payload"]->empty()) {
        _ret += zpt::json::pretty(_envelope["payload"]);
    }
    return _ret;
}

auto
zpt::rest::not_found(std::string _resource, zpt::json _headers) -> zpt::json {
    return { "channel",
             zpt::generate::r_uuid(),
             "performative",
             zpt::ev::Reply,
             "resource",
             _resource,
             "headers",
             _headers,
             "status",
             404,
             "payload",
             { "text",
               std::string("'") + _resource + std::string("' could not be found"),
               "code",
               1101,
               "assertion_failed",
               "_container->ok()" } };
}

auto
zpt::rest::bad_request(std::string _resource, zpt::json _headers) -> zpt::json {
    return { "channel",
             zpt::generate::r_uuid(),
             "performative",
             zpt::ev::Reply,
             "resource",
             _resource,
             "headers",
             _headers,
             "status",
             400,
             "payload",
             { "text", "bad request", "code", 1100, "assertion_failed", "_socket >> _req" } };
}

auto
zpt::rest::unsupported_media_type(std::string _resource, zpt::json _headers) -> zpt::json {
    return { "channel",
             zpt::generate::r_uuid(),
             "performative",
             zpt::ev::Reply,
             "resource",
             _resource,
             "headers",
             _headers,
             "status",
             415,
             "payload",
             { "text",
               std::string("'") + _resource +
                 std::string("' uri type is not supported by this server"),
               "code",
               1199 } };
}

auto
zpt::rest::accepted(std::string _resource, zpt::json _headers) -> zpt::json {
    return { "channel",      zpt::generate::r_uuid(),
             "performative", zpt::ev::Reply,
             "resource",     _resource,
             "headers",      _headers,
             "status",       202,
             "payload",      { "text", "request was accepted" } };
}

auto
zpt::rest::no_content(std::string _resource, zpt::json _headers) -> zpt::json {
    return { "channel",      zpt::generate::r_uuid(),
             "performative", zpt::ev::Reply,
             "resource",     _resource,
             "headers",      _headers,
             "status",       204 };
}

auto
zpt::rest::temporary_redirect(std::string _resource,
                              std::string _target_resource,
                              zpt::json _headers) -> zpt::json {
    return { "channel",      zpt::generate::r_uuid(),
             "performative", zpt::ev::Reply,
             "resource",     _resource,
             "status",       307,
             "headers",      _headers + zpt::json{ "Location", _target_resource },
             "payload",      { "text", "temporarily redirecting you to another location" } };
}

auto
zpt::rest::see_other(std::string _resource, std::string _target_resource, zpt::json _headers)
  -> zpt::json {
    return { "channel",      zpt::generate::r_uuid(),
             "performative", zpt::ev::Reply,
             "resource",     _resource,
             "status",       303,
             "headers",      _headers + zpt::json{ "Location", _target_resource },
             "payload",      { "text", "temporarily redirecting you to another location" } };
}

auto
zpt::rest::options(std::string _resource, std::string _origin, zpt::json _headers) -> zpt::json {
    return { "channel",
             zpt::generate::r_uuid(),
             "performative",
             zpt::ev::Reply,
             "resource",
             _resource,
             "status",
             200,
             "headers",
             _headers + zpt::json{ "Access-Control-Allow-Origin",
                                   _origin,
                                   "Access-Control-Allow-Methods",
                                   "POST,GET,PUT,DELETE,OPTIONS,HEAD,SYNC,APPLY",
                                   "Access-Control-Allow-Headers",
                                   REST_ACCESS_CONTROL_HEADERS,
                                   "Access-Control-Expose-Headers",
                                   REST_ACCESS_CONTROL_HEADERS,
                                   "Access-Control-Max-Age",
                                   "1728000" },
             "payload",
             { "text", "request was accepted" } };
}

auto
zpt::rest::internal_server_error(std::string _resource, std::exception& _e, zpt::json _headers)
  -> zpt::json {
    return { "channel",      zpt::generate::r_uuid(),
             "performative", zpt::ev::Reply,
             "resource",     _resource,
             "status",       500,
             "headers",      _headers,
             "payload",      { "text", _e.what(), "code", 0 } };
}

auto
zpt::rest::assertion_error(std::string _resource, zpt::missed_expectation& _e, zpt::json _headers)
  -> zpt::json {
    return { "channel",
             zpt::generate::r_uuid(),
             "performative",
             zpt::ev::Reply,
             "resource",
             _resource,
             "status",
             _e.status(),
             "headers",
             _headers,
             "payload",
             { "text", _e.what(), "assertion_failed", _e.description(), "code", _e.code() } };
}
