#include <zapata/applications/documents/applications.h>

auto zpt::apps::documents::applications::restify(zpt::ev::emitter _emitter) -> void {
_emitter->on("^/v2/datums/applications/([^/]+)$",
{
{
zpt::ev::Get,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate("/v2/datums/applications/{id}", _envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::datums::Applications::get(_topic, _emitter, _identity, _envelope);

if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
_r_body["payload"] << "links" << zpt::json({ "users", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "users" }, "/"), "templates", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-templates" }, "/"), "services", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "services" }, "/"), "paired", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-subscriptions" }, "/"), "subscribed", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "service-subscriptions" }, "/"),  });
}
return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body  };

}
}

,
{
zpt::ev::Put,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_object(_envelope["payload"], "admin", 412);
_envelope["payload"] >> "app_secret";
_envelope["payload"] << "app_secret" << zpt::generate::r_key(24);
_envelope["payload"] >> "cloud_secret";
_envelope["payload"] << "cloud_secret" << zpt::generate::r_key(24);
_envelope["payload"] >> "device_secret";
_envelope["payload"] << "device_secret" << zpt::generate::r_key(24);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_mandatory(_envelope["payload"], "name", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_mandatory(_envelope["payload"], "namespace", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_object(_envelope["payload"], "token", 412);
assertz_array(_envelope["payload"], "users", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate("/v2/datums/applications/{id}", _envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::datums::Applications::save(_topic, _envelope["payload"], _emitter, _identity, _envelope);

if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
_r_body["payload"] << "links" << zpt::json({ "users", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "users" }, "/"), "templates", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-templates" }, "/"), "services", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "services" }, "/"), "paired", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-subscriptions" }, "/"), "subscribed", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "service-subscriptions" }, "/"),  });
}
return { "status", 200, "payload", _r_body };

}
}
,
{
zpt::ev::Patch,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_object(_envelope["payload"], "admin", 412);
_envelope["payload"] >> "app_secret";
_envelope["payload"] << "app_secret" << zpt::generate::r_key(24);
_envelope["payload"] >> "cloud_secret";
_envelope["payload"] << "cloud_secret" << zpt::generate::r_key(24);
_envelope["payload"] >> "device_secret";
_envelope["payload"] << "device_secret" << zpt::generate::r_key(24);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_object(_envelope["payload"], "token", 412);
assertz_array(_envelope["payload"], "users", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate("/v2/datums/applications/{id}", _envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::datums::Applications::set(_topic, _envelope["payload"], _emitter, _identity, _envelope);

if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
_r_body["payload"] << "links" << zpt::json({ "users", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "users" }, "/"), "templates", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-templates" }, "/"), "services", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "services" }, "/"), "paired", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-subscriptions" }, "/"), "subscribed", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "service-subscriptions" }, "/"),  });
}
return { "status", 200, "payload", _r_body };

}
}
,
{
zpt::ev::Delete,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate("/v2/datums/applications/{id}", _envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::datums::Applications::remove(_topic, _emitter, _identity, _envelope);

return { "status", 200, "payload", _r_body };

}
}
,
{
zpt::ev::Head,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate("/v2/datums/applications/{id}", _envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::datums::Applications::get(_topic, _emitter, _identity, _envelope);

if (_r_body["payload"]->ok() && int(_r_body["status"]) < 300) {
_r_body["payload"] << "links" << zpt::json({ "users", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "users" }, "/"), "templates", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-templates" }, "/"), "services", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "services" }, "/"), "paired", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "channel-subscriptions" }, "/"), "subscribed", zpt::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_id), "service-subscriptions" }, "/"),  });
}
return { "headers",
{ "Content-Length", std::string(_r_body).length() }, "status", (_r_body->ok() ? 200 : 204) };

}
}

},
{"0mq", true,"amqp", true,"http", true,"mqtt", true}
);
}

