#pragma once

#include <string>
#include <zapata/rest.h>

namespace zpt {
namespace apps {

namespace ResourceOwners {
auto
get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
query(std::string _topic,
      zpt::json _filter,
      zpt::ev::emitter _emitter,
      zpt::json _identity,
      zpt::json _envelope) -> zpt::json;
auto
insert(std::string _topic,
       zpt::json _document,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
auto
save(std::string _topic,
     zpt::json _document,
     zpt::ev::emitter _emitter,
     zpt::json _identity,
     zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::json _filter,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
remove(std::string _topic,
       zpt::json _filter,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
} // namespace ResourceOwners
} // namespace apps
} // namespace zpt

auto
zpt::apps::ResourceOwners::get(std::string _topic,
                               zpt::ev::emitter _emitter,
                               zpt::json _identity,
                               zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
    zpt::json _r_data = _c->get("ResourceOwners", _topic);

    return _r_data;
}

auto
zpt::apps::ResourceOwners::query(std::string _topic,
                                 zpt::json _filter,
                                 zpt::ev::emitter _emitter,
                                 zpt::json _identity,
                                 zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
    zpt::json _r_data = _c->query("ResourceOwners", _filter, _filter);

    return _r_data;
}

auto
zpt::apps::ResourceOwners::insert(std::string _topic,
                                  zpt::json _document,
                                  zpt::ev::emitter _emitter,
                                  zpt::json _identity,
                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "created" << zpt::json::date() << "updated" << zpt::json::date();

    zpt::json _r_data = _c->insert("ResourceOwners", _topic, _document);

    return _r_data;
}

auto
zpt::apps::ResourceOwners::save(std::string _topic,
                                zpt::json _document,
                                zpt::ev::emitter _emitter,
                                zpt::json _identity,
                                zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "updated" << zpt::json::date();

    zpt::json _r_data = _c->save("ResourceOwners", _topic, _document);

    return _r_data;
}

auto
zpt::apps::ResourceOwners::set(std::string _topic,
                               zpt::json _document,
                               zpt::ev::emitter _emitter,
                               zpt::json _identity,
                               zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "updated" << zpt::json::date();

    zpt::json _r_data = _c->set("ResourceOwners", _topic, _document);

    return _r_data;
}

auto
zpt::apps::ResourceOwners::set(std::string _topic,
                               zpt::json _document,
                               zpt::json _filter,
                               zpt::ev::emitter _emitter,
                               zpt::json _identity,
                               zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "updated" << zpt::json::date();

    zpt::json _r_data = _c->set("ResourceOwners", _filter, _document);

    return _r_data;
}

auto
zpt::apps::ResourceOwners::remove(std::string _topic,
                                  zpt::ev::emitter _emitter,
                                  zpt::json _identity,
                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
    zpt::json _r_data = _c->remove("ResourceOwners", _topic);

    return _r_data;
}

auto
zpt::apps::ResourceOwners::remove(std::string _topic,
                                  zpt::json _filter,
                                  zpt::ev::emitter _emitter,
                                  zpt::json _identity,
                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
    zpt::json _r_data = _c->remove("ResourceOwners", _filter, _filter);

    return _r_data;
}

#pragma once

#include <string>
#include <zapata/rest.h>

namespace zpt {
namespace apps {

namespace Applications {
auto
get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
query(std::string _topic,
      zpt::json _filter,
      zpt::ev::emitter _emitter,
      zpt::json _identity,
      zpt::json _envelope) -> zpt::json;
auto
insert(std::string _topic,
       zpt::json _document,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
auto
save(std::string _topic,
     zpt::json _document,
     zpt::ev::emitter _emitter,
     zpt::json _identity,
     zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::json _filter,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
remove(std::string _topic,
       zpt::json _filter,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
} // namespace Applications
} // namespace apps
} // namespace zpt

auto
zpt::apps::Applications::get(std::string _topic,
                             zpt::ev::emitter _emitter,
                             zpt::json _identity,
                             zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
    zpt::json _r_data = _c->get("Applications", _topic);

    zpt::json _d_admin = _emitter->route(
      zpt::ev::Get,
      zpt::path::join({ zpt::array, "v2", "datums", "user-applications" }),
      { "headers",
        zpt::rest::authorization::headers(_identity["access_token"]),
        "params",
        (_envelope["params"] +
         { "app_id", std::string(_r_data["id"]), "role", "m/admin/i" }) })["payload"];
    _r_data << "admin"
            << (_d_admin["elements"]->type() == zpt::JSArray ? _d_admin["elements"][0] : _d_admin);
    zpt::json _d_token =
      _emitter->route(zpt::ev::Get,
                      zpt::path::join({ zpt::array, "v2", "datums", "token" }),
                      { "headers",
                        zpt::rest::authorization::headers(_identity["access_token"]),
                        "params",
                        (_envelope["params"] + { "id", std::string(_r_data["id"]) }) })["payload"];
    _r_data << "token"
            << (_d_token["elements"]->type() == zpt::JSArray ? _d_token["elements"][0] : _d_token);
    zpt::json _d_users = _emitter->route(
      zpt::ev::Get,
      zpt::path::join({ zpt::array, "v2", "datums", "users" }),
      { "headers",
        zpt::rest::authorization::headers(_identity["access_token"]),
        "params",
        (_envelope["params"] + { "app_id", std::string(_r_data["id"]) }) })["payload"];
    _r_data << "users"
            << (_d_users["elements"]->type() == zpt::JSArray ? _d_users["elements"]
                                                             : { zpt::array, _d_users });

    return _r_data;
}

auto
zpt::apps::Applications::query(std::string _topic,
                               zpt::json _filter,
                               zpt::ev::emitter _emitter,
                               zpt::json _identity,
                               zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
    zpt::json _r_data = _c->query("Applications", _filter, _filter);

    if (_r_data["elements"]->type() == zpt::JSArray) {
        for (auto _d_element : _r_data["elements"]->arr()) {
            zpt::json _d_admin = _emitter->route(
              zpt::ev::Get,
              zpt::path::join({ zpt::array, "v2", "datums", "user-applications" }),
              { "headers",
                zpt::rest::authorization::headers(_identity["access_token"]),
                "params",
                (_envelope["params"] +
                 { "app_id", std::string(_d_element["id"]), "role", "m/admin/i" }) })["payload"];
            _d_element << "admin"
                       << (_d_admin["elements"]->type() == zpt::JSArray ? _d_admin["elements"][0]
                                                                        : _d_admin);
        }
    }
    zpt::json _d_token = _emitter->route(
      zpt::ev::Get,
      zpt::path::join({ zpt::array, "v2", "datums", "token" }),
      { "headers",
        zpt::rest::authorization::headers(_identity["access_token"]),
        "params",
        (_envelope["params"] + { "id", std::string(_d_element["id"]) }) })["payload"];
    _d_element << "token"
               << (_d_token["elements"]->type() == zpt::JSArray ? _d_token["elements"][0]
                                                                : _d_token);
    zpt::json _d_users = _emitter->route(
      zpt::ev::Get,
      zpt::path::join({ zpt::array, "v2", "datums", "users" }),
      { "headers",
        zpt::rest::authorization::headers(_identity["access_token"]),
        "params",
        (_envelope["params"] + { "app_id", std::string(_d_element["id"]) }) })["payload"];
    _d_element << "users"
               << (_d_users["elements"]->type() == zpt::JSArray ? _d_users["elements"]
                                                                : { zpt::array, _d_users });

    return _r_data;
}

auto
zpt::apps::Applications::insert(std::string _topic,
                                zpt::json _document,
                                zpt::ev::emitter _emitter,
                                zpt::json _identity,
                                zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "created" << zpt::json::date() << "updated" << zpt::json::date();

    zpt::json _r_data = _c->insert("Applications", _topic, _document);

    return _r_data;
}

auto
zpt::apps::Applications::save(std::string _topic,
                              zpt::json _document,
                              zpt::ev::emitter _emitter,
                              zpt::json _identity,
                              zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "updated" << zpt::json::date();

    zpt::json _r_data = _c->save("Applications", _topic, _document);

    return _r_data;
}

auto
zpt::apps::Applications::set(std::string _topic,
                             zpt::json _document,
                             zpt::ev::emitter _emitter,
                             zpt::json _identity,
                             zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "updated" << zpt::json::date();

    zpt::json _r_data = _c->set("Applications", _topic, _document);

    return _r_data;
}

auto
zpt::apps::Applications::set(std::string _topic,
                             zpt::json _document,
                             zpt::json _filter,
                             zpt::ev::emitter _emitter,
                             zpt::json _identity,
                             zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

    _document << "updated" << zpt::json::date();

    zpt::json _r_data = _c->set("Applications", _filter, _document);

    return _r_data;
}

auto
zpt::apps::Applications::remove(std::string _topic,
                                zpt::ev::emitter _emitter,
                                zpt::json _identity,
                                zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
    zpt::json _r_data = _c->remove("Applications", _topic);

    return _r_data;
}

auto
zpt::apps::Applications::remove(std::string _topic,
                                zpt::json _filter,
                                zpt::ev::emitter _emitter,
                                zpt::json _identity,
                                zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
    zpt::json _r_data = _c->remove("Applications", _filter, _filter);

    return _r_data;
}

#pragma once

#include <string>
#include <zapata/rest.h>

namespace zpt {
namespace apps {

namespace MyApplications {
auto
get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
query(std::string _topic,
      zpt::json _filter,
      zpt::ev::emitter _emitter,
      zpt::json _identity,
      zpt::json _envelope) -> zpt::json;
auto
insert(std::string _topic,
       zpt::json _document,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
auto
save(std::string _topic,
     zpt::json _document,
     zpt::ev::emitter _emitter,
     zpt::json _identity,
     zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::json _filter,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
remove(std::string _topic,
       zpt::json _filter,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
} // namespace MyApplications
} // namespace apps
} // namespace zpt

auto
zpt::apps::MyApplications::get(std::string _topic,
                               zpt::ev::emitter _emitter,
                               zpt::json _identity,
                               zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    _r_data = _emitter->route(zpt::ev::Get,
                              zpt::path::join({ zpt::array, "v2", "datums", "applications" }),
                              { "headers",
                                zpt::rest::authorization::headers(_identity["access_token"]),
                                "params",
                                (_envelope["params"] + { "namespace", "m/engie/i" }) })["payload"];

    zpt::json _d_channels = _emitter->route(
      zpt::ev::Get,
      zpt::path::join({ zpt::array, "v2", "datums", "channel-subscriptions" }),
      { "headers",
        zpt::rest::authorization::headers(_identity["access_token"]),
        "params",
        (_envelope["params"] + { "app_id", std::string(_r_data["id"]) }) })["payload"];
    _r_data << "channels"
            << (_d_channels["elements"]->type() == zpt::JSArray ? _d_channels["elements"]
                                                                : { zpt::array, _d_channels });

    return _r_data;
}

auto
zpt::apps::MyApplications::query(std::string _topic,
                                 zpt::json _filter,
                                 zpt::ev::emitter _emitter,
                                 zpt::json _identity,
                                 zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    _r_data = _emitter->route(zpt::ev::Get,
                              zpt::path::join({ zpt::array, "v2", "datums", "applications" }),
                              { "headers",
                                zpt::rest::authorization::headers(_identity["access_token"]),
                                "params",
                                (_envelope["params"] + { "namespace", "m/engie/i" }) })["payload"];
    _r_data << "payload"
            << (_r_data["payload"]["elements"]->ok()
                  ? _r_data["payload"]
                  : { "size", 1, "elements", _r_data["payload"] });

    if (_r_data["elements"]->type() == zpt::JSArray) {
        for (auto _d_element : _r_data["elements"]->arr()) {
            zpt::json _d_channels = _emitter->route(
              zpt::ev::Get,
              zpt::path::join({ zpt::array, "v2", "datums", "channel-subscriptions" }),
              { "headers",
                zpt::rest::authorization::headers(_identity["access_token"]),
                "params",
                (_envelope["params"] + { "app_id", std::string(_d_element["id"]) }) })["payload"];
            _d_element << "channels"
                       << (_d_channels["elements"]->type() == zpt::JSArray
                             ? _d_channels["elements"]
                             : { zpt::array, _d_channels });
        }
    }

    return _r_data;
}

auto
zpt::apps::MyApplications::insert(std::string _topic,
                                  zpt::json _document,
                                  zpt::ev::emitter _emitter,
                                  zpt::json _identity,
                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyApplications::save(std::string _topic,
                                zpt::json _document,
                                zpt::ev::emitter _emitter,
                                zpt::json _identity,
                                zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyApplications::set(std::string _topic,
                               zpt::json _document,
                               zpt::ev::emitter _emitter,
                               zpt::json _identity,
                               zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyApplications::set(std::string _topic,
                               zpt::json _document,
                               zpt::json _filter,
                               zpt::ev::emitter _emitter,
                               zpt::json _identity,
                               zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyApplications::remove(std::string _topic,
                                  zpt::ev::emitter _emitter,
                                  zpt::json _identity,
                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyApplications::remove(std::string _topic,
                                  zpt::json _filter,
                                  zpt::ev::emitter _emitter,
                                  zpt::json _identity,
                                  zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

#pragma once

#include <string>
#include <zapata/rest.h>

namespace zpt {
namespace apps {

namespace MyUsers {
auto
get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
query(std::string _topic,
      zpt::json _filter,
      zpt::ev::emitter _emitter,
      zpt::json _identity,
      zpt::json _envelope) -> zpt::json;
auto
insert(std::string _topic,
       zpt::json _document,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
auto
save(std::string _topic,
     zpt::json _document,
     zpt::ev::emitter _emitter,
     zpt::json _identity,
     zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
set(std::string _topic,
    zpt::json _document,
    zpt::json _filter,
    zpt::ev::emitter _emitter,
    zpt::json _identity,
    zpt::json _envelope) -> zpt::json;
auto
remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope)
  -> zpt::json;
auto
remove(std::string _topic,
       zpt::json _filter,
       zpt::ev::emitter _emitter,
       zpt::json _identity,
       zpt::json _envelope) -> zpt::json;
} // namespace MyUsers
} // namespace apps
} // namespace zpt

auto
zpt::apps::MyUsers::get(std::string _topic,
                        zpt::ev::emitter _emitter,
                        zpt::json _identity,
                        zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
    zpt::json _r_data = _c->get("MyUsers", _topic);

    zpt::json _d_channels = _emitter->route(
      zpt::ev::Get,
      zpt::path::join({ zpt::array, "v2", "datums", "channel-subscriptions" }),
      { "headers",
        zpt::rest::authorization::headers(_identity["access_token"]),
        "params",
        (_envelope["params"] + { "app_id", std::string(_r_data["id"]) }) })["payload"];
    _r_data << "channels"
            << (_d_channels["elements"]->type() == zpt::JSArray ? _d_channels["elements"]
                                                                : { zpt::array, _d_channels });

    return _r_data;
}

auto
zpt::apps::MyUsers::query(std::string _topic,
                          zpt::json _filter,
                          zpt::ev::emitter _emitter,
                          zpt::json _identity,
                          zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;
    zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
    zpt::json _r_data = _c->query("MyUsers", _filter, _filter);

    if (_r_data["elements"]->type() == zpt::JSArray) {
        for (auto _d_element : _r_data["elements"]->arr()) {
            zpt::json _d_channels = _emitter->route(
              zpt::ev::Get,
              zpt::path::join({ zpt::array, "v2", "datums", "channel-subscriptions" }),
              { "headers",
                zpt::rest::authorization::headers(_identity["access_token"]),
                "params",
                (_envelope["params"] + { "app_id", std::string(_d_element["id"]) }) })["payload"];
            _d_element << "channels"
                       << (_d_channels["elements"]->type() == zpt::JSArray
                             ? _d_channels["elements"]
                             : { zpt::array, _d_channels });
        }
    }

    return _r_data;
}

auto
zpt::apps::MyUsers::insert(std::string _topic,
                           zpt::json _document,
                           zpt::ev::emitter _emitter,
                           zpt::json _identity,
                           zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyUsers::save(std::string _topic,
                         zpt::json _document,
                         zpt::ev::emitter _emitter,
                         zpt::json _identity,
                         zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyUsers::set(std::string _topic,
                        zpt::json _document,
                        zpt::ev::emitter _emitter,
                        zpt::json _identity,
                        zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyUsers::set(std::string _topic,
                        zpt::json _document,
                        zpt::json _filter,
                        zpt::ev::emitter _emitter,
                        zpt::json _identity,
                        zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyUsers::remove(std::string _topic,
                           zpt::ev::emitter _emitter,
                           zpt::json _identity,
                           zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

auto
zpt::apps::MyUsers::remove(std::string _topic,
                           zpt::json _filter,
                           zpt::ev::emitter _emitter,
                           zpt::json _identity,
                           zpt::json _envelope) -> zpt::json {
    zpt::json _r_data;

    return _r_data;
}

#include <string>
#include <zapata/rest.h>

extern "C" void
_zpt_load_() {
    _emitter->connector({
      { "dbms.mongodb.zpt.apps",
        zpt::connector(new zpt::mongodb::Client(_emitter->options(), "mongodb.zpt.apps")) },
      { "dbms.pgsql.zpt.apps",
        zpt::connector(new zpt::pgsql::Client(_emitter->options(), "pgsql.zpt.apps")) },
      { "dbms.redis.zpt.apps",
        zpt::connector(new zpt::redis::Client(_emitter->options(), "redis.zpt.apps")) },
    });

    _emitter->on(
      "^/v2/datums/applications$",
      {
        { zpt::ev::Get,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::query(
                _topic, _envelope["params"], _emitter, _identity, _envelope);

              return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body };
          } },

        { zpt::ev::Post,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] << "app_secret" << zpt::generate_key(24);
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] << "cloud_secret" << zpt::generate_key(24);
              _envelope["payload"] >> "device_secret";
              _envelope["payload"] << "device_secret" << zpt::generate_key(24);
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_mandatory(_envelope["payload"], "name", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_mandatory(_envelope["payload"], "namespace", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::insert(
                _topic, _envelope["payload"], _emitter, _identity, _envelope);

              return { "status", 201, "payload", _r_body };
          } },

        { zpt::ev::Patch,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::set(
                _topic, _envelope["payload"], _envelope["params"], _emitter, _identity, _envelope);

              return { "status", 200, "payload", _r_body };
          } },

        { zpt::ev::Delete,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::remove(
                _topic, _envelope["params"], _emitter, _identity, _envelope);

              return { "status", 200, "payload", _r_body };
          } },

        { zpt::ev::Head,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::query(
                _topic, _envelope["params"], _emitter, _identity, _envelope);

              return { "headers",
                       { "Content-Length", std::string(_r_body).length() },
                       "status",
                       (_r_body->ok() ? 200 : 204) };
          } },

      },
      { "0mq", true, "amqp", true, "http", true, "mqtt", true });

    _emitter->on(
      "^/v2/datums/applications/([^/]+)$",
      {
        { zpt::ev::Get,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_id = _t_split[3];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::get(_topic, _emitter);

              if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
                  _r_body["payload"] << "links"
                                     << { "users",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "users" }) "templates",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-templates" }) "services",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "services" }) "paired",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-subscriptions" }) "subscribed",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "service-subscriptions" }) };
              }
              return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body };
          } },

        { zpt::ev::Put,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] << "app_secret" << zpt::generate_key(24);
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] << "cloud_secret" << zpt::generate_key(24);
              _envelope["payload"] >> "device_secret";
              _envelope["payload"] << "device_secret" << zpt::generate_key(24);
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_mandatory(_envelope["payload"], "name", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_mandatory(_envelope["payload"], "namespace", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_id = _t_split[3];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::save(
                _topic, _envelope["payload"], _emitter, _identity, _envelope);

              if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
                  _r_body["payload"] << "links"
                                     << { "users",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "users" }) "templates",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-templates" }) "services",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "services" }) "paired",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-subscriptions" }) "subscribed",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "service-subscriptions" }) };
              }
              return { "status", 200, "payload", _r_body };
          } },

        { zpt::ev::Patch,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_id = _t_split[3];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::set(
                _topic, _envelope["payload"], _emitter, _identity, _envelope);

              if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
                  _r_body["payload"] << "links"
                                     << { "users",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "users" }) "templates",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-templates" }) "services",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "services" }) "paired",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-subscriptions" }) "subscribed",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "service-subscriptions" }) };
              }
              return { "status", 200, "payload", _r_body };
          } },

        { zpt::ev::Delete,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_id = _t_split[3];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::remove(_topic, _emitter, _identity, _envelope);

              return { "status", 200, "payload", _r_body };
          } },

        { zpt::ev::Head,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              _envelope["payload"] >> "app_secret";
              _envelope["payload"] >> "cloud_secret";
              _envelope["payload"] >> "device_secret";
              assertz_uri(_envelope["payload"], "icon", 412);
              assertz_uri(_envelope["payload"], "image", 412);
              assertz_utf8(_envelope["payload"], "name", 412);
              assertz_ascii(_envelope["payload"], "namespace", 412);

              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_id = _t_split[3];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              _r_body = zpt::apps::Applications::get(_topic, _emitter, _identity, _envelope);

              if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
                  _r_body["payload"] << "links"
                                     << { "users",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "users" }) "templates",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-templates" }) "services",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "services" }) "paired",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "channel-subscriptions" }) "subscribed",
                                          zpt::path::join({,
                                                           "v2",
                                                           "datums",
                                                           "applications",
                                                           std::string(_tv_id),
                                                           "service-subscriptions" }) };
              }
              return { "headers",
                       { "Content-Length", std::string(_r_body).length() },
                       "status",
                       (_r_body->ok() ? 200 : 204) };
          } },

      },
      { "0mq", true, "amqp", true, "http", true, "mqtt", true });

    _emitter->on(
      "^/v2/datums/applications/([^/]+)/users/([^/]+)/channels$",
      {
        { zpt::ev::Get,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_app_id = _t_split[3];
              zpt::json _tv_user_id = _t_split[5];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              return _emitter->route(
                zpt::ev::Get,
                zpt::path::join({ zpt::array,
                                  "v2",
                                  "datums",
                                  "applications",
                                  std::string(_tv_app_id),
                                  "users",
                                  std::string(_tv_user_id),
                                  "channels" }),
                { "headers",
                  zpt::rest::authorization::headers(_identity["access_token"]),
                  "params",
                  (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) });
          } },

        { zpt::ev::Post,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_app_id = _t_split[3];
              zpt::json _tv_user_id = _t_split[5];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              return _emitter->route(zpt::ev::Post,
                                     zpt::path::join({ zpt::array,
                                                       "v2",
                                                       "datums",
                                                       "applications",
                                                       std::string(_tv_app_id),
                                                       "users",
                                                       std::string(_tv_user_id),
                                                       "channels" }),
                                     { "headers",
                                       zpt::rest::authorization::headers(_identity["access_token"]),
                                       "payload",
                                       _envelope["payload"] });
          } },

        { zpt::ev::Patch,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_app_id = _t_split[3];
              zpt::json _tv_user_id = _t_split[5];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              return _emitter->route(
                zpt::ev::Patch,
                zpt::path::join({ zpt::array,
                                  "v2",
                                  "datums",
                                  "applications",
                                  std::string(_tv_app_id),
                                  "users",
                                  std::string(_tv_user_id),
                                  "channels" }),
                { "headers",
                  zpt::rest::authorization::headers(_identity["access_token"]),
                  "payload",
                  _envelope["payload"],
                  "params",
                  (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) });
          } },

        { zpt::ev::Delete,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_app_id = _t_split[3];
              zpt::json _tv_user_id = _t_split[5];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              return _emitter->route(
                zpt::ev::Get,
                zpt::path::join({ zpt::array,
                                  "v2",
                                  "datums",
                                  "applications",
                                  std::string(_tv_app_id),
                                  "users",
                                  std::string(_tv_user_id),
                                  "channels" }),
                { "headers",
                  zpt::rest::authorization::headers(_identity["access_token"]),
                  "params",
                  (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) });
          } },

        { zpt::ev::Head,
          [](zpt::performative _performative,
             std::string _topic,
             zpt::json _envelope,
             zpt::ev::emitter _emitter) -> zpt::json {
              zpt::json _t_split = zpt::split(_topic, "/");
              zpt::json _tv_app_id = _t_split[3];
              zpt::json _tv_user_id = _t_split[5];
              zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

              zpt::json _r_body;
              return _emitter->route(
                zpt::ev::Head,
                zpt::path::join({ zpt::array,
                                  "v2",
                                  "datums",
                                  "applications",
                                  std::string(_tv_app_id),
                                  "users",
                                  std::string(_tv_user_id),
                                  "channels" }),
                { "headers",
                  zpt::rest::authorization::headers(_identity["access_token"]),
                  "params",
                  (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) });
          } },

      },
      { "0mq", true, "amqp", true, "http", true, "mqtt", true });
}
