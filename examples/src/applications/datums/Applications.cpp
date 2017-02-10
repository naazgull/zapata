#include <zapata/applications/datums/Applications.h>

auto zpt::apps::datums::Applications::get(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
_r_data = _c->get("Applications", _topic, { "href", _topic });

	zpt::json _d_token = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "token" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "id", std::string(_r_data["id"]) })) }
)["payload"];
_r_data << "token" << (_d_token["elements"]->type() == zpt::JSArray ? _d_token["elements"][0] : _d_token);

	return _r_data;
}

auto zpt::apps::datums::Applications::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
_r_data = _c->query("Applications", _filter, _filter + zpt::json({ "href", _topic }));

	if (_r_data["elements"]->type() == zpt::JSArray) {
for (auto _d_element : _r_data["elements"]->arr()) {
zpt::json _d_token = _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "token" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + zpt::json({ "id", std::string(_d_element["id"]) })) }
)["payload"];
_d_element << "token" << (_d_token["elements"]->type() == zpt::JSArray ? _d_token["elements"][0] : _d_token);
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

_r_data = { "href", (_topic + std::string("/") + _c->insert("Applications", _topic, _document, { "href", _topic })) };

	
	_emitter->mutations()->route(zpt::mutation::Insert, _topic, { "performative", "insert", "href", _document["href"], "new", _document });
	return _r_data;
}

auto zpt::apps::datums::Applications::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

_document <<
"updated" << zpt::json::date();

_r_data = { "href", _topic, "n_updated", _c->save("Applications", _topic, _document, { "href", _topic }) };

	
	_emitter->mutations()->route(zpt::mutation::Replace, _topic, { "performative", "save", "href", _topic, "new", _document });
	return _r_data;
}

auto zpt::apps::datums::Applications::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

_document <<
"updated" << zpt::json::date();

_r_data = { "href", _topic, "n_updated", _c->set("Applications", _topic, _document, { "href", _topic }) };

	
	_emitter->mutations()->route(zpt::mutation::Update, _topic, { "performative", "set", "href", _topic, "changes", _document });
	return _r_data;
}

auto zpt::apps::datums::Applications::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");

_document <<
"updated" << zpt::json::date();

_r_data = { "href", _topic, "n_updated", _c->set("Applications", _filter, _document, { "href", _topic }) };

	
	_emitter->mutations()->route(zpt::mutation::Update, _topic, { "performative", "set", "href", _topic, "changes", _document, "filter", _filter });
	return _r_data;
}

auto zpt::apps::datums::Applications::remove(std::string _topic, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
_r_data = { "href", _topic, "n_deleted", _c->remove("Applications", _topic, { "href", _topic }) };

	
	_emitter->mutations()->route(zpt::mutation::Remove, _topic, { "performative", "remove", "href", _topic });
	return _r_data;
}

auto zpt::apps::datums::Applications::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter, zpt::json _identity, zpt::json _envelope) -> zpt::json {
	zpt::json _r_data;
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
_r_data = { "href", _topic, "n_deleted", _c->remove("Applications", _filter, _filter + zpt::json({ "href", _topic })) };

	
	_emitter->mutations()->route(zpt::mutation::Remove, _topic, { "performative", "remove", "href", _topic, "filter", _filter });
	return _r_data;
}

