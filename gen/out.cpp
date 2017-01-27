#pragma once

#include <zapata/rest.h>
#include <string>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {
namespace apps {

namespace ResourceOwners {
auto get(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json;
auto query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json;
auto insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json;
auto save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json;
auto set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json;
auto set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json;
auto remove(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json;
auto remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json;
}
}
}




auto zpt::apps::ResourceOwners::get(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
	zpt::json _r_data = _c->get("ResourceOwners", _topic);
	
	return _r_data;
}

auto zpt::apps::ResourceOwners::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
	zpt::json _r_data = _c->query("ResourceOwners", _filter, _filter);
	
	return _r_data;
}

auto zpt::apps::ResourceOwners::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->insert("ResourceOwners", _topic, _document);
	
	return _r_data;
}

auto zpt::apps::ResourceOwners::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->save("ResourceOwners", _topic, _document);
	
	return _r_data;
}

auto zpt::apps::ResourceOwners::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->set("ResourceOwners", _topic, _document);
	
	return _r_data;
}

auto zpt::apps::ResourceOwners::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->set("ResourceOwners", _filter, _document);
	
	return _r_data;
}

auto zpt::apps::ResourceOwners::remove(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->remove("ResourceOwners", _topic);
	
	return _r_data;
}

auto zpt::apps::ResourceOwners::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->remove("ResourceOwners", _filter, _filter);
	
	return _r_data;
}


#pragma once

#include <zapata/rest.h>
#include <string>

using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

namespace zpt {
namespace apps {

namespace Applications {
auto get(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json;
auto query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json;
auto insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json;
auto save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json;
auto set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json;
auto set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json;
auto remove(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json;
auto remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json;
}
}
}




auto zpt::apps::Applications::get(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.redis.zpt.apps");
	zpt::json _r_data = _c->get("Applications", _topic);
	
	return _r_data;
}

auto zpt::apps::Applications::query(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.mongodb.zpt.apps");
	zpt::json _r_data = _c->query("Applications", _filter, _filter);
	
	return _r_data;
}

auto zpt::apps::Applications::insert(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->insert("Applications", _topic, _document);
	
	return _r_data;
}

auto zpt::apps::Applications::save(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->save("Applications", _topic, _document);
	
	return _r_data;
}

auto zpt::apps::Applications::set(std::string _topic, zpt::json _document, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->set("Applications", _topic, _document);
	
	return _r_data;
}

auto zpt::apps::Applications::set(std::string _topic, zpt::json _document, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->set("Applications", _filter, _document);
	
	return _r_data;
}

auto zpt::apps::Applications::remove(std::string _topic, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->remove("Applications", _topic);
	
	return _r_data;
}

auto zpt::apps::Applications::remove(std::string _topic, zpt::json _filter, zpt::ev::emitter _emitter) -> zpt::json {
	zpt::connector _c = _emitter->connector("dbms.postgresql.zpt.apps");
	zpt::json _r_data = _c->remove("Applications", _filter, _filter);
	
	return _r_data;
}


#include <zapata/rest.h>
#include <string>


using namespace std;
#if !defined __APPLE__
using namespace __gnu_cxx;
#endif

extern "C" void restify(zpt::ev::emitter _emitter) {
_emitter->connector({ { "dbms.mongodb.zpt.apps", zpt::connector(new zpt::mongodb::Client(_emitter->options(), "mongodb.zpt.apps")) }, { "dbms.pgsql.zpt.apps", zpt::connector(new zpt::pgsql::Client(_emitter->options(), "pgsql.zpt.apps")) }, { "dbms.redis.zpt.apps", zpt::connector(new zpt::redis::Client(_emitter->options(), "redis.zpt.apps")) }, });


_emitter->on("^/v2/datums/applications$",
{
{
zpt::ev::Get,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::query(_topic, _envelope["params"], _emitter);

return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body };

}
},

{
zpt::ev::Post,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] << "app_secret" << zpt::generate_key(24);
_envelope["payload"] >> "cloud_secret";
_envelope["payload"] << "cloud_secret" << zpt::generate_key(24);
assertz_mandatory(_envelope["payload"], "created", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
_envelope["payload"] << "device_secret" << zpt::generate_key(24);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_mandatory(_envelope["payload"], "name", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_mandatory(_envelope["payload"], "namespace", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_mandatory(_envelope["payload"], "updated", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::insert(_topic, _envelope["payload"], _emitter);

return { "status", 201, "payload", _r_body };

}
},


{
zpt::ev::Patch,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::set(_topic, _envelope["payload"], _envelope["params"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Delete,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::remove(_topic, _envelope["params"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Head,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::query(_topic, _envelope["params"], _emitter);

return { "headers",
{ "Content-Length", std::string(_r_body).length() }, "status", (_r_body->ok() ? 200 : 204) };

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
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::get(_topic, _emitter);

return { "status", (_r_body->ok() ? 200 : 204), "payload", _r_body  };

}
},


{
zpt::ev::Put,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] << "app_secret" << zpt::generate_key(24);
_envelope["payload"] >> "cloud_secret";
_envelope["payload"] << "cloud_secret" << zpt::generate_key(24);
assertz_mandatory(_envelope["payload"], "created", 412);
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
_envelope["payload"] << "device_secret" << zpt::generate_key(24);
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_mandatory(_envelope["payload"], "name", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_mandatory(_envelope["payload"], "namespace", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_mandatory(_envelope["payload"], "updated", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::save(_topic, _envelope["payload"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Patch,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::set(_topic, _envelope["payload"], _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Delete,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::remove(_topic, _emitter);

return { "status", 200, "payload", _r_body };

}
},

{
zpt::ev::Head,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {
_envelope["payload"] >> "app_secret";
_envelope["payload"] >> "cloud_secret";
assertz_timestamp(_envelope["payload"], "created", 412);
_envelope["payload"] << "created" << zpt::json::date();
_envelope["payload"] >> "device_secret";
assertz_uri(_envelope["payload"], "icon", 412);
assertz_uri(_envelope["payload"], "image", 412);
assertz_utf8(_envelope["payload"], "name", 412);
assertz_ascii(_envelope["payload"], "namespace", 412);
assertz_timestamp(_envelope["payload"], "updated", 412);
_envelope["payload"] << "updated" << zpt::json::date();

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_id = _t_split[3];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
_r_body = zpt::apps::Applications::get(_topic, _emitter);

return { "headers",
{ "Content-Length", std::string(_r_body).length() }, "status", (_r_body->ok() ? 200 : 204) };

}
},

},
{"0mq", true,"amqp", true,"http", true,"mqtt", true}
);

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
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) }
);


}
},

{
zpt::ev::Post,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_app_id = _t_split[3];
zpt::json _tv_user_id = _t_split[5];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
return _emitter->route(
zpt::ev::Post,
zpt::path::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_app_id), "users", std::string(_tv_user_id), "channels" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "payload", _envelope["payload"] }
);


}
},


{
zpt::ev::Patch,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_app_id = _t_split[3];
zpt::json _tv_user_id = _t_split[5];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
return _emitter->route(
zpt::ev::Patch,
zpt::path::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_app_id), "users", std::string(_tv_user_id), "channels" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "payload", _envelope["payload"], "params", (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) }
);


}
},

{
zpt::ev::Delete,
[] (zpt::ev::performative _performative, std::string _topic, zpt::json _envelope, zpt::ev::emitter _emitter) -> zpt::json {

zpt::json _t_split = zpt::split(_topic, "/");
zpt::json _tv_app_id = _t_split[3];
zpt::json _tv_user_id = _t_split[5];
zpt::json _identity = zpt::rest::authorization::validate(_envelope, _emitter);

zpt::json _r_body;
return _emitter->route(
zpt::ev::Get,
zpt::path::join({ zpt::array, "v2", "datums", "applications", std::string(_tv_app_id), "users", std::string(_tv_user_id), "channels" }),
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) }
);


}
},

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
{ "headers", zpt::rest::authorization::headers(_identity["access_token"]), "params", (_envelope["params"] + { "app_id", _tv_app_id, "user_id", _tv_user_id }) }
);


}
},

},
{"0mq", true,"amqp", true,"http", true,"mqtt", true}
);

}

