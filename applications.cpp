#include <zapata/rest.h>
#include <string>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

extern "C" void restify(zpt::ev::emitter _emitter) {
_emitter->connector({ { "dbms.mongodb", zpt::connector(new zpt::mongodb::Client(_emitter->options(), "mongodb.zpt.apps")) }, { "dbms.pgsql", zpt::connector(new zpt::pgsql::Client(_emitter->options(), "pgsql.zpt.apps")) }, { "dbms.redis", zpt::connector(new zpt::redis::Client(_emitter->options(), "redis.zpt.apps")) }, });


_emitter->on("^/v2/datums/applications$",
{
{
zpt::ev::Get,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::query("application", _topic, _envelope["payload"], _envelope["params"], _emitter);

return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body };

}
},

{
zpt::ev::Post,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_mandatory(_envelope["payload"], "created", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_mandatory(_envelope["payload"], "name", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_mandatory(_envelope["payload"], "namespace", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_mandatory(_envelope["payload"], "updated", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::insert("application", _topic, _envelope["payload"], _envelope["params"], _emitter);

return { "status", 201, "payload", _r_body };

}
},


{
zpt::ev::Patch,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::set("application", _topic, _envelope["payload"], _envelope["params"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Delete,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::remove("application", _topic, _envelope["payload"], _envelope["params"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Head,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::query("application", _topic, _envelope["payload"], _envelope["params"], _emitter);

return { "headers", { "Content-Length", std::string(_r_body).length() }, "status", (_r_body->ok() ? 200 : 204) };

}
},

},
{"0mq", true,"amqp", true,"http", true,"mqtt", true}
);

_emitter->on("^/v2/datums/applications/([^/]+)$",
{
{
zpt::ev::Get,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::get("application", _topic, _envelope["payload"], _emitter);

return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body  };

}
},


{
zpt::ev::Put,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_mandatory(_envelope["payload"], "created", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_mandatory(_envelope["payload"], "name", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_mandatory(_envelope["payload"], "namespace", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_mandatory(_envelope["payload"], "updated", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::save("application", _topic, _envelope["payload"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Patch,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::set("application", _topic, _envelope["payload"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Delete,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::remove("application", _topic, _envelope["payload"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Head,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
assertz_token(_envelope["payload"], "app_secret", 412);
assertz_token(_envelope["payload"], "cloud_secret", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
assertz_token(_envelope["payload"], "device_secret", 412);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::datum::application::get("application", _topic, _envelope["payload"], _emitter);

return { "headers", { "Content-Length", std::string(_r_body).length() }, "status", (_r_body->ok() ? 200 : 204) };

}
},

},
{"0mq", true,"amqp", true,"http", true,"mqtt", true}
);

}

