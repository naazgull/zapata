#include <zapata/applications/datums/MyUsers.h>

auto zpt::apps::datums::MyUsers::get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
_r_data = _c->get("MyUsers", _topic);

	zpt::json _d_channels = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "channel-subscriptions" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", std::string(_r_data["id"]) })) }
)["payload"];
_r_data << "channels" << (_d_channels["elements"]->type() == zpt::JSArray ? _d_channels["elements"] : zpt::json({ zpt::array, _d_channels }));

	return _r_data;
}

auto zpt::apps::datums::MyUsers::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
_r_data = _c->query("MyUsers", _filter, _filter);

	if (_r_data["elements"]->type() == zpt::JSArray) {
for (auto _d_element : _r_data["elements"]->arr()) {
zpt::json _d_channels = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "channel-subscriptions" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", std::string(_d_element["id"]) })) }
)["payload"];
_d_element << "channels" << (_d_channels["elements"]->type() == zpt::JSArray ? _d_channels["elements"] : zpt::json({ zpt::array, _d_channels }));
}
}

	return _r_data;
}

auto zpt::apps::datums::MyUsers::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}

auto zpt::apps::datums::MyUsers::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	
	
	return _r_data;
}


