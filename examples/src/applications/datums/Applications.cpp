#include <zapata/applications/datums/Applications.h>

auto zpt::apps::datums::Applications::get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
_r_data = _c->get("Applications", _topic);

	zpt::json _d_admin = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "user-applications" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", std::string(_r_data["id"]), "role", "m/admin/i" })) }
)["payload"];
_r_data << "admin" << (_d_admin["elements"]->type() == zpt::JSArray ? _d_admin["elements"][0] : _d_admin);
zpt::json _d_token = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "token" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "id", std::string(_r_data["id"]) })) }
)["payload"];
_r_data << "token" << (_d_token["elements"]->type() == zpt::JSArray ? _d_token["elements"][0] : _d_token);
zpt::json _d_users = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "users" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", std::string(_r_data["id"]) })) }
)["payload"];
_r_data << "users" << (_d_users["elements"]->type() == zpt::JSArray ? _d_users["elements"] : zpt::json({ zpt::array, _d_users }));

	return _r_data;
}

auto zpt::apps::datums::Applications::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
_r_data = _c->query("Applications", _filter, _filter);

	if (_r_data["elements"]->type() == zpt::JSArray) {
for (auto _d_element : _r_data["elements"]->arr()) {
zpt::json _d_admin = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "user-applications" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", std::string(_d_element["id"]), "role", "m/admin/i" })) }
)["payload"];
_d_element << "admin" << (_d_admin["elements"]->type() == zpt::JSArray ? _d_admin["elements"][0] : _d_admin);
zpt::json _d_token = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "token" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "id", std::string(_d_element["id"]) })) }
)["payload"];
_d_element << "token" << (_d_token["elements"]->type() == zpt::JSArray ? _d_token["elements"][0] : _d_token);
zpt::json _d_users = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "users" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "app_id", std::string(_d_element["id"]) })) }
)["payload"];
_d_element << "users" << (_d_users["elements"]->type() == zpt::JSArray ? _d_users["elements"] : zpt::json({ zpt::array, _d_users }));
}
}

	return _r_data;
}

auto zpt::apps::datums::Applications::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

_document <<
"created" << zpt::json::date() <<
"updated" << zpt::json::date();

_r_data = { "href", (_topic + std::string("/") + _c->insert("Applications", _topic, _document)) };

	
	return _r_data;
}

auto zpt::apps::datums::Applications::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

_document <<
"updated" << zpt::json::date();

_r_data = { "href", _topic, "n_updated", _c->save("Applications", _topic, _document) };

	
	return _r_data;
}

auto zpt::apps::datums::Applications::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

_document <<
"updated" << zpt::json::date();

_r_data = { "href", _topic, "n_updated", _c->set("Applications", _topic, _document) };

	
	return _r_data;
}

auto zpt::apps::datums::Applications::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

_document <<
"updated" << zpt::json::date();

_r_data = { "href", _topic, "n_updated", _c->set("Applications", _filter, _document) };

	
	return _r_data;
}

auto zpt::apps::datums::Applications::remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
_r_data = { "href", _topic, "n_deleted", _c->remove("Applications", _topic) };

	
	return _r_data;
}

auto zpt::apps::datums::Applications::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
_r_data = { "href", _topic, "n_deleted", _c->remove("Applications", _filter, _filter) };

	
	return _r_data;
}


