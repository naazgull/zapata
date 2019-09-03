#pragma once

#include <zapata/base.h>
#include <zapata/events.h>

namespace zpt {
class RESTOntology : public zpt::Ontology {
  public:
    RESTOntology();
    virtual ~RESTOntology();

    virtual auto pretty(zpt::json _envelope) -> std::string;

    virtual auto compose_reply(zpt::performative _method,
                               std::string _resource,
                               zpt::json _payload,
                               zpt::json _headers) -> zpt::json;
    virtual auto compose_request(zpt::performative _method,
                                 std::string _resource,
                                 zpt::json _payload,
                                 zpt::json _headers) -> zpt::json;

    virtual auto extract_from_reply(zpt::json _envelope)
      -> std::tuple<zpt::performative, std::string, zpt::json, zpt::json>;
    virtual auto extract_from_request(zpt::json _envelope)
      -> std::tuple<zpt::performative, std::string, zpt::json, zpt::json>;
};

class RESTListener : public zpt::EventListener {
  public:
    RESTListener(std::string _regex);
    virtual ~RESTListener();

    virtual auto get(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> void;
    virtual auto put(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> void;
    virtual auto post(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter)
      -> void;
    virtual auto del(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter) -> void;
    virtual auto head(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter)
      -> void;
    virtual auto options(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter)
      -> void;
    virtual auto patch(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter)
      -> void;
    virtual auto reply(std::string _resource, zpt::json _envelope, zpt::ev::emitter _emitter)
      -> void;
};

namespace rest {
typedef std::shared_ptr<zpt::RESTListener> listener;

auto
init_request(std::string _cid = "") -> zpt::json;
auto
init_reply(std::string _cid = "", zpt::json _request = zpt::undefined) -> zpt::json;

auto
pretty(zpt::json _envelope) -> std::string;

auto
not_found(std::string _resource, zpt::json _headers = zpt::undefined) -> zpt::json;
auto
bad_request(std::string _resource, zpt::json _headers = zpt::undefined) -> zpt::json;
auto
unsupported_media_type(std::string _resource, zpt::json _headers = zpt::undefined) -> zpt::json;
auto
accepted(std::string _resource, zpt::json _headers = zpt::undefined) -> zpt::json;
auto
no_content(std::string _resource, zpt::json _headers = zpt::undefined) -> zpt::json;
auto
temporary_redirect(std::string _resource,
                   std::string _target_resource,
                   zpt::json _headers = zpt::undefined) -> zpt::json;
auto
see_other(std::string _resource, std::string _target_resource, zpt::json _headers = zpt::undefined)
  -> zpt::json;
auto
options(std::string _resource, std::string _origin, zpt::json _headers = zpt::undefined)
  -> zpt::json;
auto
internal_server_error(std::string _resource,
                      std::exception& _e,
                      zpt::json _headers = zpt::undefined) -> zpt::json;
auto
assertion_error(std::string _resource, zpt::assertion& _e, zpt::json _headers = zpt::undefined)
  -> zpt::json;
} // namespace rest
} // namespace zpt
