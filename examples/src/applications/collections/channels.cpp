#include <zapata/applications/collections/channels.h>

auto zpt::apps::collections::channels::restify(zpt::ev::emitter _emitter) -> void {
_emitter->on("^/v2/datums/applications/([^/]+)/users/([^/]+)/channels$",
{
{
zpt::ev::Get,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_app_id = _t_split[3];
zpt::json _tv_user_id = _t_split[5];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
return _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_app_id), "users", std::string(_tv_user_id), "channels" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", _tv_app_id, "user_id", _tv_user_id })) }
);


}
}




,
{
zpt::ev::Head,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_app_id = _t_split[3];
zpt::json _tv_user_id = _t_split[5];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
return _emitter->route(
zpt::ev::Head,
zpt::path::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_app_id), "users", std::string(_tv_user_id), "channels" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", _tv_app_id, "user_id", _tv_user_id })) }
);


}
}

},
{"0mq", true,"amqp", true,"http", true,"mqtt", true}
);
}

